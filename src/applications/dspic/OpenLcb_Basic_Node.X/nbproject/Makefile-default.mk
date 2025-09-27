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
FINAL_IMAGE=${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../dsPIC_Common/ecan1_helper.c main.c drivers.c node_parameters.c ../../../drivers/common/can_tx_message_handler.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_rx_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_login_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/common/can_main_statemachine_handler.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/protocol_datagram_config_mem_handler.c ../../../openlcb/openlcb_node.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/protocol_datagram_handler.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_application.c uart_handler.c traps.c common_debug_helper.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/main.o ${OBJECTDIR}/drivers.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_application.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/common_debug_helper.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/drivers.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d ${OBJECTDIR}/uart_handler.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/common_debug_helper.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/main.o ${OBJECTDIR}/drivers.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_application.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/common_debug_helper.o

# Source Files
SOURCEFILES=../dsPIC_Common/ecan1_helper.c main.c drivers.c node_parameters.c ../../../drivers/common/can_tx_message_handler.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_rx_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_login_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/common/can_main_statemachine_handler.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/protocol_datagram_config_mem_handler.c ../../../openlcb/openlcb_node.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/protocol_datagram_handler.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_application.c uart_handler.c traps.c common_debug_helper.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script=p33EP512GP504.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/59b2e4603c3859af27a6fc3117e12c33f3b5c954 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/2edea46a9e92849c6eb5041d7e70a33d34ab7bbd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/fd04ab055975ca5bc67bc3c4c30480de9c8acbc4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/6acba6e31a7ac7585e13af3b99f94ebc8bcd3ce6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o: ../../../drivers/common/can_tx_message_handler.c  .generated_files/flags/default/3d67ef7dd4d8bc4c7e91e8082fef3b447448b226 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/8d691c649549d50d01b237e13ee84378dc444295 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/faa6f2c7d1ad01dd4b7a5a9b09481c27f2803816 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/1d553ad20b97e561d4e82d0af386ff7adbf8270a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o: ../../../drivers/common/can_rx_message_handler.c  .generated_files/flags/default/d4ecde2938aa46959762b8b445b56e74cd3aa45c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/723aa18340c8b305f2fe0272300ba4661d4bf5dc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/ee97f8ce420462e46e81664385e4b1c548af3b68 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o: ../../../drivers/common/can_login_statemachine.c  .generated_files/flags/default/d485f9ca78240f0043cf100dc347cafe23e29690 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/59c5948947185a546ef7683e1e884815abb028e6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/6a0303fa991a95e01b82b5dfeab3d97f62d62dd2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o: ../../../drivers/common/can_main_statemachine_handler.c  .generated_files/flags/default/1d892fd52901c736ba9157eea2c6753e26031f59 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/13f32d21b9a2cc63e7aaf5acd38ac526dcd7f3a2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o: ../../../openlcb/protocol_datagram_config_mem_handler.c  .generated_files/flags/default/6db40f7b3793fc22d80f52431a6f1e14dc22d734 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_config_mem_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/df111cfac6d947a5f448303c33581dc930403fb9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/19747c120e45fd0bce9500c52ef0f758487737c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/a5593d8af5ca9983151a2c69385b5d39666999a5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/6cd09673db6d4fbd2f718c7daa1550be93624fe6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/e6bf881af3a3f70a3cf4c0e9bb9d7b9cffdb1234 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/53d87e3dabfc4dfc524fc1be5a7ad0a1be4b1ffd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/3b9660ba138091091ea1df322211a5883c95692b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/82f5da4f5e4c53cccec1d5edfa1fc71817c6a108 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/88a86239c8c397ef11a7336e0f3b98089ee3771b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/62d821f4b0fddef6c3ae8c2f5651b2ef9553d5ea .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/f3c273a259ab92abb3ca6819d97bc24fcf28b792 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/84f615c0b69a30db356d36c89d6f5784144c0b5e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/d733c855911d9ea7be908626c2954d7c2386a0cf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/358e40d840ef05e3fdd340aa28b280d8fa75cfa0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/4f58b1af5e80ea88c40e1b2b405acb88a91fee0b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/77595303150801a1c259032195bbf203f26951c0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/6c73a68303c87648e4418b01547651edffe7bd2a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/2e5bc6b8c0590df8527f43ae23eadd7d662ffd62 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o: ../../../drivers/common/can_tx_message_handler.c  .generated_files/flags/default/7352630e626d458cba98fbc8651b59479c3ec184 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/37c71c53697f50697a08315be75a628ed0fa6afc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/874979500e075ce88bacc46b1c44b095071acc2b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/27733c88f24df1a428793eae77f8d953880535ac .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o: ../../../drivers/common/can_rx_message_handler.c  .generated_files/flags/default/a79f03794106d4e3c9d102941e4c384f2eb7d0fb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/9cd36911d4fdabf91ed586c4c3a500c2e677d2b6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/db6e8893d6a51d65924d23d3154afe7f6b0ebdf2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o: ../../../drivers/common/can_login_statemachine.c  .generated_files/flags/default/3054bb550c60a6c7f02740b60bffe73e7ef3ee9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/8a46db344e2dfcec6f8a8dcde8685eee7ba662d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/51a9ae4b7aec109a9168d78c8f074cdebe9fadcb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o: ../../../drivers/common/can_main_statemachine_handler.c  .generated_files/flags/default/8f9f90b9b13847339d8a199d9e1e7fcdaa960c77 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/86778f8afa080bd8be088a931a9f9c38388eeb14 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o: ../../../openlcb/protocol_datagram_config_mem_handler.c  .generated_files/flags/default/389b50151a93af50bb2bc7374851b67ed7e7bc89 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_config_mem_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/7783dc701af0158c0847659d92328e1d2c6d665c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/74cba3abc3a1171ef6558c0a46a5c3abf9c94b37 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/9fa7dfab074471726d3ad325dbcad22672c7085a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/4e99cec3ba096f0d39a9523f67b5a2c22b07b2ca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/159cbdbfde1a397600ef150e02b7e80ba3073c4e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/e2778c8c43a51e6f15164c3382fdc3fe65aefc38 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/4f2efae98ea985a2aa18f51796581ceb4c9debc2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/dbb504e93ce10eaf10240381c5a5ff970e302435 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/5e28653b8bccc8acd11b21589294b105f1e9161c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/11e72d599d8b3458e482ff4d5c68c7c568347378 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/92528cd1c4c05781a02a5cc663532f66f6a30bb1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/37fdc7b5eb2a4e21bd338c6c6a0b9153ebaa5a7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/73db3fe685f331d2bf7e0a0e0b344d67d912eb36 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/1bad6bd04213592fcef4eabee6a5d14308679857 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=0 -DUSER_DEFINED_CONSUMER_COUNT=0 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
