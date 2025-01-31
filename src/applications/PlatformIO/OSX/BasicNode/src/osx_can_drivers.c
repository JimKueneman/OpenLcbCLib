/** \copyright
 * Copyright (c) 2025, Jim Kueneman
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
 * \file osc_can_drivers.c
 *
 * This file in the interface between the OpenLcbCLib and the specific MCU/PC implementation
 * to read/write on the CAN bus.  A new supported MCU/PC will create a file that handles the
 * specifics then hook them into this file through #ifdefs
 *
 * @author Jim Kueneman
 * @date 5 Jan 2025
 */

#include "src/drivers/common/can_types.h"
#include "src/openlcb/openlcb_gridconnect.h"
#include "src/utilities/mustangpeak_string_helper.h"

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

can_rx_callback_func_t internal_can_rx_callback_func;

#define RETRY_TIME 5
#define PORT_NUMBER 12021

// How full the chips CAN fifo has gotten if supported
uint8_olcb_t DriverCan_max_can_fifo_depth = 0;

StringList _outgoing_gridconnect_strings;
uint8_olcb_t _rx_paused = FALSE;
uint8_olcb_t _is_connected = FALSE;

pthread_mutex_t can_mutex;

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

    fd_set writefds;

    FD_ZERO(&writefds);
    FD_SET(socket_fd, &writefds);

    struct timeval timeout;

    timeout.tv_sec = 5; // Set connection timeout to 5 seconds
    timeout.tv_usec = 0;

    int select_result = select_result = select(socket_fd + 1, NULL, &writefds, NULL, &timeout);

    if (select_result < 0)
    {
        printf("Connection error: %d\n", errno);
        return 0;
    }
    else if (select_result == 0)
    {
        printf("Connection timed out\n");
        return 0;
    }
    else
    {
        if (FD_ISSET(socket_fd, &writefds))
        {
            printf("Connection established\n");
            return 1;
        }
        else
        {
            printf("Unexpected select result\n");
            return 0;
        }
    }
}

int _connect_to_server(char ip_address[], uint16_olcb_t port)
{

    int socket_fd, connect_result;
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_address);
    servaddr.sin_port = htons(port);

    printf("Creating socket\n");

    // socket create and verification
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        printf("Socket creation failed: %d\n", socket_fd);
        return -1;
    }

    printf("Socket successfully created: %d\n", socket_fd);

    _set_blocking_socket_enabled(socket_fd, 0);

    connect_result = connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    if ((connect_result >= 0) || ((connect_result == -1) && (errno == EINPROGRESS)))

        if (_wait_for_connect_non_blocking(socket_fd))
            return socket_fd;

    close(socket_fd);
    return -1;
}

void *thread_function_can(void *arg)
{
    int thread_id = *((int *)arg); // Access argument passed to thread

    char ip_address[] = "127.0.0.1";
    uint16_olcb_t port = PORT_NUMBER;

    printf("TCP/IP GridConnect Thread %d started\n", thread_id);

    gridconnect_buffer_t gridconnect_buffer;
    char *gridconnect_buffer_ptr;
    uint8_olcb_t next_byte;
    long result = 0;
    int socket_fd = -1;
    can_msg_t can_message;
    uint64_olcb_t timer = 0;
    char *msg = (void *)0;

    can_message.state.allocated = 1;
    can_message.state.direct_tx = 0;

    socket_fd = _connect_to_server(ip_address, port);
    if (socket_fd < 0)
        exit(1);

    _is_connected = TRUE;
    _rx_paused = FALSE;

    while (1)
    {

        //  if (timer % 5000 == 0)
        //      printf("thread 1 heartbeat\n");
        timer++;

        pthread_mutex_lock(&can_mutex);
        if (!_rx_paused)
        {
            result = read(socket_fd, &next_byte, sizeof(next_byte));

            if (result > 0)
            {
                if (OpenLcbGridConnect_copy_out_gridconnect_when_done(next_byte, &gridconnect_buffer))
                {
                    OpenLcbGridConnect_to_can_msg(&gridconnect_buffer, &can_message);

                    msg = strcatnew("R", (char *)&gridconnect_buffer);
                    printf("%s\n", msg);
                    free(msg);

                    if (internal_can_rx_callback_func)
                        internal_can_rx_callback_func(0, &can_message);
                }
            }
            else if (result < 0) // zero is just timout for no data
            {
                if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) //  No data available use the dead time to send data
                {
                    gridconnect_buffer_ptr = ThreadSafeStringList_pop(&_outgoing_gridconnect_strings);
                    while (gridconnect_buffer_ptr)
                    {
                        msg = strcatnew("S", gridconnect_buffer_ptr);
                        printf("%s\n", msg);
                        free(msg);

                        msg = strcatnew(gridconnect_buffer_ptr, "\n\r");
                        write(socket_fd, msg, strlen(msg));
                        free(msg);
                        free(gridconnect_buffer_ptr);

                        gridconnect_buffer_ptr = ThreadSafeStringList_pop(&_outgoing_gridconnect_strings);
                    }

                    usleep(500);
                }
                else
                {
                    _is_connected = FALSE;
                    printf("Connection error detected: %d\n", errno);
                    printf("Shutting down connection.... \n");
                    result = shutdown(socket_fd, 2);
                    result = close(socket_fd);
                    exit(1);
                }
            }
        }
        pthread_mutex_unlock(&can_mutex);
        usleep(50);
    }
}

uint8_olcb_t OSxCanDriver_is_connected(void)
{
    pthread_mutex_lock(&can_mutex);
    uint8_olcb_t result = _is_connected;
    pthread_mutex_unlock(&can_mutex);
    return result;
}

uint8_olcb_t OSxCanDriver_is_can_tx_buffer_clear(uint16_olcb_t channel)
{

    // Socket has more than enough buffer to always take it
    return TRUE;
}

uint8_olcb_t OSxCanDriver_transmit_raw_can_frame(uint8_olcb_t channel, can_msg_t *msg)
{

    gridconnect_buffer_t gridconnect_buffer;

    OpenLcbGridConnect_from_can_msg(&gridconnect_buffer, msg);
    // printf("decomposed gridconnect: %s\n", gridconnect_buffer);
    ThreadSafeStringList_push(&_outgoing_gridconnect_strings, (char *)&gridconnect_buffer);

    return 0;
}

void OSxCanDriver_pause_can_rx(void)
{

    pthread_mutex_lock(&can_mutex);
    _rx_paused = TRUE;
    pthread_mutex_unlock(&can_mutex);
}

void OSxCanDriver_resume_can_rx(void)
{

    pthread_mutex_lock(&can_mutex);
    _rx_paused = FALSE;
    pthread_mutex_unlock(&can_mutex);
}

void OSxCanDriver_setup(can_rx_callback_func_t can_rx_callback)
{

    internal_can_rx_callback_func = can_rx_callback;

    printf("Mutex initialization for CAN - Result Code: %d\n", pthread_mutex_init(&can_mutex, NULL));

    ThreadSafeStringList_init(&_outgoing_gridconnect_strings);

    pthread_t thread1;
    int thread_num1 = 1;
    pthread_create(&thread1, NULL, thread_function_can, &thread_num1);
}
