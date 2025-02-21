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
FINAL_IMAGE=${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c mcc_generated_files/memory/flash.s ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/application_callbacks.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/application.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c local_drivers/_25AA1024/25AA1024_driver.c turnoutboss_bootloader_drivers.c turnoutboss_bootloader_traps.c turnoutboss_bootloader_uart_handler.c turnoutboss_bootloader_node_parameters.c turnoutboss_bootloader_ecan1_helper.c turnoutboss_bootloader_main.c turnoutboss_bootloader_hex_file_statemachine.c reset_vector.s ../TurnoutBossCommon/common_loader_app.c ../TurnoutBossCommon/common_debug_helper.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/mcc_generated_files/memory/flash.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o ${OBJECTDIR}/turnoutboss_bootloader_traps.o ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o ${OBJECTDIR}/turnoutboss_bootloader_main.o ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o ${OBJECTDIR}/reset_vector.o ${OBJECTDIR}/_ext/598189670/common_loader_app.o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d ${OBJECTDIR}/_ext/954557028/driver_can.o.d ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/application.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d ${OBJECTDIR}/turnoutboss_bootloader_traps.o.d ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d ${OBJECTDIR}/turnoutboss_bootloader_main.o.d ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d ${OBJECTDIR}/reset_vector.o.d ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/mcc_generated_files/memory/flash.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o ${OBJECTDIR}/turnoutboss_bootloader_traps.o ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o ${OBJECTDIR}/turnoutboss_bootloader_main.o ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o ${OBJECTDIR}/reset_vector.o ${OBJECTDIR}/_ext/598189670/common_loader_app.o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o

# Source Files
SOURCEFILES=../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c mcc_generated_files/memory/flash.s ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/application_callbacks.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/application.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c local_drivers/_25AA1024/25AA1024_driver.c turnoutboss_bootloader_drivers.c turnoutboss_bootloader_traps.c turnoutboss_bootloader_uart_handler.c turnoutboss_bootloader_node_parameters.c turnoutboss_bootloader_ecan1_helper.c turnoutboss_bootloader_main.c turnoutboss_bootloader_hex_file_statemachine.c reset_vector.s ../TurnoutBossCommon/common_loader_app.c ../TurnoutBossCommon/common_debug_helper.c



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
ProjectDir=/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossBootloader.X
ProjectName=TurnoutBossBootloader
ConfName=default
ImagePath=dist/default/${IMAGE_TYPE}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
ImageDir=dist/default/${IMAGE_TYPE}
ImageName=TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: ["/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -addressing=2 -o/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/turnoutboss_merged.hex ]"
	@"/Applications/microchip/mplabx/v6.20/mplab_platform/bin/hexmate" r0xB000-0x55800,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBOSS.X/dist/default/production/TurnoutBOSS.X.production.hex" r0x0000-0xB000,"/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/TurnoutBossBootloader.X/dist/default/production/TurnoutBossBootloader.X.production.hex" -addressing=2 -o/Users/jimkueneman/Documents/OpenLcbCLib/src/applications/dsPIC/turnoutboss_merged.hex 
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script="p33EP512GP504_TurnoutBossBootloader.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/374c6e44ae0cbcd2d5be1f1877ac46ac74013860 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/1f0000a53ab5b4a9137d82a044c6e7e4d27d58e2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/6cc92a6d3747c6883d0d18852f80bbb21137239a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/78cb71cd616141080e8b7d55a700f387164794bf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/281674bcb95714cd1f05dd3c2ebfd9ac91b5f67e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/457891a77bff89ab3aa4ca729aac5bc936760996 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/8968354c0b36cfe11998618f71f006bf027c802c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/ab7e9643a6d0c60ce723e537ccad8d0a14fc3b2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/61070ac6bdde420dcc5843270f3748e8814b2b9e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/e4e15b671a35df6ac50e838723719ce508b1ab57 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/4e24ae10282243f2df0557f7046bce8cff57926f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/80d732f1052de18689221a84b21301d22d598c5c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/9bb857d7ecb329f8d91f9df65946cc3459fe5f28 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/81b727a3c4962488d48b53d211901379190c29ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/8d1c1eb62e0e13845f0c887f1689bdd7079f07c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/25d882c3a7871408d881be5aba3a051bec39c6ee .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/e03a5430a828442db9275d1ab4302783830399a9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/42aabfa0f81365ed8b07910c2ec5767f62ca15d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/86c7eb6f053fea243e32dc0b352f1c01f7993821 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/bb68e955edc91212d0ab824b878ebd5ec38116e4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/4499a3b59017ed94cf654162f147a0120a406d2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/ba37da7e04597ba5fa25ce38cf3b8b988b5e8418 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/b81a8df26e225cca131167260eb0d5e2c2d87a57 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/bae8a3c24432f963ebdd06d030522f065b48b32d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/2182e09cef2d2f21d6871a83177c4e53e9047a8b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/aff9cbd6cc2317e07267f34c8573958988e8cd94 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/164b29ee43b2de4b8eacc3a75f120a3b2a7b75d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_drivers.o: turnoutboss_bootloader_drivers.c  .generated_files/flags/default/cbe424ab938a868c7a9b08e9c8b0cd7a591e1f34 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_drivers.c  -o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_traps.o: turnoutboss_bootloader_traps.c  .generated_files/flags/default/33829900d2eedd6661d5e8c696db36dec3ab7858 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_traps.c  -o ${OBJECTDIR}/turnoutboss_bootloader_traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o: turnoutboss_bootloader_uart_handler.c  .generated_files/flags/default/6927e9d5407a9f77cded1c6f6c46d136521a5b2b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_uart_handler.c  -o ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o: turnoutboss_bootloader_node_parameters.c  .generated_files/flags/default/fd849a80341c6b4786603e59d2f2ccb85faf8772 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_node_parameters.c  -o ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o: turnoutboss_bootloader_ecan1_helper.c  .generated_files/flags/default/3d30f64746fb86f25e90bb4eb31c30827142e79b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_ecan1_helper.c  -o ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_main.o: turnoutboss_bootloader_main.c  .generated_files/flags/default/543329a5b6484cc3d9c93e3610999e0feb71160f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_main.c  -o ${OBJECTDIR}/turnoutboss_bootloader_main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o: turnoutboss_bootloader_hex_file_statemachine.c  .generated_files/flags/default/7075c0604d906f14b5b058944c376babcbd53a22 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_hex_file_statemachine.c  -o ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_loader_app.o: ../TurnoutBossCommon/common_loader_app.c  .generated_files/flags/default/a0aaf5beb1d9f781e2909cf8ed5d1211107ebeac .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_loader_app.c  -o ${OBJECTDIR}/_ext/598189670/common_loader_app.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_loader_app.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_debug_helper.o: ../TurnoutBossCommon/common_debug_helper.c  .generated_files/flags/default/f326151ab842486d63f5d705b22a800c0cdf101c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_debug_helper.c  -o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/190c066838bf49a027dbca6ef36083caaf20510d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/8b9f9843b7641259947e7e2823eda02c234d5a40 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/9065502a9b32c4cf75bc054a57edd70da848b23c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/d6399c331bc7f7c44287a33951ba63169d9376ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/93e3430926ee25fa187c2c7da22b668117552bc0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/551bf83412a480eade27428fcafe13b0187517df .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/6fe6f500692a675370a3dee24cc058cd4d068d7e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/9a025c0db6ae7c6b9c70aa4b1cf76ae64073edbc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/528f4c372a4b3bf0c451a0ab00ba631004632343 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/e34cfa4f0b7c033de16d1d1dfa69957e663eb5c7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/fb6e550ae84230c37d5618e82a86816b1800fe9c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/f04c259598e6a357b9195fbcf0f041009c5fd978 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/9feff644084621de1c0c79b0fdf59e3615539b54 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/c18cae1d211a626719b1e41d2c04c4b81300cd18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/13d93edc3d0ef9ea101ed683349445af6453fb49 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/905546bc145d00d1e7588adc57fabae62bf291e6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/7e48c6c62f4c6f74ef34c3c51a15e37922d8d753 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/95d864265a8dd367249efd05b2e963fa368749ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/32b9b160161558004fc76c0abc143f9a784c721e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/8cfaf5e5c6adab926503346bfc52c528aae40df8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/ad45e719343b1bf31fa4c50765e99c9f2e183560 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/eb3f0c5c8093202d94209d94780cf30908dffaf7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/c01184ae21ccf209535777eb7bbbc01a2cd8bf43 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/dddeb74e951115fc18936066f4719f1da51f9b4f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/8b60125662b62d5f31bc5920b4866eb451747a73 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/e5a41afbc4d7810dd505902583573cca07404c54 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/abc9e49c4bc58456b14ca2630b229afd602632da .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_drivers.o: turnoutboss_bootloader_drivers.c  .generated_files/flags/default/83bcb945de5e52c43f65779cafbf3f35f3e2cb4d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_drivers.c  -o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_traps.o: turnoutboss_bootloader_traps.c  .generated_files/flags/default/a9047d029434d5846e9e8de8a320c0b0e01dc8a1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_traps.c  -o ${OBJECTDIR}/turnoutboss_bootloader_traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o: turnoutboss_bootloader_uart_handler.c  .generated_files/flags/default/412e531cf649d2961c3f29146c61898b48bf997f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_uart_handler.c  -o ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o: turnoutboss_bootloader_node_parameters.c  .generated_files/flags/default/eeb336110a90c491f65b32fa9b9ff6eee7a00a9b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_node_parameters.c  -o ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o: turnoutboss_bootloader_ecan1_helper.c  .generated_files/flags/default/bd17018ac45cd11a4476aba4dcdeee89a377518 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_ecan1_helper.c  -o ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_main.o: turnoutboss_bootloader_main.c  .generated_files/flags/default/1da2d8d464bfc9e5fa328d435c030c5d969eb551 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_main.c  -o ${OBJECTDIR}/turnoutboss_bootloader_main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o: turnoutboss_bootloader_hex_file_statemachine.c  .generated_files/flags/default/9dc58abc102cbd965e260b451839d45c37914a03 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_hex_file_statemachine.c  -o ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_loader_app.o: ../TurnoutBossCommon/common_loader_app.c  .generated_files/flags/default/c2187801796bc6a9f16c3f799b0712c8494019a9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_loader_app.c  -o ${OBJECTDIR}/_ext/598189670/common_loader_app.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_loader_app.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_debug_helper.o: ../TurnoutBossCommon/common_debug_helper.c  .generated_files/flags/default/336b0225742ffac2a2e73c28b4a3f17fd09753f8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_debug_helper.c  -o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/mcc_generated_files/memory/flash.o: mcc_generated_files/memory/flash.s  .generated_files/flags/default/17fecb1aa6d7db27914d0f3473d18e3517584d1e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files/memory" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  mcc_generated_files/memory/flash.s  -o ${OBJECTDIR}/mcc_generated_files/memory/flash.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/mcc_generated_files/memory/flash.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/reset_vector.o: reset_vector.s  .generated_files/flags/default/bb8268d20f3fba598de59aa850c99d2ad5a7c1b0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/reset_vector.o.d 
	@${RM} ${OBJECTDIR}/reset_vector.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  reset_vector.s  -o ${OBJECTDIR}/reset_vector.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/reset_vector.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/mcc_generated_files/memory/flash.o: mcc_generated_files/memory/flash.s  .generated_files/flags/default/5737a55a4b421483fd32b9da45f65904aab30cc9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files/memory" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  mcc_generated_files/memory/flash.s  -o ${OBJECTDIR}/mcc_generated_files/memory/flash.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/mcc_generated_files/memory/flash.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/reset_vector.o: reset_vector.s  .generated_files/flags/default/b389b664d703694ee03ba760c028c0e7a06d3b0b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/reset_vector.o.d 
	@${RM} ${OBJECTDIR}/reset_vector.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  reset_vector.s  -o ${OBJECTDIR}/reset_vector.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/reset_vector.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    p33EP512GP504_TurnoutBossBootloader.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   p33EP512GP504_TurnoutBossBootloader.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS1 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
	@echo Normalizing hex file
	@"/Applications/microchip/mplabx/v6.20/MPLAB X IDE v6.20.app/Contents/Resources/mplab_ide/platform/../mplab_ide/modules/../../bin/hexmate" --edf="/Applications/microchip/mplabx/v6.20/MPLAB X IDE v6.20.app/Contents/Resources/mplab_ide/platform/../mplab_ide/modules/../../dat/en_msgs.txt" ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.hex -o${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.hex

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
