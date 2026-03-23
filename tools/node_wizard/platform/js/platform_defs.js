/* =========================================================================
 * Platform Definitions — code templates for each supported target platform.
 *
 * Each platform provides template code extracted from the working demo
 * applications in applications/.  Templates use {{paramName}} placeholders
 * that are substituted with user-configured values at generation time.
 *
 * Structure:
 *   canDrivers.extraIncludes  — additional #include lines for the CAN .c file
 *   canDrivers.globals        — module-level vars, helpers, ISRs (goes after includes)
 *   canDrivers.templates.*    — function body for each CAN driver function
 *   olcbDrivers.extraIncludes — additional #include lines for the OLCB .c file
 *   olcbDrivers.globals       — module-level vars, helpers, ISRs
 *   olcbDrivers.templates.*   — function body for each OLCB driver function
 *
 * If a template key is missing or null, the wizard falls back to a TODO stub.
 * ========================================================================= */

const PLATFORM_DEFS = {

    /* ------------------------------------------------------------------ */
    'none': {

        name: 'None / Custom',
        description: 'Generate empty stub functions (fill in your own)',
        image: 'images/custom.svg',
        isArduino: false,
        framework: '',
        libraries: [],
        notes: [],
        parameters: [],
        canDrivers: {
            extraIncludes: [],
            globals: '',
            templates: {}
        },
        olcbDrivers: {
            extraIncludes: [],
            globals: '',
            templates: {}
        }

    },

    /* ================================================================== */
    /*  ESP32  +  TWAI  (built-in CAN controller)                         */
    /* ================================================================== */
    'esp32-twai': {

        disabled: true,
        name: 'ESP32 + TWAI',
        description: 'Espressif ESP32 with built-in TWAI CAN controller',
        image: 'images/esp32-twai.svg',
        isArduino: true,
        framework: 'Arduino / PlatformIO',
        libraries: [],
        notes: [],
        parameters: [
            { id: 'can_tx_pin', label: 'CAN TX GPIO', defaultValue: '21', type: 'number' },
            { id: 'can_rx_pin', label: 'CAN RX GPIO', defaultValue: '22', type: 'number' }
        ],

        canDrivers: {

            extraIncludes: [
                '#include "Arduino.h"',
                '#include "driver/gpio.h"',
                '#include "driver/twai.h"'
            ],

            globals: [
                '',
                '#define INCLUDE_vTaskSuspend 1',
                '',
                'static bool _is_connected = false;',
                'static TaskHandle_t _receive_task_handle = NULL;',
                'static int _tx_queue_len = 0;',
                '',
                'static void _receive_task(void *arg) {',
                '',
                '    can_msg_t can_msg;',
                '    can_msg.state.allocated = 1;',
                '',
                '    while (1) {',
                '',
                '        twai_message_t message;',
                '        esp_err_t err = twai_receive(&message, pdMS_TO_TICKS(100));',
                '',
                '        if (err == ESP_OK && message.extd) {',
                '',
                '            can_msg.identifier = message.identifier;',
                '            can_msg.payload_count = message.data_length_code;',
                '            for (int i = 0; i < message.data_length_code; i++) {',
                '',
                '                can_msg.payload[i] = message.data[i];',
                '            }',
                '',
                '            CanRxStatemachine_incoming_can_driver_callback(&can_msg);',
                '        }',
                '    }',
                '}',
                '',
                'static void _pause_can_rx(void) {',
                '',
                '    vTaskSuspend(_receive_task_handle);',
                '}',
                '',
                'static void _resume_can_rx(void) {',
                '',
                '    vTaskResume(_receive_task_handle);',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_{{can_tx_pin}}, GPIO_NUM_{{can_rx_pin}}, TWAI_MODE_NORMAL);',
                    '    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();',
                    '    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();',
                    '',
                    '    _tx_queue_len = g_config.tx_queue_len;',
                    '',
                    '    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {',
                    '',
                    '        if (twai_start() == ESP_OK) {',
                    '',
                    '            _is_connected = true;',
                    '            xTaskCreate(_receive_task, "can_rx", 2048, NULL, 10, &_receive_task_handle);',
                    '        }',
                    '    }'
                ].join('\n'),

                transmit_raw_can_frame: [
                    '    twai_message_t message;',
                    '    message.identifier = can_msg->identifier;',
                    '    message.extd = 1;',
                    '    message.data_length_code = can_msg->payload_count;',
                    '    message.rtr = 0;',
                    '    message.ss = 0;',
                    '    message.self = 0;',
                    '    message.dlc_non_comp = 0;',
                    '',
                    '    for (int i = 0; i < can_msg->payload_count; i++) {',
                    '',
                    '        message.data[i] = can_msg->payload[i];',
                    '    }',
                    '',
                    '    return (twai_transmit(&message, 0) == ESP_OK);'
                ].join('\n'),

                is_can_tx_buffer_clear: [
                    '    twai_status_info_t status;',
                    '    twai_get_status_info(&status);',
                    '',
                    '    switch (status.state) {',
                    '',
                    '        case TWAI_STATE_RUNNING:',
                    '            return ((_tx_queue_len - status.msgs_to_tx) > 0);',
                    '',
                    '        case TWAI_STATE_BUS_OFF:',
                    '            twai_initiate_recovery();',
                    '            return false;',
                    '',
                    '        default:',
                    '            return false;',
                    '    }'
                ].join('\n'),

                is_connected: [
                    '    return _is_connected;'
                ].join('\n')
            }
        },

        olcbDrivers: {

            extraIncludes: [
                '#include "Arduino.h"',
                '#include "esp_system.h"',
                '#include "esp32-hal-timer.h"',
                '#include "esp32-hal-cpu.h"'
            ],

            globals: [
                '',
                'static hw_timer_t *_timer = NULL;',
                '',
                'static void IRAM_ATTR _timer_isr(void) {',
                '',
                '    OpenLcb_100ms_timer_tick();',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    setCpuFrequencyMhz(240);',
                    '',
                    '    _timer = timerBegin(1000000);              // 1 MHz',
                    '    timerAttachInterrupt(_timer, &_timer_isr);',
                    '    timerAlarm(_timer, 100000, true, 0);       // 100 ms'
                ].join('\n'),

                lock_shared_resources: [
                    '    timerStop(_timer);',
                    '    _pause_can_rx();'
                ].join('\n'),

                unlock_shared_resources: [
                    '    timerStart(_timer);',
                    '    _resume_can_rx();'
                ].join('\n'),

                reboot: [
                    '    esp_restart();'
                ].join('\n')

                /* config_mem_read/write: not provided => stubs */
            }
        }

    },

    /* ================================================================== */
    /*  ESP32  +  WiFi GridConnect  (TCP/IP CAN gateway)                  */
    /* ================================================================== */
    'esp32-wifi': {

        disabled: true,
        name: 'ESP32 + WiFi GridConnect',
        description: 'ESP32 using WiFi TCP/IP CAN gateway (no physical CAN bus)',
        image: 'images/esp32-wifi.svg',
        isArduino: true,
        framework: 'Arduino / PlatformIO',
        libraries: [],
        notes: [
            'Requires a GridConnect server (e.g. JMRI, OpenLCB Hub) on the network',
            'Default port is 12021'
        ],
        parameters: [
            { id: 'wifi_server_ip', label: 'Server IP Address', defaultValue: '192.168.1.100', type: 'text' },
            { id: 'wifi_port', label: 'GridConnect Port', defaultValue: '12021', type: 'number' }
        ],

        canDrivers: {

            extraIncludes: [
                '#include "Arduino.h"',
                '#include "WiFi.h"',
                '#include <lwip/sockets.h>',
                '#include "src/openlcb/openlcb_gridconnect.h"'
            ],

            globals: [
                '',
                'static int _socket = -1;',
                'static TaskHandle_t _receive_task_handle = NULL;',
                'static bool _is_connected = false;',
                'static gridconnect_buffer_t _gc_buffer;',
                '',
                'static void _receive_task(void *arg) {',
                '',
                '    can_msg_t can_msg;',
                '    can_msg.state.allocated = 1;',
                '    char rx_char;',
                '    gridconnect_buffer_t gc_in;',
                '',
                '    OpenLcbGridConnect_initialize(&gc_in);',
                '',
                '    while (1) {',
                '',
                '        int result = recv(_socket, &rx_char, 1, MSG_DONTWAIT);',
                '',
                '        if (result > 0) {',
                '',
                '            if (OpenLcbGridConnect_copy_out_gridconnect_when_done(&gc_in, rx_char)) {',
                '',
                '                if (OpenLcbGridConnect_to_can_msg(&gc_in, &can_msg)) {',
                '',
                '                    CanRxStatemachine_incoming_can_driver_callback(&can_msg);',
                '                }',
                '                OpenLcbGridConnect_initialize(&gc_in);',
                '            }',
                '',
                '        } else {',
                '',
                '            vTaskDelay(pdMS_TO_TICKS(1));',
                '        }',
                '    }',
                '}',
                '',
                'static void _pause_can_rx(void) {',
                '',
                '    if (_receive_task_handle) { vTaskSuspend(_receive_task_handle); }',
                '}',
                '',
                'static void _resume_can_rx(void) {',
                '',
                '    if (_receive_task_handle) { vTaskResume(_receive_task_handle); }',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    // TODO: Connect to WiFi access point before calling this',
                    '    // WiFi.begin("your_ssid", "your_password");',
                    '    // while (WiFi.status() != WL_CONNECTED) { delay(500); }',
                    '',
                    '    struct sockaddr_in server_addr;',
                    '    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);',
                    '',
                    '    if (_socket >= 0) {',
                    '',
                    '        memset(&server_addr, 0, sizeof(server_addr));',
                    '        server_addr.sin_family = AF_INET;',
                    '        server_addr.sin_port = htons({{wifi_port}});',
                    '        inet_pton(AF_INET, "{{wifi_server_ip}}", &server_addr.sin_addr);',
                    '',
                    '        if (connect(_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {',
                    '',
                    '            _is_connected = true;',
                    '            xTaskCreate(_receive_task, "gc_rx", 4096, NULL, 10, &_receive_task_handle);',
                    '        }',
                    '    }'
                ].join('\n'),

                transmit_raw_can_frame: [
                    '    if (_socket < 0) { return false; }',
                    '',
                    '    gridconnect_buffer_t gc_buffer;',
                    '    OpenLcbGridConnect_from_can_msg(&gc_buffer, can_msg);',
                    '',
                    '    int len = strlen((char *)&gc_buffer);',
                    '    return (send(_socket, &gc_buffer, len, 0) == len);'
                ].join('\n'),

                is_can_tx_buffer_clear: [
                    '    return (_socket >= 0);'
                ].join('\n'),

                is_connected: [
                    '    return _is_connected;'
                ].join('\n')
            }
        },

        olcbDrivers: {

            extraIncludes: [
                '#include "Arduino.h"',
                '#include "esp_system.h"',
                '#include "esp32-hal-timer.h"',
                '#include "esp32-hal-cpu.h"'
            ],

            globals: [
                '',
                'static hw_timer_t *_timer = NULL;',
                '',
                'static void IRAM_ATTR _timer_isr(void) {',
                '',
                '    OpenLcb_100ms_timer_tick();',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    setCpuFrequencyMhz(240);',
                    '',
                    '    _timer = timerBegin(1000000);              // 1 MHz',
                    '    timerAttachInterrupt(_timer, &_timer_isr);',
                    '    timerAlarm(_timer, 100000, true, 0);       // 100 ms'
                ].join('\n'),

                lock_shared_resources: [
                    '    timerStop(_timer);',
                    '    _pause_can_rx();'
                ].join('\n'),

                unlock_shared_resources: [
                    '    timerStart(_timer);',
                    '    _resume_can_rx();'
                ].join('\n'),

                reboot: [
                    '    esp_restart();'
                ].join('\n')
            }
        }

    },

    /* ================================================================== */
    /*  RP2040  (Raspberry Pi Pico)  +  MCP2517FD  via SPI                */
    /* ================================================================== */
    'rp2040-mcp2517': {

        disabled: true,
        name: 'RP2040 + MCP2517FD',
        description: 'Raspberry Pi Pico with MCP2517FD CAN controller via SPI',
        image: 'images/rp2040-mcp2517.svg',
        isArduino: true,
        framework: 'Arduino (Earle Philhower core)',
        libraries: ['ACAN2517'],
        notes: [
            'Install ACAN2517 library via Arduino Library Manager',
            'Uses Earle Philhower RP2040 Arduino core'
        ],
        parameters: [
            { id: 'spi_cs',   label: 'SPI CS Pin',   defaultValue: '17', type: 'number' },
            { id: 'can_int',  label: 'CAN INT Pin',   defaultValue: '20', type: 'number' },
            { id: 'osc_freq', label: 'MCP2517 Oscillator', defaultValue: '40',
              type: 'select', options: [
                  { value: '20', label: '20 MHz' },
                  { value: '40', label: '40 MHz' }
              ] }
        ],

        canDrivers: {

            extraIncludes: [
                '#include "Arduino.h"',
                '#include <ACAN2517.h>',
                '#include <SPI.h>'
            ],

            globals: [
                '',
                'static const byte MCP2517_CS  = {{spi_cs}};',
                'static const byte MCP2517_INT = {{can_int}};',
                '',
                'static ACAN2517 _can(MCP2517_CS, SPI, MCP2517_INT);',
                '',
                '// Call this from your main loop to poll for incoming CAN frames',
                'void CanDriver_process_receive(void) {',
                '',
                '    can_msg_t can_msg;',
                '    CANMessage frame;',
                '',
                '    if (_can.available()) {',
                '',
                '        if (_can.receive(frame)) {',
                '',
                '            if (frame.ext) {',
                '',
                '                can_msg.state.allocated = true;',
                '                can_msg.payload_count = frame.len;',
                '                can_msg.identifier = frame.id;',
                '',
                '                for (int i = 0; i < frame.len; i++) {',
                '',
                '                    can_msg.payload[i] = frame.data[i];',
                '                }',
                '',
                '                CanRxStatemachine_incoming_can_driver_callback(&can_msg);',
                '            }',
                '        }',
                '    }',
                '}',
                '',
                'static void _pause_can_rx(void) {',
                '',
                '    // Not required — ACAN2517 uses an interrupt for background buffering',
                '    // and we access frames from the main loop via process_receive()',
                '}',
                '',
                'static void _resume_can_rx(void) {',
                '',
                '    // Not required — see _pause_can_rx()',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    SPI.begin(true);',
                    '    ACAN2517Settings settings(ACAN2517Settings::OSC_{{osc_freq}}MHz, 125UL * 1000UL);',
                    '',
                    '    _can.begin(settings, [] {',
                    '',
                    '        _can.isr();',
                    '    });'
                ].join('\n'),

                transmit_raw_can_frame: [
                    '    CANMessage frame;',
                    '',
                    '    frame.ext = true;',
                    '    frame.id = can_msg->identifier;',
                    '    frame.len = can_msg->payload_count;',
                    '',
                    '    for (int i = 0; i < frame.len; i++) {',
                    '',
                    '        frame.data[i] = can_msg->payload[i];',
                    '    }',
                    '',
                    '    return _can.tryToSend(frame);'
                ].join('\n'),

                is_can_tx_buffer_clear: [
                    '    // ACAN2517 library does not expose buffer status.',
                    '    // Return true and let transmit handle any failure.',
                    '    return true;'
                ].join('\n'),

                is_connected: [
                    '    return true;  // Always connected after setup'
                ].join('\n')
            }
        },

        olcbDrivers: {

            extraIncludes: [
                '#include "Arduino.h"',
                '#include "pico/stdlib.h"',
                '#include "pico/time.h"'
            ],

            globals: [
                '',
                'static struct repeating_timer _timer;',
                'static bool _timer_enabled = false;',
                'static bool _timer_unhandled_tick = false;',
                '',
                'static void _handle_timer_tick(void) {',
                '',
                '    OpenLcb_100ms_timer_tick();',
                '}',
                '',
                'static bool _timer_callback(__unused struct repeating_timer *t) {',
                '',
                '    if (_timer_enabled) {',
                '',
                '        _handle_timer_tick();',
                '',
                '    } else {',
                '',
                '        _timer_unhandled_tick = true;',
                '    }',
                '',
                '    return true;',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    _timer_enabled = add_repeating_timer_ms(-100, _timer_callback, NULL, &_timer);'
                ].join('\n'),

                lock_shared_resources: [
                    '    _pause_can_rx();',
                    '    _timer_enabled = false;'
                ].join('\n'),

                unlock_shared_resources: [
                    '    _resume_can_rx();',
                    '',
                    '    if (_timer_unhandled_tick) {',
                    '',
                    '        _timer_enabled = true;',
                    '        _timer_unhandled_tick = false;',
                    '        _handle_timer_tick();',
                    '',
                    '    } else {',
                    '',
                    '        _timer_enabled = true;',
                    '    }'
                ].join('\n'),

                reboot: [
                    '    rp2040.reboot();'
                ].join('\n')

                /* config_mem_read/write: not provided => stubs */
            }
        }

    },

    /* ================================================================== */
    /*  STM32F4xx  +  built-in CAN  (HAL / CubeIDE)                      */
    /* ================================================================== */
    'stm32f4': {

        disabled: true,
        name: 'STM32F4xx + CAN',
        description: 'STM32F4 with built-in CAN peripheral (CubeIDE / HAL)',
        image: 'images/stm32f4.svg',
        isArduino: false,
        framework: 'STM32 HAL (CubeIDE)',
        libraries: [],
        notes: [
            'CAN and timer peripherals must be configured in STM32CubeMX',
            'Call CanDriver_initialize(&hcan1) and Drivers_initialize(&htim7) from main()'
        ],
        parameters: [],

        canDrivers: {

            extraIncludes: [
                '#include "stm32f4xx_hal.h"'
            ],

            globals: [
                '',
                'static bool _is_transmitting = false;',
                'static CAN_HandleTypeDef *_hcan = NULL;',
                '',
                '// Call from main() after CubeMX init: CanDriver_initialize(&hcan1);',
                'void CanDriver_initialize_hal(CAN_HandleTypeDef *hcan) {',
                '',
                '    CAN_FilterTypeDef rx_filter;',
                '',
                '    _hcan = hcan;',
                '',
                '    rx_filter.FilterActivation = CAN_FILTER_ENABLE;',
                '    rx_filter.FilterBank = 0;',
                '    rx_filter.SlaveStartFilterBank = 0;',
                '    rx_filter.FilterFIFOAssignment = CAN_RX_FIFO0;',
                '    rx_filter.FilterIdHigh = 0;',
                '    rx_filter.FilterIdLow = 0;',
                '    rx_filter.FilterMaskIdHigh = 0;',
                '    rx_filter.FilterMaskIdLow = 0;',
                '    rx_filter.FilterMode = CAN_FILTERMODE_IDMASK;',
                '    rx_filter.FilterScale = CAN_FILTERSCALE_32BIT;',
                '',
                '    HAL_CAN_ConfigFilter(hcan, &rx_filter);',
                '    HAL_CAN_Start(hcan);',
                '    HAL_CAN_ActivateNotification(hcan, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING);',
                '}',
                '',
                'static void _pause_can_rx(void) {',
                '',
                '    HAL_CAN_DeactivateNotification(_hcan, CAN_IT_RX_FIFO0_MSG_PENDING);',
                '}',
                '',
                'static void _resume_can_rx(void) {',
                '',
                '    HAL_CAN_ActivateNotification(_hcan, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING);',
                '}',
                '',
                '// HAL TX complete callbacks',
                'void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) { _is_transmitting = false; }',
                'void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) { _is_transmitting = false; }',
                'void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) { _is_transmitting = false; }',
                '',
                '// HAL RX callback — receives CAN frames from interrupt',
                'void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {',
                '',
                '    CAN_RxHeaderTypeDef rx_header;',
                '    uint8_t rx_data[8];',
                '    can_msg_t can_msg;',
                '',
                '    memset(&rx_header, 0x00, sizeof(rx_header));',
                '    memset(&can_msg, 0x00, sizeof(can_msg));',
                '',
                '    while (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {',
                '',
                '        if ((rx_header.IDE == CAN_ID_EXT) && (rx_header.RTR == CAN_RTR_DATA)) {',
                '',
                '            can_msg.state.allocated = true;',
                '            can_msg.identifier = rx_header.ExtId;',
                '            can_msg.payload_count = rx_header.DLC;',
                '',
                '            for (int i = 0; i < rx_header.DLC; i++) {',
                '',
                '                can_msg.payload[i] = rx_data[i];',
                '            }',
                '',
                '            CanRxStatemachine_incoming_can_driver_callback(&can_msg);',
                '        }',
                '    }',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    // CAN peripheral is configured by CubeMX.',
                    '    // Call CanDriver_initialize_hal(&hcan1) from main() after MX_CAN1_Init().'
                ].join('\n'),

                transmit_raw_can_frame: [
                    '    CAN_TxHeaderTypeDef tx_header;',
                    '    uint8_t tx_data[8];',
                    '    uint32_t tx_mailbox;',
                    '',
                    '    if (!CanDriver_is_can_tx_buffer_clear()) { return false; }',
                    '',
                    '    tx_header.DLC = can_msg->payload_count;',
                    '    tx_header.ExtId = can_msg->identifier;',
                    '    tx_header.RTR = CAN_RTR_DATA;',
                    '    tx_header.IDE = CAN_ID_EXT;',
                    '    tx_header.TransmitGlobalTime = DISABLE;',
                    '    tx_header.StdId = 0x00;',
                    '',
                    '    for (int i = 0; i < can_msg->payload_count; i++) {',
                    '',
                    '        tx_data[i] = can_msg->payload[i];',
                    '    }',
                    '',
                    '    if (HAL_CAN_AddTxMessage(_hcan, &tx_header, tx_data, &tx_mailbox) == HAL_OK) {',
                    '',
                    '        _is_transmitting = true;',
                    '        return true;',
                    '    }',
                    '',
                    '    return false;'
                ].join('\n'),

                is_can_tx_buffer_clear: [
                    '    _pause_can_rx();',
                    '    bool result = !_is_transmitting;',
                    '    _resume_can_rx();',
                    '    return result;'
                ].join('\n'),

                is_connected: [
                    '    return (_hcan != NULL);'
                ].join('\n')
            }
        },

        olcbDrivers: {

            extraIncludes: [
                '#include "stm32f4xx_hal.h"'
            ],

            globals: [
                '',
                'static TIM_HandleTypeDef *_htim = NULL;',
                '',
                '// Call from main() after CubeMX init: Drivers_initialize_hal(&htim7);',
                'void Drivers_initialize_hal(TIM_HandleTypeDef *htim) {',
                '',
                '    _htim = htim;',
                '    HAL_TIM_Base_Start_IT(htim);',
                '}',
                '',
                '// HAL timer callback — fires every 100 ms',
                'void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {',
                '',
                '    if (htim == _htim) {',
                '',
                '        OpenLcb_100ms_timer_tick();',
                '    }',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    // Timer peripheral is configured by CubeMX.',
                    '    // Call Drivers_initialize_hal(&htim7) from main() after MX_TIM7_Init().'
                ].join('\n'),

                lock_shared_resources: [
                    '    _pause_can_rx();',
                    '    HAL_TIM_Base_Stop(_htim);'
                ].join('\n'),

                unlock_shared_resources: [
                    '    _resume_can_rx();',
                    '    HAL_TIM_Base_Start_IT(_htim);'
                ].join('\n'),

                reboot: [
                    '    HAL_NVIC_SystemReset();'
                ].join('\n')

                /* config_mem_read/write: not provided => stubs */
            }
        }

    },

    /* ================================================================== */
    /*  TI MSPM0  +  built-in MCAN                                        */
    /* ================================================================== */
    'ti-mspm0': {

        disabled: true,
        name: 'TI MSPM0 + MCAN',
        description: 'TI MSPM0 with built-in MCAN peripheral (DriverLib)',
        image: 'images/ti-mspm0.svg',
        isArduino: false,
        framework: 'TI DriverLib (Code Composer / Theia)',
        libraries: [],
        notes: [
            'MCAN and SysTick must be configured in SysConfig (.syscfg)',
            'Uses SysTick for the 100 ms timer and MCAN0 for CAN'
        ],
        parameters: [],

        canDrivers: {

            extraIncludes: [
                '#include <ti/driverlib/m0p/dl_interrupt.h>',
                '#include "ti_msp_dl_config.h"'
            ],

            globals: [
                '',
                'static bool _is_transmitting = false;',
                '',
                'static void _pause_can_rx(void) {',
                '',
                '    NVIC_DisableIRQ(MCAN0_INST_INT_IRQN);',
                '}',
                '',
                'static void _resume_can_rx(void) {',
                '',
                '    NVIC_EnableIRQ(MCAN0_INST_INT_IRQN);',
                '}',
                '',
                '// MCAN0 interrupt handler — receives CAN frames and TX complete',
                'void MCAN0_INST_IRQHandler(void) {',
                '',
                '    uint32_t interrupt_flags = 0;',
                '    DL_MCAN_RxFIFOStatus fifo_status;',
                '    DL_MCAN_RxBufElement rx_msg;',
                '    can_msg_t can_msg;',
                '',
                '    DL_MCAN_IIDX pending = DL_MCAN_getPendingInterrupt(MCAN0_INST);',
                '',
                '    if (pending == DL_MCAN_IIDX_LINE1) {',
                '',
                '        interrupt_flags = DL_MCAN_getIntrStatus(MCAN0_INST) & MCAN0_INST_MCAN_INTERRUPTS;',
                '        DL_MCAN_clearIntrStatus(MCAN0_INST, interrupt_flags, DL_MCAN_INTR_SRC_MCAN_LINE_1);',
                '',
                '        if ((interrupt_flags & DL_MCAN_INTERRUPT_RF1N) == DL_MCAN_INTERRUPT_RF1N) {',
                '',
                '            memset(&fifo_status, 0x00, sizeof(fifo_status));',
                '            fifo_status.num = DL_MCAN_RX_FIFO_NUM_1;',
                '            DL_MCAN_getRxFIFOStatus(MCAN0_INST, &fifo_status);',
                '',
                '            uint16_t count = fifo_status.fillLvl;',
                '            while (count > 0) {',
                '',
                '                memset(&rx_msg, 0x00, sizeof(rx_msg));',
                '                DL_MCAN_readMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_FIFO, 0U, DL_MCAN_RX_FIFO_NUM_1, &rx_msg);',
                '                DL_MCAN_writeRxFIFOAck(MCAN0_INST, DL_MCAN_RX_FIFO_NUM_1, fifo_status.getIdx);',
                '',
                '                can_msg.identifier = rx_msg.id;',
                '                can_msg.state.allocated = true;',
                '                can_msg.payload_count = rx_msg.dlc;',
                '                for (int i = 0; i < rx_msg.dlc; i++) {',
                '',
                '                    can_msg.payload[i] = rx_msg.data[i];',
                '                }',
                '',
                '                CanRxStatemachine_incoming_can_driver_callback(&can_msg);',
                '                count--;',
                '            }',
                '        }',
                '',
                '        if ((interrupt_flags & DL_MCAN_INTERRUPT_TC) == DL_MCAN_INTERRUPT_TC) {',
                '',
                '            _is_transmitting = false;',
                '        }',
                '    }',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    NVIC_EnableIRQ(MCAN0_INST_INT_IRQN);'
                ].join('\n'),

                transmit_raw_can_frame: [
                    '    DL_MCAN_TxBufElement tx_msg;',
                    '',
                    '    if (!CanDriver_is_can_tx_buffer_clear()) { return false; }',
                    '',
                    '    tx_msg.id = can_msg->identifier;',
                    '    tx_msg.rtr = 0U;',
                    '    tx_msg.xtd = 1U;',
                    '    tx_msg.esi = 0U;',
                    '    tx_msg.dlc = can_msg->payload_count;',
                    '    tx_msg.brs = 0U;',
                    '    tx_msg.fdf = 0U;',
                    '    tx_msg.efc = 1U;',
                    '    tx_msg.mm = 0x00U;',
                    '',
                    '    for (int i = 0; i < can_msg->payload_count; i++) {',
                    '',
                    '        tx_msg.data[i] = can_msg->payload[i];',
                    '    }',
                    '',
                    '    _is_transmitting = true;',
                    '    DL_MCAN_writeMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_BUF, 0U, &tx_msg);',
                    '    DL_MCAN_TXBufTransIntrEnable(MCAN0_INST, 0U, 1U);',
                    '    DL_MCAN_TXBufAddReq(MCAN0_INST, 0U);',
                    '',
                    '    return true;'
                ].join('\n'),

                is_can_tx_buffer_clear: [
                    '    _pause_can_rx();',
                    '    bool result = !_is_transmitting;',
                    '    _resume_can_rx();',
                    '    return result;'
                ].join('\n'),

                is_connected: [
                    '    return true;  // Always connected after MCAN init'
                ].join('\n')
            }
        },

        olcbDrivers: {

            extraIncludes: [
                '#include <ti/driverlib/m0p/dl_sysctl.h>',
                '#include <ti/driverlib/m0p/dl_interrupt.h>',
                '#include "ti_msp_dl_config.h"'
            ],

            globals: [
                '',
                '// SysTick handler — fires every 100 ms (configured in SysConfig)',
                'void SysTick_Handler(void) {',
                '',
                '    OpenLcb_100ms_timer_tick();',
                '}'
            ].join('\n'),

            templates: {

                initialize: [
                    '    // SysTick is configured by SysConfig (.syscfg) — no additional setup needed.'
                ].join('\n'),

                lock_shared_resources: [
                    '    _pause_can_rx();',
                    '    DL_SYSTICK_disable();'
                ].join('\n'),

                unlock_shared_resources: [
                    '    _resume_can_rx();',
                    '    DL_SYSTICK_enable();'
                ].join('\n'),

                reboot: [
                    '    DL_SYSCTL_resetDevice(0x03);'
                ].join('\n')

                /* config_mem_read/write: not provided => stubs */
            }
        }

    }

};
