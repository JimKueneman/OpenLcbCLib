################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/openlcb_c_lib/openlcb/openlcb_application.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_config.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_float16.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_node.c \
../Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_message_network.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_snip.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.c \
../Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.c 

OBJS += \
./Core/Src/openlcb_c_lib/openlcb/openlcb_application.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_config.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_float16.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_node.o \
./Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_message_network.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_snip.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.o \
./Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.o 

C_DEPS += \
./Core/Src/openlcb_c_lib/openlcb/openlcb_application.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_config.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_float16.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_node.d \
./Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_message_network.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_snip.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.d \
./Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/openlcb_c_lib/openlcb/%.o Core/Src/openlcb_c_lib/openlcb/%.su Core/Src/openlcb_c_lib/openlcb/%.cyclo: ../Core/Src/openlcb_c_lib/openlcb/%.c Core/Src/openlcb_c_lib/openlcb/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-openlcb_c_lib-2f-openlcb

clean-Core-2f-Src-2f-openlcb_c_lib-2f-openlcb:
	-$(RM) ./Core/Src/openlcb_c_lib/openlcb/openlcb_application.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_application.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_application.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_application.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_application_train.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_list.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_buffer_store.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_config.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_config.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_config.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_config.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_float16.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_float16.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_float16.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_float16.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_gridconnect.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_main_statemachine.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_node.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_node.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_node.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_node.su ./Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.cyclo ./Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.d ./Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.o ./Core/Src/openlcb_c_lib/openlcb/openlcb_utilities.su ./Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.su ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.su ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.su ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.su ./Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_datagram_handler.su ./Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.d ./Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.o ./Core/Src/openlcb_c_lib/openlcb/protocol_event_transport.su ./Core/Src/openlcb_c_lib/openlcb/protocol_message_network.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_message_network.d ./Core/Src/openlcb_c_lib/openlcb/protocol_message_network.o ./Core/Src/openlcb_c_lib/openlcb/protocol_message_network.su ./Core/Src/openlcb_c_lib/openlcb/protocol_snip.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_snip.d ./Core/Src/openlcb_c_lib/openlcb/protocol_snip.o ./Core/Src/openlcb_c_lib/openlcb/protocol_snip.su ./Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_train_handler.su ./Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.cyclo ./Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.d ./Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.o ./Core/Src/openlcb_c_lib/openlcb/protocol_train_search_handler.su

.PHONY: clean-Core-2f-Src-2f-openlcb_c_lib-2f-openlcb

