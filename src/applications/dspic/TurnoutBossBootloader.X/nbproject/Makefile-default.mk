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
SOURCEFILES_QUOTED_IF_SPACED=../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../dsPIC_Common/ecan1_helper.c mcc_generated_files/memory/flash.s ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/application_callbacks.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/application.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c local_drivers/_25AA1024/25AA1024_driver.c turnoutboss_drivers.c main.c node_parameters.c traps.c uart_handler.c debug.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/mcc_generated_files/memory/flash.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/turnoutboss_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/debug.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d ${OBJECTDIR}/_ext/954557028/driver_can.o.d ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/application.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/turnoutboss_drivers.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/uart_handler.o.d ${OBJECTDIR}/debug.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/mcc_generated_files/memory/flash.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/turnoutboss_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/debug.o

# Source Files
SOURCEFILES=../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../dsPIC_Common/ecan1_helper.c mcc_generated_files/memory/flash.s ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/application_callbacks.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/application.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c local_drivers/_25AA1024/25AA1024_driver.c turnoutboss_drivers.c main.c node_parameters.c traps.c uart_handler.c debug.c



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

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script="p33EP512GP504_TurnoutBossBootloader.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/cf9cc645fff3926003412f0c6bc31c1031ab0654 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/cb97c7b7ba31a59422bc8969f2cbfc762d3065f5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/65531103a4cfa9ac9c5215a4abb8e5028ad40a51 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/dca6d7aa837830a3bedb4204d7797b5fe0166914 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/13259a23e1a51992616221c1d2b726a8b0254ac4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/612da13867c5ef24ccac12f8b830dd36a15c04be .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/9270a66b01bc3e6c95b676bf12d320f7f26749a8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/c45ea42ba1de1b9850f27aa7bfd1383262797769 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/d260f7aa85677dc8573d488d6fba730ee1a7c561 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/bc79435eb194765517b08147bb55a716ccc84c2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/3ff64544c09807fe363f4bb77fa4aec58c17b442 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/af69e13dce4eff1e9f8cdfbb43c94785d1c67e07 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/62404fa5dff581288b151e7037f0f7f70fa1d31f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/d02d5bc19772eb30fc998734bcd16303c63fab91 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/d248e7ce30ee08173217301f0e5e05725c6916a2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/425a23e06f433ccb64ac2c78f15d7901053fa629 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/da3bdb9eb2367d3314f8a59558b659d700ba2033 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/7404184f9fdd89f079d3eee1ba5b077b6976c092 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/ea9c26d57eee63e1c9ecb3a4fa386f3e983da800 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/c3b302acc55b8a594ca83f922cce16c101c70b75 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/20f97919b986a6b0f4de9cb60710e1535d27bdb9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/906965571413bc6e70b52d2981ffb97260793f98 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/4a9171f2a6aedbd0b0b53e3ab4be0049306637d1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/964f0fa232311af198bac4476427183806d80a37 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/1f34602c8c3790dab8dc621d53018e87845e1bfd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/d458be966704d6924922c73e3b4c7f59c5508b3c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/f8fc31b0602a13688e8b820ec0545f0a1b9ab8c6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/ae32baf022817e86c0bea08440cbe5def5ef7023 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_drivers.o: turnoutboss_drivers.c  .generated_files/flags/default/ea725087370e91248f6b9ca61215e055be01d138 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_drivers.c  -o ${OBJECTDIR}/turnoutboss_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/869b4742e8d695e6763bad5511e611d494faab16 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/ac71a55d5475b611202cb3c97fe0408f81821916 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/4a6631e3adc1346bc81e0d4f5b7f413d097469b9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/ca913957af231fa32ee66a0bb7daec6465c5c95 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/5fcb2b95703e3d8454fce419adc5bdeb2f4b1612 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/1c1bf86d70a7081734dda455c644a3904052630b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/747131d9187b3034b8d154602debfead345b63d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/6056a7be0693cfa99a90c43c33855f12ee9c8f49 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/36b36f9b0224599e7c47211fe80113a847af8066 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/d073a5f912b2336850b2a5cf44a9c1002c6ad5e2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/d2048c58a5aae43e6abace7e57c8965798246e46 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/ddfdb914e620ff781148524ad9cf7c81304fbe46 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/7f6f0ca4af9b9957e1c52efd5bdc68583356983 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/88561032fe8caca58aed995b99f44cf0d8690c57 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/f50e3ddc5d363bf5d3e10c286fbfa7719c72e564 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/acd0a37d692332948a22b3986f81fa5c425a762c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/838fbc272a3170b6d500c4919cfbe146b09a9471 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/a0b83dadb3a77829b731f12b223d7ccd35073d3e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/3a6f27c2dc4f50f76268d4081e7f6aab3afcf0ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/5b33069c518c721f5c7d0e448ac5d12eb82b3862 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/67beab3f26d81a28e377653218c399cf46c5cf1a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/d9c7e5b10c5d6c3fbdc3e5827357737ec47c4d8a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/e4926e842965e921a194b010e929dbe07980be1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/9cd95a2a0e31ac2b720277f5828ddb2d6ae8e5f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/9f5f1c34de603520b0925aa2034e6d09ce0aec2a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/9929374b55c0c99f496bf84397d644135616c5ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/822866a83dd110e64da1291a8a5bb552c5dd23af .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/a3abcd0c2596cb224f1d4865346b6e6beab49270 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/38621f0d240dc5766ebef02a28e09aec3e378007 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/3a1ab85a3708de7e6fd55a570804bae446905db3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/b01177c1cd13997df9ede4bf500cafd8c644c750 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/63778fd7a5c1a857144d13c51b4b31e0b4d06f3a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/c71bb076901faeed091f2d39da27726c9944d48f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_drivers.o: turnoutboss_drivers.c  .generated_files/flags/default/5766b91a5a609b63955e55ec58c2419f5392ae12 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_drivers.c  -o ${OBJECTDIR}/turnoutboss_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/63d25c98faaa3cd959f783a0c09bed03bac66d5f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/b7a24dd3be1c2a851444b58664edf677c49a216a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/3b899d3ade6d76a3a796f5fe89b1fac736dc2817 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/9a70609ab3da0df32d2a8a313dda24e145d55e22 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/6f07b7bc283763b332181d3b7ebc4e2311e6a983 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/mcc_generated_files/memory/flash.o: mcc_generated_files/memory/flash.s  .generated_files/flags/default/efdaccd17a78a1b2058af2ba6ff14e4f30970c39 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files/memory" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  mcc_generated_files/memory/flash.s  -o ${OBJECTDIR}/mcc_generated_files/memory/flash.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/mcc_generated_files/memory/flash.o.d",--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/mcc_generated_files/memory/flash.o: mcc_generated_files/memory/flash.s  .generated_files/flags/default/5315ed2203606d813775a3b9886c99b0f16b505e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/mcc_generated_files/memory" 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o.d 
	@${RM} ${OBJECTDIR}/mcc_generated_files/memory/flash.o 
	${MP_CC} $(MP_EXTRA_AS_PRE)  mcc_generated_files/memory/flash.s  -o ${OBJECTDIR}/mcc_generated_files/memory/flash.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    -Wa,-MD,"${OBJECTDIR}/mcc_generated_files/memory/flash.o.d",--defsym=__MPLAB_BUILD=1,-g,--no-relax$(MP_EXTRA_AS_POST)  -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   p33EP512GP504_TurnoutBossBootloader.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DMPLAB -DSUPPORT_FIRMWARE_BOOTLOADER -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=4 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
