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
SOURCEFILES_QUOTED_IF_SPACED=local_drivers/_25AA1024/25AA1024_driver.c debug.c bootloader_drivers.c main.c node_parameters.c uart_handler.c src/drivers/ecan1_helper.c src/drivers/driver_mcu.c src/drivers/driver_can.c src/drivers/driver_100ms_clock.c src/drivers/driver_configuration_memory.c src/drivers/common/can_buffer_fifo.c src/drivers/common/can_main_statemachine.c src/drivers/common/can_login_message_handler.c src/drivers/common/can_utilities.c src/drivers/common/can_frame_message_handler.c src/drivers/common/can_rx_statemachine.c src/drivers/common/can_tx_statemachine.c src/drivers/common/can_buffer_store.c src/openlcb/openlcb_buffer_fifo.c src/openlcb/openlcb_node.c src/openlcb/protocol_datagram.c src/openlcb/openlcb_buffer_store.c src/openlcb/openlcb_utilities.c src/openlcb/protocol_event_transport.c src/openlcb/openlcb_buffer_list.c src/openlcb/protocol_snip.c src/openlcb/application_callbacks.c src/openlcb/openlcb_gridconnect.c src/openlcb/openlcb_main_statemachine.c src/openlcb/application.c src/openlcb/protocol_message_network.c src/openlcb/openlcb_tx_driver.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/bootloader_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/src/drivers/ecan1_helper.o ${OBJECTDIR}/src/drivers/driver_mcu.o ${OBJECTDIR}/src/drivers/driver_can.o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o ${OBJECTDIR}/src/drivers/common/can_utilities.o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o ${OBJECTDIR}/src/openlcb/openlcb_node.o ${OBJECTDIR}/src/openlcb/protocol_datagram.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o ${OBJECTDIR}/src/openlcb/protocol_snip.o ${OBJECTDIR}/src/openlcb/application_callbacks.o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o ${OBJECTDIR}/src/openlcb/application.o ${OBJECTDIR}/src/openlcb/protocol_message_network.o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o
POSSIBLE_DEPFILES=${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/bootloader_drivers.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/uart_handler.o.d ${OBJECTDIR}/src/drivers/ecan1_helper.o.d ${OBJECTDIR}/src/drivers/driver_mcu.o.d ${OBJECTDIR}/src/drivers/driver_can.o.d ${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d ${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d ${OBJECTDIR}/src/drivers/common/can_utilities.o.d ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d ${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d ${OBJECTDIR}/src/openlcb/openlcb_node.o.d ${OBJECTDIR}/src/openlcb/protocol_datagram.o.d ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d ${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d ${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d ${OBJECTDIR}/src/openlcb/protocol_snip.o.d ${OBJECTDIR}/src/openlcb/application_callbacks.o.d ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d ${OBJECTDIR}/src/openlcb/application.o.d ${OBJECTDIR}/src/openlcb/protocol_message_network.o.d ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/bootloader_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/src/drivers/ecan1_helper.o ${OBJECTDIR}/src/drivers/driver_mcu.o ${OBJECTDIR}/src/drivers/driver_can.o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o ${OBJECTDIR}/src/drivers/common/can_utilities.o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o ${OBJECTDIR}/src/openlcb/openlcb_node.o ${OBJECTDIR}/src/openlcb/protocol_datagram.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o ${OBJECTDIR}/src/openlcb/protocol_snip.o ${OBJECTDIR}/src/openlcb/application_callbacks.o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o ${OBJECTDIR}/src/openlcb/application.o ${OBJECTDIR}/src/openlcb/protocol_message_network.o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o

# Source Files
SOURCEFILES=local_drivers/_25AA1024/25AA1024_driver.c debug.c bootloader_drivers.c main.c node_parameters.c uart_handler.c src/drivers/ecan1_helper.c src/drivers/driver_mcu.c src/drivers/driver_can.c src/drivers/driver_100ms_clock.c src/drivers/driver_configuration_memory.c src/drivers/common/can_buffer_fifo.c src/drivers/common/can_main_statemachine.c src/drivers/common/can_login_message_handler.c src/drivers/common/can_utilities.c src/drivers/common/can_frame_message_handler.c src/drivers/common/can_rx_statemachine.c src/drivers/common/can_tx_statemachine.c src/drivers/common/can_buffer_store.c src/openlcb/openlcb_buffer_fifo.c src/openlcb/openlcb_node.c src/openlcb/protocol_datagram.c src/openlcb/openlcb_buffer_store.c src/openlcb/openlcb_utilities.c src/openlcb/protocol_event_transport.c src/openlcb/openlcb_buffer_list.c src/openlcb/protocol_snip.c src/openlcb/application_callbacks.c src/openlcb/openlcb_gridconnect.c src/openlcb/openlcb_main_statemachine.c src/openlcb/application.c src/openlcb/protocol_message_network.c src/openlcb/openlcb_tx_driver.c



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
	
${OBJECTDIR}/src/drivers/ecan1_helper.o: src/drivers/ecan1_helper.c  .generated_files/flags/default/5527d3f6b3d42dacdc746d972836cacb34092ad5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/ecan1_helper.c  -o ${OBJECTDIR}/src/drivers/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_mcu.o: src/drivers/driver_mcu.c  .generated_files/flags/default/deb37aa35e70ba1feb5980ead700379f252a47e5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_mcu.c  -o ${OBJECTDIR}/src/drivers/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_can.o: src/drivers/driver_can.c  .generated_files/flags/default/ab7abe6dbfdbd2988f7a6399638162cf111a6201 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_can.c  -o ${OBJECTDIR}/src/drivers/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_100ms_clock.o: src/drivers/driver_100ms_clock.c  .generated_files/flags/default/cbfcf3c459ecb47234b2d0a3ff64dd6ee5caa3c8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_configuration_memory.o: src/drivers/driver_configuration_memory.c  .generated_files/flags/default/2dac2da4b9702e232a4ccb20e86de0b9372bc250 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o: src/drivers/common/can_buffer_fifo.c  .generated_files/flags/default/ed8ff8c34d77e21c68bc59d1089a5f175b649e13 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_main_statemachine.o: src/drivers/common/can_main_statemachine.c  .generated_files/flags/default/d02142196fa33f20538cbaf5c5106a84793847c8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_login_message_handler.o: src/drivers/common/can_login_message_handler.c  .generated_files/flags/default/6ef260278ad7a76db0b0ca9e601bdac347144836 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_utilities.o: src/drivers/common/can_utilities.c  .generated_files/flags/default/6708de215b123bde82a9ab6b94297edab341cb19 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_utilities.c  -o ${OBJECTDIR}/src/drivers/common/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o: src/drivers/common/can_frame_message_handler.c  .generated_files/flags/default/4e5d9801c5e9d40504dd63e51ac25e9470577227 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o: src/drivers/common/can_rx_statemachine.c  .generated_files/flags/default/f2d21b0624159fdd44283c3fac298626fc075004 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o: src/drivers/common/can_tx_statemachine.c  .generated_files/flags/default/25024817e1bb14bcfc85185a9ca98444c86a97ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_store.o: src/drivers/common/can_buffer_store.c  .generated_files/flags/default/1b854957e4d36963705f00993d756d1888174dd6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o: src/openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/787edf23df290fecd34f8a730cc4d7ffc2cb4d45 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_node.o: src/openlcb/openlcb_node.c  .generated_files/flags/default/9e2ec376e11f04de9ff703a6b7638e3f3c32aa0b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_node.c  -o ${OBJECTDIR}/src/openlcb/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_datagram.o: src/openlcb/protocol_datagram.c  .generated_files/flags/default/c31f19cf004fe0714a4f88d764904f59cfd50be2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_datagram.c  -o ${OBJECTDIR}/src/openlcb/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o: src/openlcb/openlcb_buffer_store.c  .generated_files/flags/default/e78da6286ab2bec4544c46b2d53af845b6fa8510 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_utilities.o: src/openlcb/openlcb_utilities.c  .generated_files/flags/default/6da5f612ac9437ef48ea41efe9f46a2a1ed818b8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_event_transport.o: src/openlcb/protocol_event_transport.c  .generated_files/flags/default/c63fc167d68f02ad7d03dba729f12200d52ad7c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o: src/openlcb/openlcb_buffer_list.c  .generated_files/flags/default/8582579ab8dda8f6e25a9528631f453a5f2dfb99 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_snip.o: src/openlcb/protocol_snip.c  .generated_files/flags/default/34ed7ce0ea0b87264cf701cc1a95564e10614211 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_snip.c  -o ${OBJECTDIR}/src/openlcb/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application_callbacks.o: src/openlcb/application_callbacks.c  .generated_files/flags/default/a4b4897b40dd3a1e8a34e5e98b6e0563d1f8d306 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application_callbacks.c  -o ${OBJECTDIR}/src/openlcb/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o: src/openlcb/openlcb_gridconnect.c  .generated_files/flags/default/be31974e1f17142f7c73b3f0631452ce4370aa49 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o: src/openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/35218af208acbb274708f5d6a98ba55980b25a38 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application.o: src/openlcb/application.c  .generated_files/flags/default/78e6483abf17355255825d56cfc512132a26b2e5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application.c  -o ${OBJECTDIR}/src/openlcb/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_message_network.o: src/openlcb/protocol_message_network.c  .generated_files/flags/default/1df4fb1014ffa2797790d08de0ae545d6b4705cb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_message_network.c  -o ${OBJECTDIR}/src/openlcb/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o: src/openlcb/openlcb_tx_driver.c  .generated_files/flags/default/2f507951711008f22da5c048bd1649133190b2a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
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
	
${OBJECTDIR}/src/drivers/ecan1_helper.o: src/drivers/ecan1_helper.c  .generated_files/flags/default/60cd53b5254f2ed498d932a3093dc3db405bd72a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/ecan1_helper.c  -o ${OBJECTDIR}/src/drivers/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_mcu.o: src/drivers/driver_mcu.c  .generated_files/flags/default/a7ef813c5c45a43691fc2b1657e26c5b43c42c6e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_mcu.c  -o ${OBJECTDIR}/src/drivers/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_can.o: src/drivers/driver_can.c  .generated_files/flags/default/f8fd534e0e683e7e0105332d23dcfa1da4285f71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_can.c  -o ${OBJECTDIR}/src/drivers/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_100ms_clock.o: src/drivers/driver_100ms_clock.c  .generated_files/flags/default/c015bafcffc4cf4e37159d83de2ceab3de7395e3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_configuration_memory.o: src/drivers/driver_configuration_memory.c  .generated_files/flags/default/11cb5a5850599586e9e05e2c64f1166e92c163ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o: src/drivers/common/can_buffer_fifo.c  .generated_files/flags/default/3d90e6e6fa2a0d07afe00aced9eac1801c7d4800 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_main_statemachine.o: src/drivers/common/can_main_statemachine.c  .generated_files/flags/default/1d40a7d025d4e06a34f61b9a6ab5d66852ea689 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_login_message_handler.o: src/drivers/common/can_login_message_handler.c  .generated_files/flags/default/d42bff2f3e6fe17cf4c8d265cb8f47e92db1ce48 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_utilities.o: src/drivers/common/can_utilities.c  .generated_files/flags/default/b007b57bd639d41822a70f8894dead7da3f47d16 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_utilities.c  -o ${OBJECTDIR}/src/drivers/common/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o: src/drivers/common/can_frame_message_handler.c  .generated_files/flags/default/98ef57496b4c7180d1a4ff85583acfb916870d62 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o: src/drivers/common/can_rx_statemachine.c  .generated_files/flags/default/f253e6a51f69cef729a92be5e604743db2520c82 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o: src/drivers/common/can_tx_statemachine.c  .generated_files/flags/default/187e458671ff267ab6e2a2bce0fced97a038de03 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_store.o: src/drivers/common/can_buffer_store.c  .generated_files/flags/default/7d7fedb978cada14b636a071e664d1f37f30d614 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o: src/openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/a4fd1978fcf8093f7fadde8ba47548b1e247dada .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_node.o: src/openlcb/openlcb_node.c  .generated_files/flags/default/2dfa1babafa32248b809ed1bc69474cea197aadc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_node.c  -o ${OBJECTDIR}/src/openlcb/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_datagram.o: src/openlcb/protocol_datagram.c  .generated_files/flags/default/d31daa8994983bfd5496e9fa0709f9dfd590fc6e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_datagram.c  -o ${OBJECTDIR}/src/openlcb/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o: src/openlcb/openlcb_buffer_store.c  .generated_files/flags/default/cc11c600c597a766a2e2775f659f113778d72b2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_utilities.o: src/openlcb/openlcb_utilities.c  .generated_files/flags/default/15c7f5a71a6459d48715e31eb7d2a212360f9095 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_event_transport.o: src/openlcb/protocol_event_transport.c  .generated_files/flags/default/442ae6b0ab765794cd46f476a9329bb0051eb7b7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o: src/openlcb/openlcb_buffer_list.c  .generated_files/flags/default/4bd4a42d6c09a42fc673f5705071c1ca898b8be7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_snip.o: src/openlcb/protocol_snip.c  .generated_files/flags/default/490f826d825edcd9960f1c7acf092479103555c6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_snip.c  -o ${OBJECTDIR}/src/openlcb/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application_callbacks.o: src/openlcb/application_callbacks.c  .generated_files/flags/default/b574d35c56709745686a33a9433e1f9a29d0296b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application_callbacks.c  -o ${OBJECTDIR}/src/openlcb/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application_callbacks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o: src/openlcb/openlcb_gridconnect.c  .generated_files/flags/default/2fb3736d88737207c43b9fdf00b26384f3128192 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o: src/openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/f5783691228041f00e7a2ab134f978433e58cb7f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application.o: src/openlcb/application.c  .generated_files/flags/default/641b33ae023e2a2130687482bdb660a706e2c0a4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application.c  -o ${OBJECTDIR}/src/openlcb/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_message_network.o: src/openlcb/protocol_message_network.c  .generated_files/flags/default/4cb775da591862643c0438c4e84fb591ac3a7ea4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_message_network.c  -o ${OBJECTDIR}/src/openlcb/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o: src/openlcb/openlcb_tx_driver.c  .generated_files/flags/default/7d63ded367553a96e179510a032bd9d7a1b64bec .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DUSER_DEFINED_CAN_MSG_BUFFER_DEPTH=2 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=1 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_CDI_LENGTH=0 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DSUPPORT_FIRMWARE_BOOTLOADER -DPRINT_DEBUG -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
