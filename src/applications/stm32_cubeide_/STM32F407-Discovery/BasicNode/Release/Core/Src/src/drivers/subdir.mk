################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/src/drivers/alias_mappings.c 

OBJS += \
./Core/Src/src/drivers/alias_mappings.o 

C_DEPS += \
./Core/Src/src/drivers/alias_mappings.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/src/drivers/%.o Core/Src/src/drivers/%.su Core/Src/src/drivers/%.cyclo: ../Core/Src/src/drivers/%.c Core/Src/src/drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-src-2f-drivers

clean-Core-2f-Src-2f-src-2f-drivers:
	-$(RM) ./Core/Src/src/drivers/alias_mappings.cyclo ./Core/Src/src/drivers/alias_mappings.d ./Core/Src/src/drivers/alias_mappings.o ./Core/Src/src/drivers/alias_mappings.su

.PHONY: clean-Core-2f-Src-2f-src-2f-drivers

