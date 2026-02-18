################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/application_drivers/stm32_driverlib_can_driver.c \
../Core/Src/application_drivers/stm32_driverlib_drivers.c 

OBJS += \
./Core/Src/application_drivers/stm32_driverlib_can_driver.o \
./Core/Src/application_drivers/stm32_driverlib_drivers.o 

C_DEPS += \
./Core/Src/application_drivers/stm32_driverlib_can_driver.d \
./Core/Src/application_drivers/stm32_driverlib_drivers.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/application_drivers/%.o Core/Src/application_drivers/%.su Core/Src/application_drivers/%.cyclo: ../Core/Src/application_drivers/%.c Core/Src/application_drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../USB_HOST/App -I../USB_HOST/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-application_drivers

clean-Core-2f-Src-2f-application_drivers:
	-$(RM) ./Core/Src/application_drivers/stm32_driverlib_can_driver.cyclo ./Core/Src/application_drivers/stm32_driverlib_can_driver.d ./Core/Src/application_drivers/stm32_driverlib_can_driver.o ./Core/Src/application_drivers/stm32_driverlib_can_driver.su ./Core/Src/application_drivers/stm32_driverlib_drivers.cyclo ./Core/Src/application_drivers/stm32_driverlib_drivers.d ./Core/Src/application_drivers/stm32_driverlib_drivers.o ./Core/Src/application_drivers/stm32_driverlib_drivers.su

.PHONY: clean-Core-2f-Src-2f-application_drivers

