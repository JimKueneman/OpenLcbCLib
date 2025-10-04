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
SOURCEFILES_QUOTED_IF_SPACED=../../../drivers/common/can_tx_message_handler.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_rx_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_login_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/common/can_main_statemachine_handler.c ../../../drivers/alias_mappings.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/protocol_datagram_config_mem_handler.c ../../../openlcb/openlcb_node.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/protocol_datagram_handler.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_application.c main.c drivers.c node_parameters.c uart_handler.c traps.c common_debug_helper.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o ${OBJECTDIR}/_ext/954557028/alias_mappings.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_application.o ${OBJECTDIR}/main.o ${OBJECTDIR}/drivers.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/common_debug_helper.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d ${OBJECTDIR}/_ext/954557028/alias_mappings.o.d ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/drivers.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/uart_handler.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/common_debug_helper.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o ${OBJECTDIR}/_ext/954557028/alias_mappings.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_application.o ${OBJECTDIR}/main.o ${OBJECTDIR}/drivers.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/common_debug_helper.o

# Source Files
SOURCEFILES=../../../drivers/common/can_tx_message_handler.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_rx_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_login_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/common/can_main_statemachine_handler.c ../../../drivers/alias_mappings.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/protocol_datagram_config_mem_handler.c ../../../openlcb/openlcb_node.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/protocol_datagram_handler.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_application.c main.c drivers.c node_parameters.c uart_handler.c traps.c common_debug_helper.c



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

MP_PROCESSOR_OPTION=33EP512MC506
MP_LINKER_FILE_OPTION=,--script=p33EP512MC506.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o: ../../../drivers/common/can_tx_message_handler.c  .generated_files/flags/default/d6ae2f4391081e3a4a5c132b7bca77507f19f336 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/4925be5ad5d27d1c2f68e53324f9a6337d683bcf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/10de725e754a077db0916318e28d7a268dc3b085 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/2d8ce5b2e5a99e3bc08c13a57dbdc7a6ab945e40 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o: ../../../drivers/common/can_rx_message_handler.c  .generated_files/flags/default/3e8fa56342769d27990c5a184a819845986dbbee .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/d134dba4820f1a921526e61d387ce34a02e13f01 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/5b967bf8ff895df43cfb08232229245a21f61c84 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o: ../../../drivers/common/can_login_statemachine.c  .generated_files/flags/default/40151eaef297a49e830ab26b28ca83b2a3208376 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/7af051293078831231ad22e8b5f77a85e6a0e205 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/b7ed5b1991c57cfddb840538660a338ac4d5b849 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o: ../../../drivers/common/can_main_statemachine_handler.c  .generated_files/flags/default/e6195ee35bd8b0cb79abd38709303215d13fb1af .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/alias_mappings.o: ../../../drivers/alias_mappings.c  .generated_files/flags/default/1e4c0c4e95798d11cd4451aed3860d52277a6baf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/alias_mappings.c  -o ${OBJECTDIR}/_ext/954557028/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/alias_mappings.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/7683b6f42b9a1d377fb4d90d97a2a3b31cde20ae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/47e46714a6c1ba5466acc2c535d562dbcdc89a24 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o: ../../../openlcb/protocol_datagram_config_mem_handler.c  .generated_files/flags/default/9b9ac56dcd7d5e44d502aa3266c82e2c4e66d148 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_config_mem_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/fb55aaeed8ea89c42f4dc9bc412f86512b560a54 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/ecfc6940a7d9d95252cc5e3550f10b9b0d242e47 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/b922aa8a5fda9673c3fbc214991475e736a458cb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/6bc076579cf9e3030505ab05033d5f0a857c3179 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/5f6f161a20063bb4550f786602523aa5de9e54a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/6c3b6173da5f894d526917f4ca30fee6f0ce71ef .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/7cc094afafe142459d89543e8797329c84615b9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/332a6b60aa35ba3f7c4e89ba44bfa68db59b33e7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/c76a67f57d8d72c84e8981ff9a165d0257a398b6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/da0796dd676f20be4454f638207fa616276b3385 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/177557987dbbddcf951e71dec919f3c4c685e95c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/6d960f83f1a72c2cc2b33f9ad167a26e92a5219b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/d2516af2b39a8a057c9d940511cbf1abb41be09b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/a4e78cfa1647646ab081e04feab74f425f4a89db .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/c5d5e4bf0f9392e7bfc71597d3d1ad4d6e2fcca5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/c682d86ccb10c2d09e1950c55d022770696350df .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/233fd04ffc97a5e0df84b4368fd80ef3c47e546a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o: ../../../drivers/common/can_tx_message_handler.c  .generated_files/flags/default/8d169a68101a6830d6dd0668fd070085572c0463 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/3bc95212d70e43312d92500e7932315e4a41faa8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/d13658f8d489e0215272cb64f9b89e50b1740c64 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/f3510083f8ca0b1f96d0aa3437a92d1e177d8107 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o: ../../../drivers/common/can_rx_message_handler.c  .generated_files/flags/default/ecbe76590919edb608b702b2ece1187c20e13883 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/bbe58bd360622a579b3c057c28a22ad604a03bf7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/15f1d1093ce04680356edd512daaa5c917f3f9ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o: ../../../drivers/common/can_login_statemachine.c  .generated_files/flags/default/b686f15ff8db1ba7351fc92cf295d6c516f05b83 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/2c9d154890e82acbf315ed40d863b00acc5c5d1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/843c143b1cb04e0a747ddf19033fc198b0d7ea6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o: ../../../drivers/common/can_main_statemachine_handler.c  .generated_files/flags/default/761ea204b4348247ad3a6e17dec4bcd5476af245 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/alias_mappings.o: ../../../drivers/alias_mappings.c  .generated_files/flags/default/9d30d6d76681d552c68f82148e3c72bddb41db44 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/alias_mappings.c  -o ${OBJECTDIR}/_ext/954557028/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/alias_mappings.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/8b85bee7fd11075ea26ac7c6f618ce17ac873ad4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/629de0280a05d96bcba74d58c38cd03978bdf767 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o: ../../../openlcb/protocol_datagram_config_mem_handler.c  .generated_files/flags/default/4a592fa2be6606aa5f757d7fc17595ba0e68e600 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_config_mem_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/93a0449496f10b3698ac7327a86546b3483aa93a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/888fab31992ac984d3e2c4d2a7cf3a47032c829 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/d463088a2eb7e9008572428b6239fc3c62843af4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/70d89a25eedffd20004aca55e6a9df843aa1a465 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/23903c452525d5a76b3c62bfa32bec8c2297f306 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/bfb21b4b90005ae8120036d4548ac734f306d152 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/b17920482cf8b39b74e22d7ca09b1fb0dba945d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/ed73679227561bb5cffe9c6d4cf4ade813436d06 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/fb0d25a10faa647b635dd2988aa54c1f05535c5d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/4a8450cf4ff97c72ef46e85eec4797b02ee319d5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/ba02feab3a845a690c78026a5e33550c1b9e733d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/45d535c0b6e929591f26518fc8fc93c8337865ed .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/ead0debaa8c4492d6c5ccc8a5b10ae9706b4aae5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/94d5c5f45a542f0d6fb78d315558ad1b00188bfc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/2c44ab23028b84da3078a7c36cc6ad27b0773a9b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/a66f418b78aab5c24b3d668f6f4e56f31c991264 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/697170e4bbe4f39d93bb610a395e9fd22491bd72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=2 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
