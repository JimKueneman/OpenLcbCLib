################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/application_callbacks/callbacks_can.c \
../Core/Src/application_callbacks/callbacks_config_mem.c \
../Core/Src/application_callbacks/callbacks_olcb.c 

OBJS += \
./Core/Src/application_callbacks/callbacks_can.o \
./Core/Src/application_callbacks/callbacks_config_mem.o \
./Core/Src/application_callbacks/callbacks_olcb.o 

C_DEPS += \
./Core/Src/application_callbacks/callbacks_can.d \
./Core/Src/application_callbacks/callbacks_config_mem.d \
./Core/Src/application_callbacks/callbacks_olcb.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/application_callbacks/%.o Core/Src/application_callbacks/%.su Core/Src/application_callbacks/%.cyclo: ../Core/Src/application_callbacks/%.c Core/Src/application_callbacks/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Core/Src -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-application_callbacks

clean-Core-2f-Src-2f-application_callbacks:
	-$(RM) ./Core/Src/application_callbacks/callbacks_can.cyclo ./Core/Src/application_callbacks/callbacks_can.d ./Core/Src/application_callbacks/callbacks_can.o ./Core/Src/application_callbacks/callbacks_can.su ./Core/Src/application_callbacks/callbacks_config_mem.cyclo ./Core/Src/application_callbacks/callbacks_config_mem.d ./Core/Src/application_callbacks/callbacks_config_mem.o ./Core/Src/application_callbacks/callbacks_config_mem.su ./Core/Src/application_callbacks/callbacks_olcb.cyclo ./Core/Src/application_callbacks/callbacks_olcb.d ./Core/Src/application_callbacks/callbacks_olcb.o ./Core/Src/application_callbacks/callbacks_olcb.su

.PHONY: clean-Core-2f-Src-2f-application_callbacks

