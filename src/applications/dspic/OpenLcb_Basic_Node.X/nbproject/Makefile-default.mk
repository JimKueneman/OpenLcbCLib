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
${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o: ../../../drivers/common/can_tx_message_handler.c  .generated_files/flags/default/350b7a59f0e463cafb47243a784c1236827871a5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/70217ca1ab8f0dfbbdd5fee90c024f63226d943a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/edfd141393e8647d7bde1bbe5e242763e55dc750 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/821313d3368146723c8e53e20406eae23eb9b591 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o: ../../../drivers/common/can_rx_message_handler.c  .generated_files/flags/default/90d3792cc9c781767d4d15297bd028d16dbeef94 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/341436d0169b86fa23ec7a8dfc4f9bb9f172f7a8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/688b7a645d2bf555659ad184bccd1898c293a864 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o: ../../../drivers/common/can_login_statemachine.c  .generated_files/flags/default/bbb2b35bf2c2e07a3def82504975cb6df6839d7a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/d25db2d1c4e5261173244702f2d5d82a514cbb8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/2d2f05cbd6ef9aa871e43ac42b5a98bb52e32170 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o: ../../../drivers/common/can_main_statemachine_handler.c  .generated_files/flags/default/4a0a8522536842440ff0565c2461c47ec8c579a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/alias_mappings.o: ../../../drivers/alias_mappings.c  .generated_files/flags/default/db084b3d7fecddb142164e6707c8cde87f66ef89 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/alias_mappings.c  -o ${OBJECTDIR}/_ext/954557028/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/alias_mappings.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/a08f54f2190d03403f73b203920b29a3d2aad427 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/9e373383ecf733584f1007e49927a8848d4c9621 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o: ../../../openlcb/protocol_datagram_config_mem_handler.c  .generated_files/flags/default/7371b037ea593a63f7b9825a28b05e14f65c5708 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_config_mem_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/137064c4320e5204da9e3d687db922b89950a52a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/f7d7863b70d7d1ff2579894cb4cbbe0c253152ea .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/9d560c20f4548e504fe3843d76bbdae61bf66b0a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/5c5ee78a5c9ff40ab1505166a0c6f072615f7932 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/b402cc0bc3f134ad481f93bd6a4b2f84ee3e7f21 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/9f69dfe321eda59b3080247ca55ff9c90ca6b2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/c4f905c1c5ddd2113106d511fa7d9c0102fe4191 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/7012fdb5f2030f2aa5b8341c978bae878640ef14 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/86cf975f9f578191d5f0e007bb23a78e3092a4c0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/aec889895816cdf41f45894675de1de0fef03077 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/d988a0dd125da9d96aeed878189403f272b98f30 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/99732c442619088c03374bbc947748896730b2f3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/822adf26cc9856b2cc3ef1fe77435a8d2088bcad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/47a59777f8737c8e737ce809f3a3a855a5ccbd45 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/afb5d9049ddd853268670b809ec74393114741d5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/b370fe39af5b1c2bd455efff1b24b2d0de150fd9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/ccc5f6f67e09765c2dc87b78f8ed133e9e208e11 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o: ../../../drivers/common/can_tx_message_handler.c  .generated_files/flags/default/f4a5cbc071d6a306d928f175a4a7400aa304b10a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/6b95ec550f20113eb854ef9322d7479340ec31f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/f78b454c55ed53bc4ec6d1933c112a4aa209701 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/da07c189bc548f9c80572c85b2c20c7c4a93db9f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o: ../../../drivers/common/can_rx_message_handler.c  .generated_files/flags/default/b110eeae2db19bf91dc2e52a4869d8170a0fed9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/146c9439a9b3b09b6a91348b58f9b831c989afa6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/fa2ceb2014f30e99075beaee14b65a9d56933365 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o: ../../../drivers/common/can_login_statemachine.c  .generated_files/flags/default/1c8ad8c420a9003351cdcb7aa347fe7602ca4327 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/bdcbf4d6e4f8fea8f05af7e14f2af02ca82666b0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/a0f8ee66f7fc041357198a3a48ba0b885f2d1c5e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o: ../../../drivers/common/can_main_statemachine_handler.c  .generated_files/flags/default/e09e264cb447f1120f75b4c5dca8dd6a20de4807 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/alias_mappings.o: ../../../drivers/alias_mappings.c  .generated_files/flags/default/e22e177fd9fb1e6ec3e7c9cd95c04c0523d5efa2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/alias_mappings.c  -o ${OBJECTDIR}/_ext/954557028/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/alias_mappings.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/955a865b8fa753ba0327ac0ce4ba1aad188dd1fe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/d70803adfaddb19b71479d0afb4be62fb7b1b06f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o: ../../../openlcb/protocol_datagram_config_mem_handler.c  .generated_files/flags/default/1fb055d5d7489ea0aeb090206a0d760e75242556 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_config_mem_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_config_mem_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/72d638402186ff582a84c3332e08fb8d58150558 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/fdc32423f2829fa9489905bb451da6ec97ee0504 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/af0e70ffcc014380fc10f72c2d3401955b4dc56e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/5c21c1c350f779b8d02984e491ae0f37091c034b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/ca1db3b290fcb03d6c0f60b75d1222502cd9dfb8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/eaeffb0f526e3f87f15a66d3e8758a0b0a43891c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/f84483d22c80f368880115b706808c7a976be5eb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/675c8921669884c22487cfd82e571709451516c9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/112c9518ac60777e88a8fbaf44ddcc73fb161e39 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/ba648f7075cb82319a658633d538e354821206fd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/a1ad05b31438ef65db3e843b08836618b7c18a9c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/f45ed7c88d6e5cba4972054887115cd005a69031 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/55360685bc73829fad151a7862b37622e4e5b535 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/6412932bf6ab2d10e0d5d52a62049be2e6908a41 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/6bd535341b4807baed35d2743a15ec95f61a0132 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/ca2f5b77c377ab3981a58378a59707895d794148 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/bcc066783068e733d1f0898148a1d3da27546313 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/OpenLcb_Basic_Node.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=1 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=10 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=10 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
