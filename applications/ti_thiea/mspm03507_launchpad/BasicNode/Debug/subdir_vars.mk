################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
SYSCFG_SRCS += \
../basic_node.syscfg 

C_SRCS += \
../basic_node.c \
./ti_msp_dl_config.c \
/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c \
../debug_tools.c \
../openlcb_user_config.c 

GEN_CMDS += \
./device_linker.cmd 

GEN_FILES += \
./device_linker.cmd \
./device.opt \
./ti_msp_dl_config.c 

C_DEPS += \
./basic_node.d \
./ti_msp_dl_config.d \
./startup_mspm0g350x_ticlang.d \
./debug_tools.d \
./openlcb_user_config.d 

GEN_OPTS += \
./device.opt 

OBJS += \
./basic_node.o \
./ti_msp_dl_config.o \
./startup_mspm0g350x_ticlang.o \
./debug_tools.o \
./openlcb_user_config.o 

GEN_MISC_FILES += \
./device.cmd.genlibs \
./ti_msp_dl_config.h \
./Event.dot 

OBJS__QUOTED += \
"basic_node.o" \
"ti_msp_dl_config.o" \
"startup_mspm0g350x_ticlang.o" \
"debug_tools.o" \
"openlcb_user_config.o" 

GEN_MISC_FILES__QUOTED += \
"device.cmd.genlibs" \
"ti_msp_dl_config.h" \
"Event.dot" 

C_DEPS__QUOTED += \
"basic_node.d" \
"ti_msp_dl_config.d" \
"startup_mspm0g350x_ticlang.d" \
"debug_tools.d" \
"openlcb_user_config.d" 

GEN_FILES__QUOTED += \
"device_linker.cmd" \
"device.opt" \
"ti_msp_dl_config.c" 

C_SRCS__QUOTED += \
"../basic_node.c" \
"./ti_msp_dl_config.c" \
"/Users/jimkueneman/ti/mspm0_sdk_2_07_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c" \
"../openlcb_user_config.c" 

SYSCFG_SRCS__QUOTED += \
"../basic_node.syscfg" 


