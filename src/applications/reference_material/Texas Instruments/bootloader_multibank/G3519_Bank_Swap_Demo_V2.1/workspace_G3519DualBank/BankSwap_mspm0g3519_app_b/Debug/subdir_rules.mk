################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1653796465: ../app_b.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1280/ccs/utils/sysconfig_1.21.0/sysconfig_cli.bat" --script "C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/app_b.syscfg" -o "syscfg" -s "C:/ti/mspm0_sdk_2_02_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/device.opt: build-1653796465 ../app_b.syscfg
syscfg/device.cmd.genlibs: build-1653796465
syscfg/ti_msp_dl_config.c: build-1653796465
syscfg/ti_msp_dl_config.h: build-1653796465
syscfg/Event.dot: build-1653796465
syscfg: build-1653796465

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/Debug" -I"C:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_02_00_05/source" -gdwarf-3 -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g351x_ticlang.o: C:/ti/mspm0_sdk_2_02_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g351x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/Debug" -I"C:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_02_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1280/ccs/tools/compiler/ti-cgt-armllvm_3.2.2.LTS/bin/tiarmclang.exe" -c @"syscfg/device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/Debug" -I"C:/ti/mspm0_sdk_2_02_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_02_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/a0508911/Desktop/G3519_Bank_Swap_Demo_V2.0/workspace_G3519DualBank/BankSwap_mspm0g3519_app_b/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


