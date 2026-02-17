################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/src/openlcb/openlcb_application.c \
../Core/Src/src/openlcb/openlcb_application_broadcast_time.c \
../Core/Src/src/openlcb/openlcb_buffer_fifo.c \
../Core/Src/src/openlcb/openlcb_buffer_list.c \
../Core/Src/src/openlcb/openlcb_buffer_store.c \
../Core/Src/src/openlcb/openlcb_config.c \
../Core/Src/src/openlcb/openlcb_gridconnect.c \
../Core/Src/src/openlcb/openlcb_login_statemachine.c \
../Core/Src/src/openlcb/openlcb_login_statemachine_handler.c \
../Core/Src/src/openlcb/openlcb_main_statemachine.c \
../Core/Src/src/openlcb/openlcb_node.c \
../Core/Src/src/openlcb/openlcb_utilities.c \
../Core/Src/src/openlcb/protocol_broadcast_time_handler.c \
../Core/Src/src/openlcb/protocol_config_mem_operations_handler.c \
../Core/Src/src/openlcb/protocol_config_mem_read_handler.c \
../Core/Src/src/openlcb/protocol_config_mem_write_handler.c \
../Core/Src/src/openlcb/protocol_datagram_handler.c \
../Core/Src/src/openlcb/protocol_event_transport.c \
../Core/Src/src/openlcb/protocol_message_network.c \
../Core/Src/src/openlcb/protocol_snip.c 

OBJS += \
./Core/Src/src/openlcb/openlcb_application.o \
./Core/Src/src/openlcb/openlcb_application_broadcast_time.o \
./Core/Src/src/openlcb/openlcb_buffer_fifo.o \
./Core/Src/src/openlcb/openlcb_buffer_list.o \
./Core/Src/src/openlcb/openlcb_buffer_store.o \
./Core/Src/src/openlcb/openlcb_config.o \
./Core/Src/src/openlcb/openlcb_gridconnect.o \
./Core/Src/src/openlcb/openlcb_login_statemachine.o \
./Core/Src/src/openlcb/openlcb_login_statemachine_handler.o \
./Core/Src/src/openlcb/openlcb_main_statemachine.o \
./Core/Src/src/openlcb/openlcb_node.o \
./Core/Src/src/openlcb/openlcb_utilities.o \
./Core/Src/src/openlcb/protocol_broadcast_time_handler.o \
./Core/Src/src/openlcb/protocol_config_mem_operations_handler.o \
./Core/Src/src/openlcb/protocol_config_mem_read_handler.o \
./Core/Src/src/openlcb/protocol_config_mem_write_handler.o \
./Core/Src/src/openlcb/protocol_datagram_handler.o \
./Core/Src/src/openlcb/protocol_event_transport.o \
./Core/Src/src/openlcb/protocol_message_network.o \
./Core/Src/src/openlcb/protocol_snip.o 

C_DEPS += \
./Core/Src/src/openlcb/openlcb_application.d \
./Core/Src/src/openlcb/openlcb_application_broadcast_time.d \
./Core/Src/src/openlcb/openlcb_buffer_fifo.d \
./Core/Src/src/openlcb/openlcb_buffer_list.d \
./Core/Src/src/openlcb/openlcb_buffer_store.d \
./Core/Src/src/openlcb/openlcb_config.d \
./Core/Src/src/openlcb/openlcb_gridconnect.d \
./Core/Src/src/openlcb/openlcb_login_statemachine.d \
./Core/Src/src/openlcb/openlcb_login_statemachine_handler.d \
./Core/Src/src/openlcb/openlcb_main_statemachine.d \
./Core/Src/src/openlcb/openlcb_node.d \
./Core/Src/src/openlcb/openlcb_utilities.d \
./Core/Src/src/openlcb/protocol_broadcast_time_handler.d \
./Core/Src/src/openlcb/protocol_config_mem_operations_handler.d \
./Core/Src/src/openlcb/protocol_config_mem_read_handler.d \
./Core/Src/src/openlcb/protocol_config_mem_write_handler.d \
./Core/Src/src/openlcb/protocol_datagram_handler.d \
./Core/Src/src/openlcb/protocol_event_transport.d \
./Core/Src/src/openlcb/protocol_message_network.d \
./Core/Src/src/openlcb/protocol_snip.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/src/openlcb/%.o Core/Src/src/openlcb/%.su Core/Src/src/openlcb/%.cyclo: ../Core/Src/src/openlcb/%.c Core/Src/src/openlcb/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-src-2f-openlcb

