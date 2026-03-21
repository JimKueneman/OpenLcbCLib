################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USB_HOST/Target/subdir.mk
-include USB_HOST/App/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Src/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/openlcb_c_lib/utilities/subdir.mk
-include Core/Src/openlcb_c_lib/openlcb/subdir.mk
-include Core/Src/openlcb_c_lib/drivers/canbus/subdir.mk
-include Core/Src/application_drivers/subdir.mk
-include Core/Src/application_callbacks/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := BasicNodeBootloader
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
BasicNodeBootloader.elf \

MAP_FILES += \
BasicNodeBootloader.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
BasicNodeBootloader.list \

OBJCOPY_HEX += \
BasicNodeBootloader.hex \

OBJCOPY_BIN += \
BasicNodeBootloader.bin \


# All Target
all: main-build

# Main-build Target
main-build: BasicNodeBootloader.elf secondary-outputs

# Tool invocations
BasicNodeBootloader.elf BasicNodeBootloader.map: $(OBJS) $(USER_OBJS) /Users/jimkueneman/Documents/OpenLcbCLib/src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "BasicNodeBootloader.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/stm32_cubeide/stm32f407_discovery/BasicNodeBootloader/STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="BasicNodeBootloader.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

BasicNodeBootloader.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "BasicNodeBootloader.list"
	@echo 'Finished building: $@'
	@echo ' '

BasicNodeBootloader.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "BasicNodeBootloader.hex"
	@echo 'Finished building: $@'
	@echo ' '

BasicNodeBootloader.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "BasicNodeBootloader.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) BasicNodeBootloader.bin BasicNodeBootloader.elf BasicNodeBootloader.hex BasicNodeBootloader.list BasicNodeBootloader.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
