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
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/efd19351c84593097e3e24f7dd3a43726c209c10 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/bdf1ee6726f1ec840b16fbd11a4c7e54ce285dda .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/503e02fbc8023a5e62294e8d2ea711139e97708c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/8caa3b32378f246d8c1778b19653573c8e488c3b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/206a8e2dffd0cfc776c6ff71e28dd7be6bb91324 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/e13fa5ac485d7c28e13af6e62172ab3863a49d9d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/1119b4095a23c566ff0ff87fb017f071d5f1cd38 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/e47d476a8134f419a670c227b8014c9d0c5ce883 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/d0b8e567bae327df21feb8e86c01138d20917028 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/6662264adcf017569f51a038e440896373329e8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/1ef5813a73fbdaebe96cab5d0f1267c97be4b6f0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/5fa35411f64937c6839f2ec0ee3d2d4905b13b74 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/46b1372ff2f17a49517f0ac1fe37a439f869ab02 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/e5c7a4f792bf18383c6cadcea7501b4be96fad28 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/cb071dacefb271585a46bb63fad0753335e56c67 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/48d8fc21cb0690ec21e4699812f29c420a44aaa5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/52f947408da89c7c953ef108e06b742e09a1602e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/f42d063bc19c3e96ec72863ba7833d793a5ba25a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/2258f1606a7a25b1e7c482551be98d4d0d30c306 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/3da912810daf12727845b515ddf586db17564171 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/4633219b6ed2637faa290c6ecae7a338df1fa00d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/fda5771b4e758f4907ed31aae9de9e1cb935074a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/9952e692767e9d6deb0b82af33bb83454d855563 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/38fd353653fa25360c96e5e695f96442db534fcb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/7b931bda78a6bc1039b33e7685b087a963c4c405 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/6d4e5548e7dfb60247a3034bbfc0aeeac73ea1fd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/8ca62b1493ad17aff9f195927905a4e55fee60e2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_drivers.o: turnoutboss_bootloader_drivers.c  .generated_files/flags/default/328e29aed4a48d726caa730ceaa4ad135ee202ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_drivers.c  -o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_traps.o: turnoutboss_bootloader_traps.c  .generated_files/flags/default/85868c01fd91077f560025fac30161d1f49029ef .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_traps.c  -o ${OBJECTDIR}/turnoutboss_bootloader_traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o: turnoutboss_bootloader_uart_handler.c  .generated_files/flags/default/cdf50c8841e3b25f13a419a9100bd1a83479cd7e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_uart_handler.c  -o ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o: turnoutboss_bootloader_node_parameters.c  .generated_files/flags/default/b0fc04bcf27cb75297e028d6cb2df86bd49ff45c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_node_parameters.c  -o ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o: turnoutboss_bootloader_ecan1_helper.c  .generated_files/flags/default/ba28b3f1cc3edd39534c93a35fd035e3ae60491e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_ecan1_helper.c  -o ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_main.o: turnoutboss_bootloader_main.c  .generated_files/flags/default/536bbb6555cd2d50388608cd0811e5e2ffd1b631 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_main.c  -o ${OBJECTDIR}/turnoutboss_bootloader_main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o: turnoutboss_bootloader_hex_file_statemachine.c  .generated_files/flags/default/66f378fdc85cb2857c345aa4376853eb115ea878 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_hex_file_statemachine.c  -o ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_loader_app.o: ../TurnoutBossCommon/common_loader_app.c  .generated_files/flags/default/ceed62d5abc9c0e5e995e822001a3994009883c7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_loader_app.c  -o ${OBJECTDIR}/_ext/598189670/common_loader_app.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_loader_app.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_debug_helper.o: ../TurnoutBossCommon/common_debug_helper.c  .generated_files/flags/default/1502e21c9562db4da0006a3df480e0edfd48ddf6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_debug_helper.c  -o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/70127ddd990b3519baa134bd86eca37cdc173584 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/70b142901e2742657ba2b9dc83f62a21184595f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/90f947d4810391e8fb7f0317c7a9a282cf8e09d0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/65d0d9b99eba959b5006fbbee3eadd6d6fc84cb4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/8433c7316cd15abea93390f25b024cb44722b859 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/bc86780a5372470980efb3a16e1a8cd762b593eb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/9796d64dd3dac8b46cead4cc63518f2bcfb5b0f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/995be4fa3ae1ddc11bcc17e00f98f9a431a2a41 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/b5f0ba98c812cbc0b23abaeeaecbaf0ae7d33c77 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/5027cf78a6c065c4bd67bd02ea16f02e4176871f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/acc5ad3580449afb911c12954323e562dd02a1bf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/f8805e7470eacb881bcf1d8d37f656a4224d9136 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/dfc9fe146e14d6122048ad98afa3ebfc93aa70d1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/4aaa8a50079e06d8e1bfe620cf4fd4e6c589076c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/2866920f15ffb97c44a9ea5db0b8618649bc2b06 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/8a137b47ad255728acac93931a25173f48972e36 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/1ba72e2a285a1563a56230013e0db8cf392604a8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/87e2c0e75f03e48c7d7994499b2f00d443740cea .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/b069ab9da0627e434c75c3800c04d0b9ae2c2aeb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/e96322c5d2046c6382ae81d67573ac2b6c571e58 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/d6eb2c00eca86d793aee975ceebb52acee4ea828 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/8a7fb95d498e4c5f16706801d1653561c0533187 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/a3efd7f27e92a64608a0201eaff2009d2cdb67d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/38a390cdbbf3bc1cb35584be1c3ee4ea0d9e6874 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/ca35bafbd2d223007beaf5beb5927c9c053c0d27 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/c3b8dd1a88aec6f072e578153b7d185b64594f47 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/c089d095d261189277e835237959286a240a97cb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_drivers.o: turnoutboss_bootloader_drivers.c  .generated_files/flags/default/2947e5a812aa5b24d441d2a2eb42843cfa1b1674 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_drivers.c  -o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_traps.o: turnoutboss_bootloader_traps.c  .generated_files/flags/default/7fc63c5355d41415806eb16aa5003530b30f7253 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_traps.c  -o ${OBJECTDIR}/turnoutboss_bootloader_traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o: turnoutboss_bootloader_uart_handler.c  .generated_files/flags/default/b35cede7830afaf267ca557c9d6386c4ca9bd2bb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_uart_handler.c  -o ${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o: turnoutboss_bootloader_node_parameters.c  .generated_files/flags/default/4381551d87178cbe546dbbf6dd83ef2c4b992ce0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_node_parameters.c  -o ${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o: turnoutboss_bootloader_ecan1_helper.c  .generated_files/flags/default/5c3e98a09447a5b110782c05a920b6362edce87c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_ecan1_helper.c  -o ${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_main.o: turnoutboss_bootloader_main.c  .generated_files/flags/default/8ce855fe2cd8ebd392936c0225504a7b4afb1aaa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_main.c  -o ${OBJECTDIR}/turnoutboss_bootloader_main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o: turnoutboss_bootloader_hex_file_statemachine.c  .generated_files/flags/default/2b55cf2b182cf14c9eaf856560cb1f9848072041 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_hex_file_statemachine.c  -o ${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_hex_file_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_loader_app.o: ../TurnoutBossCommon/common_loader_app.c  .generated_files/flags/default/15294db4097b3e9a6c616f3327c7cb9f70db3b72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_loader_app.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_loader_app.c  -o ${OBJECTDIR}/_ext/598189670/common_loader_app.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_loader_app.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/598189670/common_debug_helper.o: ../TurnoutBossCommon/common_debug_helper.c  .generated_files/flags/default/648bd3443d7625bccd37bce4e0667d215cd4297a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/598189670" 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/598189670/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../TurnoutBossCommon/common_debug_helper.c  -o ${OBJECTDIR}/_ext/598189670/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/598189670/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/mcc_generated_files/memory/flash.o: mcc_generated_files/memory/flash.s  .generated_files/flags/default/bb29be19be1df89a99cf6c95db4485ce6839522e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files/memory" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  mcc_generated_files/memory/flash.s  -o ${OBJECTDIR}/mcc_generated_files/memory/flash.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/mcc_generated_files/memory/flash.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/reset_vector.o: reset_vector.s  .generated_files/flags/default/9b8a34e0427a4e920b9a42eedbf41d516ffa0bc0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/reset_vector.o.d 
	@${RM} ${OBJECTDIR}/reset_vector.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  reset_vector.s  -o ${OBJECTDIR}/reset_vector.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/reset_vector.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/mcc_generated_files/memory/flash.o: mcc_generated_files/memory/flash.s  .generated_files/flags/default/ca7379c97c356d3c1e9c3ad25024292d6fc08a98 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files/memory" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  mcc_generated_files/memory/flash.s  -o ${OBJECTDIR}/mcc_generated_files/memory/flash.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/mcc_generated_files/memory/flash.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/reset_vector.o: reset_vector.s  .generated_files/flags/default/5a5812d18c2c83fe765610371886225f54c60c26 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/reset_vector.o.d 
	@${RM} ${OBJECTDIR}/reset_vector.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  reset_vector.s  -o ${OBJECTDIR}/reset_vector.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/reset_vector.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   p33EP512GP504_TurnoutBossBootloader.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DBOSS2 -DSUPPORT_FIRMWARE_BOOTLOADER_ADDRESS_SPACE -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
