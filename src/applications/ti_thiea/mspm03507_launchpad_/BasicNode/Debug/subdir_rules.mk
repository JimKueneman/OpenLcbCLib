################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/TI/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode" -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode/Debug" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1231101263: ../basic_node.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"/Applications/TI/ccs2020/ccs/utils/sysconfig_1.25.0/sysconfig_cli.sh" --script "/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode/basic_node.syscfg" -o "." -s "/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1231101263 ../basic_node.syscfg
device.opt: build-1231101263
device.cmd.genlibs: build-1231101263
ti_msp_dl_config.c: build-1231101263
ti_msp_dl_config.h: build-1231101263
Event.dot: build-1231101263

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/TI/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode" -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode/Debug" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: /Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/TI/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode" -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode/Debug" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

debug_tools.o: ../debug_tools.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/TI/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode" -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode/Debug" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(basename\ $(<F)).o"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


