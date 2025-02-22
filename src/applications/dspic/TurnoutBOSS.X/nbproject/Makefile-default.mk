#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../openlcb/application.c ../../../openlcb/application_callbacks.c local_drivers/_25AA1024/25AA1024_driver.c local_drivers/_MCP23S17/MCP23S17_driver.c local_drivers/_MCP4014/MCP4014_driver.c turnoutboss_drivers.c turnoutboss_event_handler.c turnoutboss_hardware_handler.c turnoutboss_event_engine.c turnoutboss_board_configuration.c turnoutboss_signal_calculations_board_right.c turnoutboss_signal_calculations_board_left.c turnoutboss_signal_calculations.c reset_vector.s turnoutboss_node_parameters.c turnoutboss_traps.c turnoutboss_uart_handler.c turnoutboss_main.c ../TurnoutBossCommon/common_loader_app.c ../TurnoutBossCommon/common_debug_helper.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o ${OBJECTDIR}/turnoutboss_drivers.o ${OBJECTDIR}/turnoutboss_event_handler.o ${OBJECTDIR}/turnoutboss_hardware_handler.o ${OBJECTDIR}/turnoutboss_event_engine.o ${OBJECTDIR}/turnoutboss_board_configuration.o ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o ${OBJECTDIR}/turnoutboss_signal_calculations.o ${OBJECTDIR}/reset_vector.o ${OBJECTDIR}/turnoutboss_node_parameters.o ${OBJECTDIR}/turnoutboss_traps.o ${OBJECTDIR}/turnoutboss_uart_handler.o ${OBJECTDIR}/turnoutboss_main.o ${OBJECTDIR}/_ext/598189670/common_loader_app.o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d ${OBJECTDIR}/_ext/954557028/driver_can.o.d ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d ${OBJECTDIR}/_ext/156864402/application.o.d ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o.d ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o.d ${OBJECTDIR}/turnoutboss_drivers.o.d ${OBJECTDIR}/turnoutboss_event_handler.o.d ${OBJECTDIR}/turnoutboss_hardware_handler.o.d ${OBJECTDIR}/turnoutboss_event_engine.o.d ${OBJECTDIR}/turnoutboss_board_configuration.o.d ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o.d ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o.d ${OBJECTDIR}/turnoutboss_signal_calculations.o.d ${OBJECTDIR}/reset_vector.o.d ${OBJECTDIR}/turnoutboss_node_parameters.o.d ${OBJECTDIR}/turnoutboss_traps.o.d ${OBJECTDIR}/turnoutboss_uart_handler.o.d ${OBJECTDIR}/turnoutboss_main.o.d ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o ${OBJECTDIR}/turnoutboss_drivers.o ${OBJECTDIR}/turnoutboss_event_handler.o ${OBJECTDIR}/turnoutboss_hardware_handler.o ${OBJECTDIR}/turnoutboss_event_engine.o ${OBJECTDIR}/turnoutboss_board_configuration.o ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o ${OBJECTDIR}/turnoutboss_signal_calculations.o ${OBJECTDIR}/reset_vector.o ${OBJECTDIR}/turnoutboss_node_parameters.o ${OBJECTDIR}/turnoutboss_traps.o ${OBJECTDIR}/turnoutboss_uart_handler.o ${OBJECTDIR}/turnoutboss_main.o ${OBJECTDIR}/_ext/598189670/common_loader_app.o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o

# Source Files
SOURCEFILES=../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../openlcb/application.c ../../../openlcb/application_callbacks.c local_drivers/_25AA1024/25AA1024_driver.c local_drivers/_MCP23S17/MCP23S17_driver.c local_drivers/_MCP4014/MCP4014_driver.c turnoutboss_drivers.c turnoutboss_event_handler.c turnoutboss_hardware_handler.c turnoutboss_event_engine.c turnoutboss_board_configuration.c turnoutboss_signal_calculations_board_right.c turnoutboss_signal_calculations_board_left.c turnoutboss_signal_calculations.c reset_vector.s turnoutboss_node_parameters.c turnoutboss_traps.c turnoutboss_uart_handler.c turnoutboss_main.c ../TurnoutBossCommon/common_loader_app.c ../TurnoutBossCommon/common_debug_helper.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

