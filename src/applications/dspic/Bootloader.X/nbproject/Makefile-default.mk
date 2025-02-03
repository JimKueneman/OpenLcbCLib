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
FINAL_IMAGE=${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=bootloader_drivers.c main.c node_parameters.c uart_handler.c local_drivers/_25AA1024/25AA1024_driver.c debug.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/application_callbacks.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/application.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/bootloader_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o
POSSIBLE_DEPFILES=${OBJECTDIR}/bootloader_drivers.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/uart_handler.o.d ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/application.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d ${OBJECTDIR}/_ext/954557028/driver_can.o.d ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/bootloader_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/application_callbacks.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/application.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o

# Source Files
SOURCEFILES=bootloader_drivers.c main.c node_parameters.c uart_handler.c local_drivers/_25AA1024/25AA1024_driver.c debug.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/application_callbacks.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/application.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script=p33EP512GP504.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/bootloader_drivers.o: bootloader_drivers.c  .generated_files/flags/default/bda83e6641d6064499241e3d8667b29ae57d99f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bootloader_drivers.c  -o ${OBJECTDIR}/bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/bootloader_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/b289f51721cd496bf339dbf2befd339104c48926 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/e7d8d40b07225ead34dd6aff69a763332626f5a4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/c871b05daed84df95f6654401da1a37df19691ef .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/16539190b852f652383bba95f9b903bdc5f997d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/db385c38f30d0c38e7e60aeeeda1d05f122a4c04 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/2e064d4a6ffed4d660245886677604e8f0264171 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/6e2e2528894f561b82cd62a281215afc24d53de1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/63e515f1f14cddf0d0a4c7dc697db0d137275014 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/7197cbd9e238998e9238168224be8127d03e5720 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/d0d11af6455326c368a074bc535e2f025c7b05ac .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/fa975c182632be0dd4983b0912ba2d0f6dcc6491 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/4cad18b1ee7fa16dae4cadc32b815edfd0a6b5c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/7d6eadb0c1200c861f9411512883660352c1e2ab .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/f2e623c2ee4249ff9d82ced80d453ad1fc676357 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/8762ebd1d1678e3fbe7551f21815f6fd484b1e78 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/7b15305f841c699683d2292e884da93957b5ca42 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/773b9019ceb29c2b27cdfd35f856f711fd07c48e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/d6d92ea1d91af5ce2357266ae762c1f620e17a23 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/1273a5a9cf05e1f39c9e68d09815cc468fea7b97 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/c26380081e825295aad922f71ccdbaba63aef977 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/20ef6ff35fe5041e6c210781dfd681ff008cbf20 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/fe6a04abac8e6406dec9f25da1d2a8b5cdcd9f09 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/c3d28daca6347b04c03305069e3b403f025ae25b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/f44c4143b69e924b71bf6d1118e6c8fa97415b64 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/f179423f6a6e9085cefce85bbaa6b6ec230f163e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/59a8efb2302fd15cb03a7f258b25a2df7cf2f600 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/9889691eb4394cce8994e9d6437801790dbec202 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/88b0d946d0e3e80eb64c7d48e3d0853747428604 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/9ee927756667f900dc13f55fdff884d1d3f4fd28 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/8e7b097167ceedd121a6794dd0dbc6d99ec2b3eb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/4918627af6d68290ef27d7dbd8e8964ce98d7590 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/6d29493477c90cfe8d4f7daed8fa097f8678c2b9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/bootloader_drivers.o: bootloader_drivers.c  .generated_files/flags/default/982d77b685b049f7ad8ad829e4e1d20bea21a1d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bootloader_drivers.c  -o ${OBJECTDIR}/bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/bootloader_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/cf4ccdac3cff7d0707e608494942e7a4330788d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/81f6158c01555ba4c00fba8925e6c0daf963155b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/ea3b3006e6b22a2fb533dc7acf2033b77aaae4a1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/3fb35210e0a6fa9a2b79e2ec743cd7b877dda040 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/9df38ba64a6799b41abbfd078df11ff43987f339 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/588cbb8c24204cb6caf16cca2c3a6b88b3bce9ef .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/25bc20fe51bc87e00c5b46576b5b1872421b2306 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/a63b68cf9e051e71482a14ff14008675e0c39c5c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/65d7d3d8e5cc149a3710c863aae84628d3b59f3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/463eb7a79d9fb9316e08b0fb74dbda2dfdef8f4e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/1e7896801c9a29e9dd6b51b6b595cd3f390f9b58 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/737c0fe51c7f32a1d809a73f8eb2f992e70145b2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/75930fd11f8590ca852ac2926574a03bc9a5d12e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/eda2507d7512ba1cf00818e28209fbcb17c0cd7b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application_callbacks.o: ../../../openlcb/application_callbacks.c  .generated_files/flags/default/bc4ded35d69aa688064f3dfba0064a562ec75a7c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application_callbacks.c  -o ${OBJECTDIR}/_ext/156864402/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application_callbacks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/3eb3ef931123d9b728176ff49349cee8928d4784 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/1143bee26863a649143104d7476d5a2d4de8a22a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/application.o: ../../../openlcb/application.c  .generated_files/flags/default/e9857093b818dee559f1c145d0c2de00aeb41766 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/application.c  -o ${OBJECTDIR}/_ext/156864402/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/fccac638d6093e2031fec3634bc9511e28ed5a2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/7b303791248c3df42dd78ca84beb6d86ec66eda3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/bcd96538a4aa48db6b1afc52ce494391c98580db .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/e2e38ff2d463ebbd5f5476f3be60b29913681de3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/a62a4594394c9eb5ce09aff21996e2db8069c1ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/27ed8fb3ffc31a87879e9c1758f366ecd017caaa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/a65d1c53dc05dcf9a7e34573784e0c689c81db82 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/4b4a06920c2f71ef75e94eef27a32d9b6859caca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/6c940ecd2cb9a684600c345753c4303aa1bbba53 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/395df888822b9e26c7b7b7b4a523bb7e0296e4a1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/be6db12ebb4dc4d35a0698b701aae050dbd9cffe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/b5a9b8b1902d4d778dfda6c23c67f0997d247287 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/ef4c2ccd2028d4195a7b24d412747ac3a16ef71c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/a02323fd60c31d237a4ec576689df95a62ab5784 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)   -mreserve=data@0x1000:0x101B -mreserve=data@0x101C:0x101D -mreserve=data@0x101E:0x101F -mreserve=data@0x1020:0x1021 -mreserve=data@0x1022:0x1023 -mreserve=data@0x1024:0x1027 -mreserve=data@0x1028:0x104F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
