################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-858485681: ../CheryE02_Bootloader.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1280/ccs/utils/sysconfig_1.21.0/sysconfig_cli.bat" --script "C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_boot/CheryE02_Bootloader.syscfg" -o "." -s "C:/ti/mspm0_sdk_2_02_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device.opt: build-858485681 ../CheryE02_Bootloader.syscfg
device.cmd.genlibs: build-858485681
ti_msp_dl_config.c: build-858485681
ti_msp_dl_config.h: build-858485681
Event.dot: build-858485681
boot_config.c: build-858485681
boot_config.h: build-858485681

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_boot" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_boot/Debug" -I"C:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_02_00_05/source" -I"C:/ti/mspm0_sdk_2_02_00_05/source/ti/driverlib/m0p/sysctl" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_boot" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_boot/Debug" -I"C:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_02_00_05/source" -I"C:/ti/mspm0_sdk_2_02_00_05/source/ti/driverlib/m0p/sysctl" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


