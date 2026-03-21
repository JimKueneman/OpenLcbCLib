################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/src/drivers/canbus/alias_mappings.c \
../Core/Src/src/drivers/canbus/can_buffer_fifo.c \
../Core/Src/src/drivers/canbus/can_buffer_store.c \
../Core/Src/src/drivers/canbus/can_config.c \
../Core/Src/src/drivers/canbus/can_login_message_handler.c \
../Core/Src/src/drivers/canbus/can_login_statemachine.c \
../Core/Src/src/drivers/canbus/can_main_statemachine.c \
../Core/Src/src/drivers/canbus/can_rx_message_handler.c \
../Core/Src/src/drivers/canbus/can_rx_statemachine.c \
../Core/Src/src/drivers/canbus/can_tx_message_handler.c \
../Core/Src/src/drivers/canbus/can_tx_statemachine.c \
../Core/Src/src/drivers/canbus/can_utilities.c 

OBJS += \
./Core/Src/src/drivers/canbus/alias_mappings.o \
./Core/Src/src/drivers/canbus/can_buffer_fifo.o \
./Core/Src/src/drivers/canbus/can_buffer_store.o \
./Core/Src/src/drivers/canbus/can_config.o \
./Core/Src/src/drivers/canbus/can_login_message_handler.o \
./Core/Src/src/drivers/canbus/can_login_statemachine.o \
./Core/Src/src/drivers/canbus/can_main_statemachine.o \
./Core/Src/src/drivers/canbus/can_rx_message_handler.o \
./Core/Src/src/drivers/canbus/can_rx_statemachine.o \
./Core/Src/src/drivers/canbus/can_tx_message_handler.o \
./Core/Src/src/drivers/canbus/can_tx_statemachine.o \
./Core/Src/src/drivers/canbus/can_utilities.o 

C_DEPS += \
./Core/Src/src/drivers/canbus/alias_mappings.d \
./Core/Src/src/drivers/canbus/can_buffer_fifo.d \
./Core/Src/src/drivers/canbus/can_buffer_store.d \
./Core/Src/src/drivers/canbus/can_config.d \
./Core/Src/src/drivers/canbus/can_login_message_handler.d \
./Core/Src/src/drivers/canbus/can_login_statemachine.d \
./Core/Src/src/drivers/canbus/can_main_statemachine.d \
./Core/Src/src/drivers/canbus/can_rx_message_handler.d \
./Core/Src/src/drivers/canbus/can_rx_statemachine.d \
./Core/Src/src/drivers/canbus/can_tx_message_handler.d \
./Core/Src/src/drivers/canbus/can_tx_statemachine.d \
./Core/Src/src/drivers/canbus/can_utilities.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/src/drivers/canbus/%.o Core/Src/src/drivers/canbus/%.su Core/Src/src/drivers/canbus/%.cyclo: ../Core/Src/src/drivers/canbus/%.c Core/Src/src/drivers/canbus/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-src-2f-drivers-2f-canbus

clean-Core-2f-Src-2f-src-2f-drivers-2f-canbus:
	-$(RM) ./Core/Src/src/drivers/canbus/alias_mappings.cyclo ./Core/Src/src/drivers/canbus/alias_mappings.d ./Core/Src/src/drivers/canbus/alias_mappings.o ./Core/Src/src/drivers/canbus/alias_mappings.su ./Core/Src/src/drivers/canbus/can_buffer_fifo.cyclo ./Core/Src/src/drivers/canbus/can_buffer_fifo.d ./Core/Src/src/drivers/canbus/can_buffer_fifo.o ./Core/Src/src/drivers/canbus/can_buffer_fifo.su ./Core/Src/src/drivers/canbus/can_buffer_store.cyclo ./Core/Src/src/drivers/canbus/can_buffer_store.d ./Core/Src/src/drivers/canbus/can_buffer_store.o ./Core/Src/src/drivers/canbus/can_buffer_store.su ./Core/Src/src/drivers/canbus/can_config.cyclo ./Core/Src/src/drivers/canbus/can_config.d ./Core/Src/src/drivers/canbus/can_config.o ./Core/Src/src/drivers/canbus/can_config.su ./Core/Src/src/drivers/canbus/can_login_message_handler.cyclo ./Core/Src/src/drivers/canbus/can_login_message_handler.d ./Core/Src/src/drivers/canbus/can_login_message_handler.o ./Core/Src/src/drivers/canbus/can_login_message_handler.su ./Core/Src/src/drivers/canbus/can_login_statemachine.cyclo ./Core/Src/src/drivers/canbus/can_login_statemachine.d ./Core/Src/src/drivers/canbus/can_login_statemachine.o ./Core/Src/src/drivers/canbus/can_login_statemachine.su ./Core/Src/src/drivers/canbus/can_main_statemachine.cyclo ./Core/Src/src/drivers/canbus/can_main_statemachine.d ./Core/Src/src/drivers/canbus/can_main_statemachine.o ./Core/Src/src/drivers/canbus/can_main_statemachine.su ./Core/Src/src/drivers/canbus/can_rx_message_handler.cyclo ./Core/Src/src/drivers/canbus/can_rx_message_handler.d ./Core/Src/src/drivers/canbus/can_rx_message_handler.o ./Core/Src/src/drivers/canbus/can_rx_message_handler.su ./Core/Src/src/drivers/canbus/can_rx_statemachine.cyclo ./Core/Src/src/drivers/canbus/can_rx_statemachine.d ./Core/Src/src/drivers/canbus/can_rx_statemachine.o ./Core/Src/src/drivers/canbus/can_rx_statemachine.su ./Core/Src/src/drivers/canbus/can_tx_message_handler.cyclo ./Core/Src/src/drivers/canbus/can_tx_message_handler.d ./Core/Src/src/drivers/canbus/can_tx_message_handler.o ./Core/Src/src/drivers/canbus/can_tx_message_handler.su ./Core/Src/src/drivers/canbus/can_tx_statemachine.cyclo ./Core/Src/src/drivers/canbus/can_tx_statemachine.d ./Core/Src/src/drivers/canbus/can_tx_statemachine.o ./Core/Src/src/drivers/canbus/can_tx_statemachine.su ./Core/Src/src/drivers/canbus/can_utilities.cyclo ./Core/Src/src/drivers/canbus/can_utilities.d ./Core/Src/src/drivers/canbus/can_utilities.o ./Core/Src/src/drivers/canbus/can_utilities.su

.PHONY: clean-Core-2f-Src-2f-src-2f-drivers-2f-canbus