clean-Core-2f-Src-2f-src-2f-openlcb:
	-$(RM) ./Core/Src/src/openlcb/openlcb_application.cyclo ./Core/Src/src/openlcb/openlcb_application.d ./Core/Src/src/openlcb/openlcb_application.o ./Core/Src/src/openlcb/openlcb_application.su ./Core/Src/src/openlcb/openlcb_application_broadcast_time.cyclo ./Core/Src/src/openlcb/openlcb_application_broadcast_time.d ./Core/Src/src/openlcb/openlcb_application_broadcast_time.o ./Core/Src/src/openlcb/openlcb_application_broadcast_time.su ./Core/Src/src/openlcb/openlcb_buffer_fifo.cyclo ./Core/Src/src/openlcb/openlcb_buffer_fifo.d ./Core/Src/src/openlcb/openlcb_buffer_fifo.o ./Core/Src/src/openlcb/openlcb_buffer_fifo.su ./Core/Src/src/openlcb/openlcb_buffer_list.cyclo ./Core/Src/src/openlcb/openlcb_buffer_list.d ./Core/Src/src/openlcb/openlcb_buffer_list.o ./Core/Src/src/openlcb/openlcb_buffer_list.su ./Core/Src/src/openlcb/openlcb_buffer_store.cyclo ./Core/Src/src/openlcb/openlcb_buffer_store.d ./Core/Src/src/openlcb/openlcb_buffer_store.o ./Core/Src/src/openlcb/openlcb_buffer_store.su ./Core/Src/src/openlcb/openlcb_config.cyclo ./Core/Src/src/openlcb/openlcb_config.d ./Core/Src/src/openlcb/openlcb_config.o ./Core/Src/src/openlcb/openlcb_config.su ./Core/Src/src/openlcb/openlcb_gridconnect.cyclo ./Core/Src/src/openlcb/openlcb_gridconnect.d ./Core/Src/src/openlcb/openlcb_gridconnect.o ./Core/Src/src/openlcb/openlcb_gridconnect.su ./Core/Src/src/openlcb/openlcb_login_statemachine.cyclo ./Core/Src/src/openlcb/openlcb_login_statemachine.d ./Core/Src/src/openlcb/openlcb_login_statemachine.o ./Core/Src/src/openlcb/openlcb_login_statemachine.su ./Core/Src/src/openlcb/openlcb_login_statemachine_handler.cyclo ./Core/Src/src/openlcb/openlcb_login_statemachine_handler.d ./Core/Src/src/openlcb/openlcb_login_statemachine_handler.o ./Core/Src/src/openlcb/openlcb_login_statemachine_handler.su ./Core/Src/src/openlcb/openlcb_main_statemachine.cyclo ./Core/Src/src/openlcb/openlcb_main_statemachine.d ./Core/Src/src/openlcb/openlcb_main_statemachine.o ./Core/Src/src/openlcb/openlcb_main_statemachine.su ./Core/Src/src/openlcb/openlcb_node.cyclo ./Core/Src/src/openlcb/openlcb_node.d ./Core/Src/src/openlcb/openlcb_node.o ./Core/Src/src/openlcb/openlcb_node.su ./Core/Src/src/openlcb/openlcb_utilities.cyclo ./Core/Src/src/openlcb/openlcb_utilities.d ./Core/Src/src/openlcb/openlcb_utilities.o ./Core/Src/src/openlcb/openlcb_utilities.su ./Core/Src/src/openlcb/protocol_broadcast_time_handler.cyclo ./Core/Src/src/openlcb/protocol_broadcast_time_handler.d ./Core/Src/src/openlcb/protocol_broadcast_time_handler.o ./Core/Src/src/openlcb/protocol_broadcast_time_handler.su ./Core/Src/src/openlcb/protocol_config_mem_operations_handler.cyclo ./Core/Src/src/openlcb/protocol_config_mem_operations_handler.d ./Core/Src/src/openlcb/protocol_config_mem_operations_handler.o ./Core/Src/src/openlcb/protocol_config_mem_operations_handler.su ./Core/Src/src/openlcb/protocol_config_mem_read_handler.cyclo ./Core/Src/src/openlcb/protocol_config_mem_read_handler.d ./Core/Src/src/openlcb/protocol_config_mem_read_handler.o ./Core/Src/src/openlcb/protocol_config_mem_read_handler.su ./Core/Src/src/openlcb/protocol_config_mem_write_handler.cyclo ./Core/Src/src/openlcb/protocol_config_mem_write_handler.d ./Core/Src/src/openlcb/protocol_config_mem_write_handler.o ./Core/Src/src/openlcb/protocol_config_mem_write_handler.su ./Core/Src/src/openlcb/protocol_datagram_handler.cyclo ./Core/Src/src/openlcb/protocol_datagram_handler.d ./Core/Src/src/openlcb/protocol_datagram_handler.o ./Core/Src/src/openlcb/protocol_datagram_handler.su ./Core/Src/src/openlcb/protocol_event_transport.cyclo ./Core/Src/src/openlcb/protocol_event_transport.d ./Core/Src/src/openlcb/protocol_event_transport.o ./Core/Src/src/openlcb/protocol_event_transport.su ./Core/Src/src/openlcb/protocol_message_network.cyclo ./Core/Src/src/openlcb/protocol_message_network.d ./Core/Src/src/openlcb/protocol_message_network.o ./Core/Src/src/openlcb/protocol_message_network.su ./Core/Src/src/openlcb/protocol_snip.cyclo ./Core/Src/src/openlcb/protocol_snip.d ./Core/Src/src/openlcb/protocol_snip.o ./Core/Src/src/openlcb/protocol_snip.su

.PHONY: clean-Core-2f-Src-2f-src-2f-openlcb