# The following macros may be used in the pre and post step lines
_/_=/
ShExtension=.sh
Device=dsPIC33EP512GP504
ProjectDir=/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X
ProjectName=TurnoutBoss
ConfName=default
ImagePath=dist/default/${IMAGE_TYPE}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
ImageDir=dist/default/${IMAGE_TYPE}
ImageName=TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: ["/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -addressing=2 -o/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/turnoutboss_merged.hex ]"
	@"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -addressing=2 -o/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/turnoutboss_merged.hex 
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script="p33EP512GP504_TurnoutBoss.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/84ccbf2f8905c2a12dd49eecc8ddb3715980d811 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/313106f6db9a0e286e2087f24e1bdf7740709345 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/962abd20d596e6cd7b0a24b592cb9b7cbb9e9b4a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/9aa4955e3dc690bed47ca6a8d3977734e6cd6993 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/f5b1177dcbe99d8152424be0f961e24b75720ee5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/b38aa7f1726967b170224cb4e23bbc4b4582b774 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/aa342fe98e4f91e29bbcdb34e1b41888b3db9469 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/6a88566311b3917309b8372c28f2b7eb2d73bb0c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/7f4613883877ff4cea027278f981a6ad2fbd9c9d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/4abcf980f8c153092f268d93bb8994ca220cb19e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/a2a89c41be5caf96178625822c850c8307b8b9e4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/b237d0d33fd222834a3a4646dcca71cbd1e2873f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/552a63943e9913b78e54b3d06515ac3d8570d83a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/391ec00d672b69fd7ac946703f83bf17fe73539c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/81abeee313f728bac72f761ec0f5128def628e90 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/d13f56023e741ffce19b3f6873d92fd39d0952d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/c03b1e5e9bc5bb0b1b24f37f91e74ce2b513c5fc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/1ab18f3148646e577714faa508f318b28a3584e6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/f00aab0ffda1413e29577b5dcef0dfb51e44e7ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/4a1afaf47e4b8dac1c7ba1637a353f4781902352 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/427c7cfd5662af5ef64511276ab590fa129c6fa2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/efbd76e59eefb37c976ecc3ae3167bb6b34d8033 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/5c0b4733fbaff2d759ec6e7772c351efeca9f01d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/91344ca433b5db9203c4698483f4056e6373f973 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/d5518cea8ae21a867f823bb46c66287f37d8c346 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/fbf8e6e4166c51c32a8301f0632af8c5d902fbd8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/6a299cfeb2327ff60a8c9fd959aa1cc209303e1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/17a971322b424410ad7a0c8c6f1b331c059b0483 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o: local_drivers/_MCP23S17/MCP23S17_driver.c  .generated_files/flags/default/9da9479c51f59eab2f577b02ff8cf840bea5eb45 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_MCP23S17" 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_MCP23S17/MCP23S17_driver.c  -o ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o: local_drivers/_MCP4014/MCP4014_driver.c  .generated_files/flags/default/af58dc795699936819e8be7ca919bb2e5f8ccd3f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_MCP4014" 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_MCP4014/MCP4014_driver.c  -o ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_drivers.o: turnoutboss_drivers.c  .generated_files/flags/default/53392e202a37bd80d9ff6d00763dc455becd0453 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_drivers.c  -o ${OBJECTDIR}/turnoutboss_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_event_handler.o: turnoutboss_event_handler.c  .generated_files/flags/default/f742882af2b295b9ddee2241ee78f7ec8dcf3f3b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_event_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_event_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_event_handler.c  -o ${OBJECTDIR}/turnoutboss_event_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_event_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_hardware_handler.o: turnoutboss_hardware_handler.c  .generated_files/flags/default/35e296ab7cf2a623bf5682c974ff1f25654a9eba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_hardware_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_hardware_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_hardware_handler.c  -o ${OBJECTDIR}/turnoutboss_hardware_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_hardware_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_event_engine.o: turnoutboss_event_engine.c  .generated_files/flags/default/80157d6a1c456d2bfa493ad2350877a2343e608f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_event_engine.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_event_engine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_event_engine.c  -o ${OBJECTDIR}/turnoutboss_event_engine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_event_engine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_board_configuration.o: turnoutboss_board_configuration.c  .generated_files/flags/default/60ca25d98bbc73c80476a33312ce4909fdd871f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_board_configuration.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_board_configuration.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_board_configuration.c  -o ${OBJECTDIR}/turnoutboss_board_configuration.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_board_configuration.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o: turnoutboss_signal_calculations_board_right.c  .generated_files/flags/default/6ba34ed19ae47ffe46c487aab3f5de3f526fe02e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_signal_calculations_board_right.c  -o ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o: turnoutboss_signal_calculations_board_left.c  .generated_files/flags/default/f0166dec09005836edf49d3f7bc56d12bb98ef3d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_signal_calculations_board_left.c  -o ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_signal_calculations.o: turnoutboss_signal_calculations.c  .generated_files/flags/default/2b8e0e8ba41afbafd601cfee7201fc50a3b37c74 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_signal_calculations.c  -o ${OBJECTDIR}/turnoutboss_signal_calculations.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_signal_calculations.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_node_parameters.o: turnoutboss_node_parameters.c  .generated_files/flags/default/9b22f0f801b0400d34991d8488fcb7d835a49034 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_node_parameters.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_node_parameters.c  -o ${OBJECTDIR}/turnoutboss_node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_traps.o: turnoutboss_traps.c  .generated_files/flags/default/4781fe5084625be50ecf18a106b5db31c3c111f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_traps.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_traps.c  -o ${OBJECTDIR}/turnoutboss_traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_uart_handler.o: turnoutboss_uart_handler.c  .generated_files/flags/default/b7db0c6bb48a598d56d39f256581dd7cc06c3cf0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_uart_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_uart_handler.c  -o ${OBJECTDIR}/turnoutboss_uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_main.o: turnoutboss_main.c  .generated_files/flags/default/52cac668ed392086a17e7800e7bbbd5be1e6313a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_main.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_main.c  -o ${OBJECTDIR}/turnoutboss_main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_loader_app.o: ../TurnoutBossCommon/common_loader_app.c  .generated_files/flags/default/5dc6dfb1d76d838291af491a8593a7c6fdda5d96 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_loader_app.c  -o ${OBJECTDIR}/_ext/598189670/common_loader_app.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_loader_app.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_debug_helper.o: ../TurnoutBossCommon/common_debug_helper.c  .generated_files/flags/default/d7d321c13e6869da350025e67e6058b27b8cbcc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_debug_helper.c  -o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/86296599f828749fc35e03fb6d914c2d60b8d19d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/4eb0c3eb79b1c6d7186e8d6b3fd42b45c24262a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/6282e0465751aa790072c13364a39daa528e4586 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/4702db3e9492e41b1b2d4894e0ccbc08bf2cfa4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/b22ab5d0782f4feddd02a5e7bd6782603b0dcd9a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/6c6d30956071980fcc64e4e928f829eddb0e680e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/8acf080668fec0ff327d7d898596d8a9bcdb40c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/e5d01533850cb9d256954fca0fc375280228b623 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/bc265b70d368baa5e959aa6d035e93eff6b5c1c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/4e6f8a71b8941f161f18e641692f447796e2943a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/de5e0b9ef8ab9f7e0f4299fcf30907a8f09223c7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/38f58aa988f653610ab8743836c4783069ec70fb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/6d0c66c7b38fedf92193869e50c9bc4258980b24 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/bce58a4e82cf641175f563d499f3870d47478055 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/aea4d1101f5ca8927b8bd8a1e7db7ec6d4b69f5d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/3e46adcbbbc50cac14a569e31a4b9dba89246d96 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/cec0af471ad7e13bc784f1162580a78d5a35b1c1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/f4dfbb8a820ec0aa62abc2356d2a49d53116ceb3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/f308fb47cc6eab71c157e824abcf36a59d2cebe0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/6a09b6196159dbfd7448acfe40aecc99d52f8825 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/6b9c299ab5870aa661934367c6ac97968b55a018 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/c930c22f004f76a6a6624badbcbc6ee27723943e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/d537bd500d74382a97fd32798f4c2b527b5e0ccd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/b8744cb07ee668430eb41a96985729f255f62f30 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/917693e0d10857ea40eb7928b60ad85253518b65 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/93c69d05dc60bb4f33eeae29c788d73da1b663ed .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/813e3d3b43ed9abb95ddc399d4c1aa1cb7cf1e81 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/8facc79fbaa6c899f746ad9a6588e12fd850e1f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o: local_drivers/_MCP23S17/MCP23S17_driver.c  .generated_files/flags/default/33897ae6a1160096e5f61d4b49861c256f245164 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_MCP23S17" 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_MCP23S17/MCP23S17_driver.c  -o ${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_MCP23S17/MCP23S17_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o: local_drivers/_MCP4014/MCP4014_driver.c  .generated_files/flags/default/820cd95ac3e19c5c94f8d67927483ed32306a63c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_MCP4014" 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_MCP4014/MCP4014_driver.c  -o ${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_MCP4014/MCP4014_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_drivers.o: turnoutboss_drivers.c  .generated_files/flags/default/19a8bae7c52d566a27b6deb956b7842861068c88 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_drivers.c  -o ${OBJECTDIR}/turnoutboss_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_event_handler.o: turnoutboss_event_handler.c  .generated_files/flags/default/31577e1dbc672eb6fe01d70c6ea5024b79b2e14f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_event_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_event_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_event_handler.c  -o ${OBJECTDIR}/turnoutboss_event_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_event_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_hardware_handler.o: turnoutboss_hardware_handler.c  .generated_files/flags/default/b8d23badb10400e664ff13f004edc709733fea29 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_hardware_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_hardware_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_hardware_handler.c  -o ${OBJECTDIR}/turnoutboss_hardware_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_hardware_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_event_engine.o: turnoutboss_event_engine.c  .generated_files/flags/default/bc0c0f5615020c9223dea0df6de4ab2164976772 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_event_engine.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_event_engine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_event_engine.c  -o ${OBJECTDIR}/turnoutboss_event_engine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_event_engine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_board_configuration.o: turnoutboss_board_configuration.c  .generated_files/flags/default/485f9c367c592090a8eb7c10829531cf2dae81b9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_board_configuration.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_board_configuration.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_board_configuration.c  -o ${OBJECTDIR}/turnoutboss_board_configuration.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_board_configuration.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o: turnoutboss_signal_calculations_board_right.c  .generated_files/flags/default/589ecbaa417d1686188bd4c2b19f8d9baa6cdca0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_signal_calculations_board_right.c  -o ${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_signal_calculations_board_right.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o: turnoutboss_signal_calculations_board_left.c  .generated_files/flags/default/36fc2cf9734ef830bc50cfe4c70a33737e81a771 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_signal_calculations_board_left.c  -o ${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_signal_calculations_board_left.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_signal_calculations.o: turnoutboss_signal_calculations.c  .generated_files/flags/default/a940f8e69ab2ed5c972f205b9f868c5775390ea7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_signal_calculations.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_signal_calculations.c  -o ${OBJECTDIR}/turnoutboss_signal_calculations.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_signal_calculations.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_node_parameters.o: turnoutboss_node_parameters.c  .generated_files/flags/default/ad8058657d773b2ed1aa3824d4c8abd90d3dde55 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_node_parameters.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_node_parameters.c  -o ${OBJECTDIR}/turnoutboss_node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_traps.o: turnoutboss_traps.c  .generated_files/flags/default/43c3d3ee7c865e8afbdb991350dabf52e6e1926a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_traps.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_traps.c  -o ${OBJECTDIR}/turnoutboss_traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_uart_handler.o: turnoutboss_uart_handler.c  .generated_files/flags/default/7574cd09815d389e90736b797a9ac3671e57c208 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_uart_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_uart_handler.c  -o ${OBJECTDIR}/turnoutboss_uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_main.o: turnoutboss_main.c  .generated_files/flags/default/5eb9d211b788a27446189de05e5ca24599529e01 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_main.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_main.c  -o ${OBJECTDIR}/turnoutboss_main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_loader_app.o: ../TurnoutBossCommon/common_loader_app.c  .generated_files/flags/default/1fb5723fd68c39007c940d8e1625ed75779aab01 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_loader_app.c  -o ${OBJECTDIR}/_ext/598189670/common_loader_app.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_loader_app.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_debug_helper.o: ../TurnoutBossCommon/common_debug_helper.c  .generated_files/flags/default/1da1fbebe274b14d5e2b527ac828bbd7a31be8eb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_debug_helper.c  -o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/reset_vector.o: reset_vector.s  .generated_files/flags/default/e57b54d9ec1ce4a674eaefca5137de90fd140c0e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/reset_vector.o.d 
	@${RM} ${OBJECTDIR}/reset_vector.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  reset_vector.s  -o ${OBJECTDIR}/reset_vector.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/reset_vector.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/reset_vector.o: reset_vector.s  .generated_files/flags/default/da3b5676bf2e0022619f425e122fce288dffbf28 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/reset_vector.o.d 
	@${RM} ${OBJECTDIR}/reset_vector.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  reset_vector.s  -o ${OBJECTDIR}/reset_vector.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/reset_vector.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    p33EP512GP504_TurnoutBoss.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   p33EP512GP504_TurnoutBoss.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DMPLAB -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
	@echo Normalizing hex file
	@"/Applications/microchip/mplabx/v6.20/MPLAB X IDE v6.20.app/Contents/Resources/mplab_ide/platform/../mplab_ide/modules/../../bin/hexmate" --edf="/Applications/microchip/mplabx/v6.20/MPLAB X IDE v6.20.app/Contents/Resources/mplab_ide/platform/../mplab_ide/modules/../../dat/en_msgs.txt" ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.hex -o${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.hex

endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
