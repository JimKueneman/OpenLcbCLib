################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../boot.cmd 

SYSCFG_SRCS += \
../CheryE02_Bootloader.syscfg 

C_SRCS += \
./ti_msp_dl_config.c \
./boot_config.c \
../main.c \
../startup_mspm0g351x_ticlang.c 

GEN_FILES += \
./device.opt \
./ti_msp_dl_config.c \
./boot_config.c 

C_DEPS += \
./ti_msp_dl_config.d \
./boot_config.d \
./main.d \
./startup_mspm0g351x_ticlang.d 

GEN_OPTS += \
./device.opt 

OBJS += \
./ti_msp_dl_config.o \
./boot_config.o \
./main.o \
./startup_mspm0g351x_ticlang.o 

GEN_MISC_FILES += \
./device.cmd.genlibs \
./ti_msp_dl_config.h \
./Event.dot \
./boot_config.h 

OBJS__QUOTED += \
"ti_msp_dl_config.o" \
"boot_config.o" \
"main.o" \
"startup_mspm0g351x_ticlang.o" 

GEN_MISC_FILES__QUOTED += \
"device.cmd.genlibs" \
"ti_msp_dl_config.h" \
"Event.dot" \
"boot_config.h" 

C_DEPS__QUOTED += \
"ti_msp_dl_config.d" \
"boot_config.d" \
"main.d" \
"startup_mspm0g351x_ticlang.d" 

GEN_FILES__QUOTED += \
"device.opt" \
"ti_msp_dl_config.c" \
"boot_config.c" 

SYSCFG_SRCS__QUOTED += \
"../CheryE02_Bootloader.syscfg" 

C_SRCS__QUOTED += \
"./ti_msp_dl_config.c" \
"./boot_config.c" \
"../main.c" \
"../startup_mspm0g351x_ticlang.c" 


