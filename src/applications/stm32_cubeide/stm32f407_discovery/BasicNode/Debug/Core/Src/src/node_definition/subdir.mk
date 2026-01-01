################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/src/node_definition/dependency_injection.c \
../Core/Src/src/node_definition/dependency_injection_canbus.c 

OBJS += \
./Core/Src/src/node_definition/dependency_injection.o \
./Core/Src/src/node_definition/dependency_injection_canbus.o 

C_DEPS += \
./Core/Src/src/node_definition/dependency_injection.d \
./Core/Src/src/node_definition/dependency_injection_canbus.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/src/node_definition/%.o Core/Src/src/node_definition/%.su Core/Src/src/node_definition/%.cyclo: ../Core/Src/src/node_definition/%.c Core/Src/src/node_definition/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-src-2f-node_definition

clean-Core-2f-Src-2f-src-2f-node_definition:
	-$(RM) ./Core/Src/src/node_definition/dependency_injection.cyclo ./Core/Src/src/node_definition/dependency_injection.d ./Core/Src/src/node_definition/dependency_injection.o ./Core/Src/src/node_definition/dependency_injection.su ./Core/Src/src/node_definition/dependency_injection_canbus.cyclo ./Core/Src/src/node_definition/dependency_injection_canbus.d ./Core/Src/src/node_definition/dependency_injection_canbus.o ./Core/Src/src/node_definition/dependency_injection_canbus.su

.PHONY: clean-Core-2f-Src-2f-src-2f-node_definition

