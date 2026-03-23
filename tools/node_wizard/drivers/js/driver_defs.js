/* =========================================================================
 * Driver Definitions — defines the function stubs for each driver group.
 * Extracted from the existing demo application driver interfaces.
 *
 * Reference files:
 *   applications/xcode/BasicNode/application_drivers/osx_can_drivers.h
 *   applications/xcode/BasicNode/application_drivers/osx_drivers.h
 *   src/openlcb/openlcb_config.h  (openlcb_config_t struct)
 * ========================================================================= */

const DRIVER_GROUPS = {

    'can-drivers': {

        title: 'CAN Drivers',
        description: 'CAN bus transmit, receive, and status',
        filePrefix: 'openlcb_can_drivers',
        headerGuard: '__OPENLCB_CAN_DRIVERS__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"',
            '#include "src/drivers/canbus/can_types.h"'
        ],
        functionPrefix: 'CanDriver',
        functions: [

            {
                name: 'initialize',
                returnType: 'void',
                params: 'void',
                description: 'Initialize CAN hardware',
                detail: 'Configure CAN controller registers, set baud rate to 125 kbps (OpenLCB/LCC standard), enable extended frames (29-bit identifiers), and start the transceiver. On interrupt-driven platforms, configure RX/TX interrupt handlers here. On polled platforms, initialize the SPI or peripheral bus to the external CAN controller (e.g. MCP2517). Called once during startup before any CAN traffic occurs.',
                required: true
            },

            {
                name: 'transmit_raw_can_frame',
                returnType: 'bool',
                params: 'can_msg_t *can_msg',
                description: 'Send a CAN frame. Return true on success',
                detail: 'Copy the identifier and payload from the can_msg_t struct into the hardware transmit mailbox or buffer, then trigger transmission. Return true if the frame was accepted by the hardware, false if the TX buffer was full or the hardware rejected it. The library will retry later if false is returned. The can_msg_t contains a 29-bit extended identifier, a payload of 0–8 bytes, and a payload count.',
                required: true
            },

            {
                name: 'is_can_tx_buffer_clear',
                returnType: 'bool',
                params: 'void',
                description: 'Return true if the transmit buffer is available',
                detail: 'Check whether the CAN hardware transmit mailbox or buffer has room for another outgoing frame. The library calls this before attempting to transmit. If the hardware does not expose buffer status (e.g. some SPI CAN controllers), return true and let transmit_raw_can_frame handle the failure.',
                required: true
            },

            {
                name: 'is_connected',
                returnType: 'bool',
                params: 'void',
                description: 'Return true if CAN bus is connected',
                detail: 'Check whether the CAN transceiver detects an active bus. This typically reads a status register or GPIO pin. Some platforms may not support connection detection — in that case, return true after setup completes. The library uses this to decide when to begin the node login (alias allocation) sequence.',
                required: true
            }

        ]

    },

    'olcb-drivers': {

        title: 'OLCB/LCC Drivers',
        description: 'Platform drivers for memory, reboot, locking, and firmware',
        filePrefix: 'openlcb_drivers',
        headerGuard: '__OPENLCB_DRIVERS__',
        includes: [
            '#include "src/openlcb/openlcb_types.h"'
        ],
        functionPrefix: 'Drivers',
        functions: [

            {
                name: 'initialize',
                returnType: 'void',
                params: 'void',
                description: 'Initialize platform hardware',
                detail: 'Initialize platform-specific peripherals that the library depends on: start the 100 ms repeating timer that calls OpenLcb_100ms_timer_tick(), set up I2C/SPI buses for external storage (EEPROM, FRAM, Flash), and configure any GPIO pins needed for status LEDs or other indicators. Called once during startup after CAN setup.',
                required: true
            },

            {
                name: 'lock_shared_resources',
                returnType: 'void',
                params: 'void',
                description: 'Protect shared data from concurrent access (e.g. disable interrupts or acquire mutex)',
                detail: 'Prevent the 100 ms timer interrupt and CAN receive interrupt (or task) from running while the library accesses shared state. On bare-metal platforms, disable the timer and CAN RX interrupts. On RTOS platforms (FreeRTOS, etc.), suspend the CAN receive task and stop the timer. This function is wired into both the can_config_t and openlcb_config_t structs so a single implementation protects all shared resources. Keep this function as fast as possible — it is called frequently during protocol processing.',
                required: true,
                configField: 'lock_shared_resources'
            },

            {
                name: 'unlock_shared_resources',
                returnType: 'void',
                params: 'void',
                description: 'Release shared data protection',
                detail: 'Re-enable the 100 ms timer interrupt and CAN receive interrupt (or resume the CAN receive task). If a timer tick occurred while locked, handle the deferred tick now to avoid missing a 100 ms cycle. This is the counterpart to lock_shared_resources and must always be called in matching pairs.',
                required: true,
                configField: 'unlock_shared_resources'
            },

            {
                name: 'config_mem_read',
                returnType: 'uint16_t',
                params: 'openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer',
                description: 'Read user configuration from persistent storage (EEPROM, Flash, or file)',
                detail: 'Read "count" bytes starting at "address" from your persistent storage into the buffer. Return the number of bytes actually read. The library uses this for all Memory Configuration protocol read operations — reading CDI, ACDI, user config, and any other address spaces that map to your storage. Typical implementations use EEPROM read, I2C EEPROM block read, Flash read, or SPIFFS/LittleFS file read. Make sure to bounds-check against your storage size.',
                required: true,
                configField: 'config_mem_read',
                requiresConfigMem: true
            },

            {
                name: 'config_mem_write',
                returnType: 'uint16_t',
                params: 'openlcb_node_t *openlcb_node, uint32_t address, uint16_t count, configuration_memory_buffer_t *buffer',
                description: 'Write user configuration to persistent storage (EEPROM, Flash, or file)',
                detail: 'Write "count" bytes from the buffer to "address" in your persistent storage. Return the number of bytes actually written. The library calls this when a configuration tool (like JMRI) writes to your node. Typical implementations use EEPROM write, I2C EEPROM block write, or Flash page write. Be aware of write endurance limits on EEPROM/Flash and consider wear-leveling for frequently written areas.',
                required: true,
                configField: 'config_mem_write',
                requiresConfigMem: true
            },

            {
                name: 'reboot',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info',
                description: 'Reboot the processor after configuration changes',
                detail: 'Perform a hardware reset of the processor. A configuration tool sends this command after writing changes so the node restarts with updated settings. Use the platform-specific reset function: esp_restart() on ESP32, rp2040.reboot() on Pico, HAL_NVIC_SystemReset() on STM32, asm("RESET") on dsPIC. The library sends an Initialization Complete message on reboot to notify the network.',
                required: true,
                configField: 'reboot',
                requiresConfigMem: true
            },

            {
                name: 'firmware_write',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_write_request_info_t *config_mem_write_request_info, write_result_t write_result',
                description: 'Write firmware data during over-the-air upgrade',
                detail: 'Write a chunk of firmware image data to storage (Flash, file system, etc.). The data arrives in config_mem_write_request_info->write_buffer with the byte count in the request. Write the chunk to the firmware image file or flash region that was opened during freeze(). Call write_result when finished to send a write-OK or write-error reply. This is called repeatedly with sequential chunks until the entire firmware image has been transferred.',
                required: true,
                configField: 'firmware_write',
                requiresFirmware: true
            },

            {
                name: 'freeze',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info',
                description: 'Prepare for firmware upgrade (stop motors, disable I/O, open image file)',
                detail: 'Called at the start of a firmware upgrade sequence. Safely shut down any hardware that should not be active during the update (motors, relays, outputs), then open or prepare the firmware storage destination (e.g. open a file on LittleFS, erase a Flash sector). Set a flag so the node knows it is in firmware upgrade mode. The upgrade tool will then send firmware data chunks via firmware_write() calls.',
                required: true,
                configField: 'freeze',
                requiresFirmware: true
            },

            {
                name: 'unfreeze',
                returnType: 'void',
                params: 'openlcb_statemachine_info_t *statemachine_info, config_mem_operations_request_info_t *config_mem_operations_request_info',
                description: 'Finalize firmware upgrade and restart',
                detail: 'Called when the firmware upgrade is complete. Close the firmware image file, verify the image if possible (checksum, CRC), apply the update (e.g. picoOTA.commit() on Pico), then reboot the processor to run the new firmware. If verification fails, discard the image and report an error. Clear the firmware upgrade mode flag.',
                required: true,
                configField: 'unfreeze',
                requiresFirmware: true
            }

        ]

    }

};
