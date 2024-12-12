/** \copyright
 * Copyright (c) 2024, Jim Kueneman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \file can.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Dec 2024
 */

// Add any compiler specific includes

#include "../common/can_types.h"
#include "../../openlcb/openlcb_gridconnect.h"

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include "errno.h"
#include <fcntl.h>

#include <pthread.h>
#include "threadsafe_stringlist.h"

#define RETRY_TIME 5
#define PORT_NUMBER 12021

uint8_olcb_t _is_connected = FALSE;

// How full the chips CAN fifo has gotten if supported
uint8_olcb_t DriverCan_max_can_fifo_depth = 0;

StringList _outgoing_gridconnect_strings;
uint8_olcb_t _rx_paused = FALSE;

void _print_can_msg(can_msg_t *can_msg)
{

    printf("Identifier: 0x%04X", (int16_t)(can_msg->identifier >> 16) & 0xFFFF);
    printf("%04X", (int16_t)can_msg->identifier & 0xFFFF);
    printf("   Payload Count: %d\n", can_msg->payload_count);
    printf("[");
    for (int i = 0; i < can_msg->payload_count; i++)
        printf(" 0x%02X", can_msg->payload[i]);
    printf("]\n");
}

uint8_olcb_t DriverCan_is_connected()
{

    return _is_connected;
}

uint8_olcb_t DriverCan_is_can_tx_buffer_clear(uint16_olcb_t Channel)
{
    // Socket has more than enough buffer to always take it
    return TRUE;
}

uint8_olcb_t DriverCan_transmit_raw_can_frame(uint8_olcb_t channel, can_msg_t *msg)
{
    gridconnect_buffer_t gridconnect_buffer;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, msg);

    printf("-------------------\n");
    printf("decomposed gridconnect: %s\n", gridconnect_buffer);

    ThreadSafeStringList_push(&_outgoing_gridconnect_strings, (char *)&gridconnect_buffer);

    printf("------------------\n");

    return 0;
}

// This must be here and used to callback when a can frame is received
can_rx_callback_func_t internal_can_rx_callback_func;

void DriverCan_pause_can_rx() {

  _rx_paused = TRUE;

};

void DriverCan_resume_can_rx() {

   _rx_paused = FALSE;

};

/** Returns true on success, or false if there was an error */
uint8_olcb_t _set_blocking_socket_enabled(int fd, uint8_olcb_t blocking)
{
    if (fd < 0)
        return FALSE;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return FALSE;

    if (blocking)
        flags = flags & ~O_NONBLOCK;
    else
        flags = flags | O_NONBLOCK;

    return (fcntl(fd, F_SETFL, flags) == 0);
#endif
}

uint16_olcb_t _wait_for_connect_non_blocking(int socket_fd)
{

    // TODO: Get this working, this MAY have to do with using the local host address in testing... it likely always return true....

    fd_set writefds, exceptfds;

    FD_ZERO(&writefds);
    FD_SET(socket_fd, &writefds);
    FD_ZERO(&exceptfds);
    FD_SET(socket_fd, &exceptfds);

    struct timeval timeout;

    timeout.tv_sec = 5; // Set connection timeout to 5 seconds
    timeout.tv_usec = 0;

    int select_result = 0;

    do
    {

        select_result = select(socket_fd + 1, NULL, &writefds, &exceptfds, &timeout);

    } while (select <= 0);

    printf("select result: %d\n", select_result);

    if (select_result < 0)
    {
        perror("select");
        close(socket_fd);
        return -1;
    }
    else if (select_result == 0)
    {
        printf("Connection timed out\n");
        close(socket_fd);
        return -1;
    }
    else
    {
        if (FD_ISSET(socket_fd, &writefds))
        {
            printf("Connection established\n");
            return socket_fd;
        }
        else
        {
            printf("Unexpected select result\n");
            close(socket_fd);
            return -1;
        }
    }
}

int _connect_to_server(char ip_address[], uint16_olcb_t port)
{

    int socket_fd, connect_result;
    struct sockaddr_in servaddr;

    printf("creating socket\n");

    // socket create and verification
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        printf("socket creation failed: %d\n", socket_fd);
        return -1;
    }
    else
    {
        printf("socket successfully created: %d\n", socket_fd);

        bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(ip_address);
        servaddr.sin_port = htons(PORT_NUMBER);

        printf("before connect\n");

        // TODO: Get this working, this MAY have to do with using the local host address in testing... it likely always return true....

        _set_blocking_socket_enabled(socket_fd, 0);

        connect_result = connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

        return _wait_for_connect_non_blocking(socket_fd);

        printf("connected_result: %d\n", connect_result);

        if (connect_result == 0)
        {
            return socket_fd;
        }
        else
        {
            return -1;
        }
    }

    return -1;
}

void *thread_function_can(void *arg)
{
    int thread_id = *((int *)arg); // Access argument passed to thread

    char ip_address[] = "127.0.0.1";
    uint16_olcb_t port = PORT_NUMBER;

    printf("Thread %d started\n", thread_id);

    gridconnect_buffer_t gridconnect_buffer;
    char *gridconnect_buffer_ptr;
    uint8_olcb_t next_byte;
    int result = 0;
    int socket_fd = -1;
    can_msg_t can_message;

    can_message.state.allocated = 1;
    can_message.state.direct_tx = 0;

    while (1)
    {
        if (socket_fd < 0)
        {
            socket_fd = _connect_to_server(ip_address, port);
            if (socket_fd < 0)
                sleep(RETRY_TIME);

            _is_connected = (socket_fd >= 0);
        }
        else
        {

            gridconnect_buffer_ptr = ThreadSafeStringList_pop(&_outgoing_gridconnect_strings);
            if (gridconnect_buffer_ptr)
            {
                strcat(gridconnect_buffer_ptr, "\n\r"); // new line and carrage return;
                write(socket_fd, gridconnect_buffer_ptr, strlen(gridconnect_buffer_ptr));

                printf("threadsafe list pop: %s\n", gridconnect_buffer_ptr);
                free(gridconnect_buffer_ptr);
            }

if (!_rx_paused) {
    
  // Need a MUTEX here.....

}
            result = read(socket_fd, &next_byte, sizeof(next_byte));

//sleep (1);
      //      if (result == 0)
                usleep(100000);

            if (result > 0)
            {

                if (OpenLcbGridConnect_copy_out_gridconnect_when_done(next_byte, &gridconnect_buffer))
                {
                    OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_message);

                    _print_can_msg(&can_message);

                    if (internal_can_rx_callback_func)
                        internal_can_rx_callback_func(0, &can_message);
                }
            }
            else if (result < 0) // zero is just timout for no data
            {
                if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
                {
                    //  No data available
                }
                else
                {

                    _is_connected = FALSE;
                    printf("server error: %d\n", errno);

                    printf("Shutting down connection..............................................: ");
                    result = shutdown(socket_fd, 2);
                    printf("%d\n", result);

                    printf("Closing down socket: ");
                    result = close(socket_fd);
                    printf("%d\n", result);

                    socket_fd = -1;
                }
            }
        }
    }
}

void DriverCan_Initialization(can_rx_callback_func_t can_rx_callback_func)
{

    // This must be here
    internal_can_rx_callback_func = can_rx_callback_func;

    // Do any other initialization here

    ThreadSafeStringList_init(&_outgoing_gridconnect_strings);

    pthread_t thread1;

    int thread_num1 = 1;

    pthread_create(&thread1, NULL, thread_function_can, &thread_num1);
}