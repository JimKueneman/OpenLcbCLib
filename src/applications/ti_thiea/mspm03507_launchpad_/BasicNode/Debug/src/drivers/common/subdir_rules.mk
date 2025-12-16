################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
src/drivers/common/%.o: ../src/drivers/common/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/TI/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode" -I"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/TI_Thiea/MSPM03507_Launchpad/BasicNode/Debug" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"src/drivers/common/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


