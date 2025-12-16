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
OUTPUT_SUFFIX=
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=main.c drivers.c node_parameters.c uart_handler.c traps.c common_debug_helper.c dependency_injection.c dependency_injectors.c ../../../drivers/canbus/can_tx_message_handler.c ../../../drivers/canbus/can_buffer_fifo.c ../../../drivers/canbus/can_main_statemachine.c ../../../drivers/canbus/can_login_message_handler.c ../../../drivers/canbus/can_rx_message_handler.c ../../../drivers/canbus/can_utilities.c ../../../drivers/canbus/alias_mappings.c ../../../drivers/canbus/can_rx_statemachine.c ../../../drivers/canbus/can_login_statemachine.c ../../../drivers/canbus/can_tx_statemachine.c ../../../drivers/canbus/can_buffer_store.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/protocol_datagram_handler.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_application.c ../../../openlcb/openlcb_login_statemachine.c ../../../openlcb/openlcb_login_statemachine_handler.c ../../../openlcb/protocol_config_mem_operations_handler.c ../../../openlcb/protocol_config_mem_read_handler.c ../../../openlcb/protocol_config_mem_write_handler.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/main.o ${OBJECTDIR}/drivers.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/common_debug_helper.o ${OBJECTDIR}/dependency_injection.o ${OBJECTDIR}/dependency_injectors.o ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o ${OBJECTDIR}/_ext/1306449789/can_utilities.o ${OBJECTDIR}/_ext/1306449789/alias_mappings.o ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_application.o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o
POSSIBLE_DEPFILES=${OBJECTDIR}/main.o.d ${OBJECTDIR}/drivers.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/uart_handler.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/common_debug_helper.o.d ${OBJECTDIR}/dependency_injection.o.d ${OBJECTDIR}/dependency_injectors.o.d ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o.d ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o.d ${OBJECTDIR}/_ext/1306449789/can_utilities.o.d ${OBJECTDIR}/_ext/1306449789/alias_mappings.o.d ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o.d ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o.d ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o.d ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o.d ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o.d ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o.d ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/main.o ${OBJECTDIR}/drivers.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/common_debug_helper.o ${OBJECTDIR}/dependency_injection.o ${OBJECTDIR}/dependency_injectors.o ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o ${OBJECTDIR}/_ext/1306449789/can_utilities.o ${OBJECTDIR}/_ext/1306449789/alias_mappings.o ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_application.o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o

# Source Files
SOURCEFILES=main.c drivers.c node_parameters.c uart_handler.c traps.c common_debug_helper.c dependency_injection.c dependency_injectors.c ../../../drivers/canbus/can_tx_message_handler.c ../../../drivers/canbus/can_buffer_fifo.c ../../../drivers/canbus/can_main_statemachine.c ../../../drivers/canbus/can_login_message_handler.c ../../../drivers/canbus/can_rx_message_handler.c ../../../drivers/canbus/can_utilities.c ../../../drivers/canbus/alias_mappings.c ../../../drivers/canbus/can_rx_statemachine.c ../../../drivers/canbus/can_login_statemachine.c ../../../drivers/canbus/can_tx_statemachine.c ../../../drivers/canbus/can_buffer_store.c ../dsPIC_Common/ecan1_helper.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/protocol_snip.c ../../../openlcb/protocol_datagram_handler.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_application.c ../../../openlcb/openlcb_login_statemachine.c ../../../openlcb/openlcb_login_statemachine_handler.c ../../../openlcb/protocol_config_mem_operations_handler.c ../../../openlcb/protocol_config_mem_read_handler.c ../../../openlcb/protocol_config_mem_write_handler.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512MC506
MP_LINKER_FILE_OPTION=,--script=p33EP512MC506.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/48ec37aed285a33814b3c7ebe2ec03fd06613145 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/d85a54ec7890f1633b588d1d5c6bbcdbab537389 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/ccf6fd086964a9b8b43f4d750bb3c3ca04a7cc9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/69236c680ba7a96073c0a1623deb47a446a91cf2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/20f2d0f1724ed8ab412e1e9b43c1d9b143b0d53 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/3f8e14e9afa7951fae3b4b6ae7a80f010fcded9a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/dependency_injection.o: dependency_injection.c  .generated_files/flags/default/d78358072de4106c04e8df1221f577c81cc46396 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dependency_injection.o.d 
	@${RM} ${OBJECTDIR}/dependency_injection.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  dependency_injection.c  -o ${OBJECTDIR}/dependency_injection.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/dependency_injection.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/dependency_injectors.o: dependency_injectors.c  .generated_files/flags/default/7982cb6b31b07e6a68031b205f2f71979d0faae6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dependency_injectors.o.d 
	@${RM} ${OBJECTDIR}/dependency_injectors.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  dependency_injectors.c  -o ${OBJECTDIR}/dependency_injectors.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/dependency_injectors.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o: ../../../drivers/canbus/can_tx_message_handler.c  .generated_files/flags/default/e4d5a8d9d240f0a3ff51c75e716d2487c0d24790 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o: ../../../drivers/canbus/can_buffer_fifo.c  .generated_files/flags/default/fc7b85ec7b2a35e5e029569e6d8fb231ad5b6493 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o: ../../../drivers/canbus/can_main_statemachine.c  .generated_files/flags/default/a47af6f0e563a48d7ff3a02de3a3757c7b42348f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o: ../../../drivers/canbus/can_login_message_handler.c  .generated_files/flags/default/d5ab9207bbc30bc42e7b2a12ed5cb0947e68423f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o: ../../../drivers/canbus/can_rx_message_handler.c  .generated_files/flags/default/6128e2b3c19b6f45e483b962a124c0603406583d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_utilities.o: ../../../drivers/canbus/can_utilities.c  .generated_files/flags/default/3d61d7d026c677100012ed1ddb78479347abaa87 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_utilities.c  -o ${OBJECTDIR}/_ext/1306449789/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/alias_mappings.o: ../../../drivers/canbus/alias_mappings.c  .generated_files/flags/default/22991aca93a8bc6ab84176f3f0063c34e43de382 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/alias_mappings.c  -o ${OBJECTDIR}/_ext/1306449789/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/alias_mappings.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o: ../../../drivers/canbus/can_rx_statemachine.c  .generated_files/flags/default/2d9b07f2d2a10a2f3cd7b3e00ca978f8b23d2429 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o: ../../../drivers/canbus/can_login_statemachine.c  .generated_files/flags/default/5b825cf2d548aa8267da95d29dd1a77c57e97a2f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o: ../../../drivers/canbus/can_tx_statemachine.c  .generated_files/flags/default/97cbdd0bb04880a41db75e73764aec536f379999 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_buffer_store.o: ../../../drivers/canbus/can_buffer_store.c  .generated_files/flags/default/a4286be286f40de85f907b3bfbc2ec19fb84ff2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/7d5a3f338bea29226da6757c60c01c7eec2f8071 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/4ca5eea18c815f56c34c53f3881bf20acf577aab .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/5bd66c002f75a9f875493eeda286c1bb715a502b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/7d03267b132344d872d95b850feb0c034ebbf198 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/5dcbe6d8a58bd522bd452a8592b7ab7df85f2701 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/e8cdb4b79bea1fe31b995ff80f302058d135d784 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/1925b90cc963dbdb8eff2fb6e133d544a39f40bc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/dd7cf938dfaf7c9b240e4c723dbd7c712c6c2f33 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/dda3a4611b8b4d823a1f6ed65aadcd5636385754 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/61f257a402397d109bb6bfbfe5d3ba5ed398b8e2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/a044f713a37d70d1700cbdaaee148ac489247d7b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/f807efeb4c3949fbd22b76bddc33c5366ba75fed .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/7b42d030673ef340b71e1d74d9a74bd9f5a33c3a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o: ../../../openlcb/openlcb_login_statemachine.c  .generated_files/flags/default/b9d7a695b3336470b28b3b9a0ccc1df7da0987e7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_login_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o: ../../../openlcb/openlcb_login_statemachine_handler.c  .generated_files/flags/default/b39fd8ecfa4cd145e06aa45e4d11933ed2ed8960 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_login_statemachine_handler.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o: ../../../openlcb/protocol_config_mem_operations_handler.c  .generated_files/flags/default/4e2ec69197eb927fcc19131953c13fe35e06d854 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_config_mem_operations_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o: ../../../openlcb/protocol_config_mem_read_handler.c  .generated_files/flags/default/eaa4c9fa448cfe9b83037d7ec959506e31574c09 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_config_mem_read_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o: ../../../openlcb/protocol_config_mem_write_handler.c  .generated_files/flags/default/21ad975bc7eb41eb77e2431a72267f2262d02fc2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_config_mem_write_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/3262360de34abcd110ced4875da3088617bd6e24 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/drivers.o: drivers.c  .generated_files/flags/default/bb3183c2806a681faeee054dfb5cb9d65978303d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/drivers.o.d 
	@${RM} ${OBJECTDIR}/drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  drivers.c  -o ${OBJECTDIR}/drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/6e2a5fa5925cd48728a493db731da1d1a12b85d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/b2ef9f4bba4c90493750e69173987b6ae4e43f72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/d7c4034774f927d633099e5ed1b87d33dcf69efe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/common_debug_helper.o: common_debug_helper.c  .generated_files/flags/default/566ab0474a85c9ba232aad828eb44b1932c3dd1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/common_debug_helper.o.d 
	@${RM} ${OBJECTDIR}/common_debug_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  common_debug_helper.c  -o ${OBJECTDIR}/common_debug_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/common_debug_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/dependency_injection.o: dependency_injection.c  .generated_files/flags/default/b0f687fc5b8841f05847b77da6d1f8fcdb7c3cee .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dependency_injection.o.d 
	@${RM} ${OBJECTDIR}/dependency_injection.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  dependency_injection.c  -o ${OBJECTDIR}/dependency_injection.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/dependency_injection.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/dependency_injectors.o: dependency_injectors.c  .generated_files/flags/default/9411227b3c84d1704ce9b1c2aa951a7a5b8f405f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/dependency_injectors.o.d 
	@${RM} ${OBJECTDIR}/dependency_injectors.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  dependency_injectors.c  -o ${OBJECTDIR}/dependency_injectors.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/dependency_injectors.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o: ../../../drivers/canbus/can_tx_message_handler.c  .generated_files/flags/default/cc0451fc70bdaf7af0e2fb097628c2f38bd416d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_tx_message_handler.c  -o ${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_tx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o: ../../../drivers/canbus/can_buffer_fifo.c  .generated_files/flags/default/7197a4f5e9f2d557f5c6573dcefd01a8c57ceb24 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o: ../../../drivers/canbus/can_main_statemachine.c  .generated_files/flags/default/dfd4baf513002c34c7a2b50d8fed866ce3a6715f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o: ../../../drivers/canbus/can_login_message_handler.c  .generated_files/flags/default/418333e2d33ce10ab6dd0c6542ec5e4c0a89cba5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o: ../../../drivers/canbus/can_rx_message_handler.c  .generated_files/flags/default/6e86ca96807be42fc68f4a46705932b53e21fe23 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_rx_message_handler.c  -o ${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_rx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_utilities.o: ../../../drivers/canbus/can_utilities.c  .generated_files/flags/default/e5ebb3431dd0cdf6d45ac1ae481a3e7ae23b832f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_utilities.c  -o ${OBJECTDIR}/_ext/1306449789/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/alias_mappings.o: ../../../drivers/canbus/alias_mappings.c  .generated_files/flags/default/442f8b335ffdf025d06f88e8bc4c110fbdf4a409 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/alias_mappings.c  -o ${OBJECTDIR}/_ext/1306449789/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/alias_mappings.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o: ../../../drivers/canbus/can_rx_statemachine.c  .generated_files/flags/default/1835b48b2bbaa2cc2ba12ca8954cc4006e5bcced .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o: ../../../drivers/canbus/can_login_statemachine.c  .generated_files/flags/default/6a21f86e44b15e5fb8eb380f6b7ed14552c118d0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_login_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o: ../../../drivers/canbus/can_tx_statemachine.c  .generated_files/flags/default/728dfe422b56dee0b7d2d19ac2623c0c6739a00c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1306449789/can_buffer_store.o: ../../../drivers/canbus/can_buffer_store.c  .generated_files/flags/default/de47b7ec79cd55000bfba1b9c72372d59f0dac20 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1306449789" 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/canbus/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1306449789/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1306449789/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1374698558/ecan1_helper.o: ../dsPIC_Common/ecan1_helper.c  .generated_files/flags/default/4eacb79657cb11c6234566013be63ec2de443e9c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1374698558" 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../dsPIC_Common/ecan1_helper.c  -o ${OBJECTDIR}/_ext/1374698558/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1374698558/ecan1_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/b861eb3d3e4f99fb08b458c856264ec142f37724 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/5f120243b144de0b64a38a4e35a3bd1247727818 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/1366af84dd3f4cbc211ad191f219f0b216cf337b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/4ddd075377aeef7b19763a5fbd3c5020b4c6960d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/1d8f01913e908df0947c6eeaa0f9c483a1d81246 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/8003e984ecb4d3c65eae79c41ba5ef8b08a39459 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/4ec1708f3f575aeeff86187195998d98ab39abed .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o: ../../../openlcb/protocol_datagram_handler.c  .generated_files/flags/default/8df508722729fea90afc1cb39371f63320afc1bc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/1dd3e0556b79d0ec9ad212b098037d7591ca720b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/db0e88ccc2726f0ff45f6a801812e1a78ddb1fca .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/f6b7b2532a651a2963d2c0ae83ac7db77d1fd9a7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_application.o: ../../../openlcb/openlcb_application.c  .generated_files/flags/default/1e2f03e6483a3615732f522f114bdb5372bf7d05 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_application.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o: ../../../openlcb/openlcb_login_statemachine.c  .generated_files/flags/default/79fd74ec0294eea161fe6578b83c8a8f5cc0b52a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_login_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o: ../../../openlcb/openlcb_login_statemachine_handler.c  .generated_files/flags/default/89c48ac9c26173f349ec81c9f1c5ef371bfdd4d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_login_statemachine_handler.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_login_statemachine_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o: ../../../openlcb/protocol_config_mem_operations_handler.c  .generated_files/flags/default/b32471c1cf87fefd2ca8562eb92c7e38c5509c02 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_config_mem_operations_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_config_mem_operations_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o: ../../../openlcb/protocol_config_mem_read_handler.c  .generated_files/flags/default/857f56f9b545d784283288733b2c7e3151126f70 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_config_mem_read_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_config_mem_read_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o: ../../../openlcb/protocol_config_mem_write_handler.c  .generated_files/flags/default/444baa3c8841d24d8be300be7ae52f26c72ac902 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_config_mem_write_handler.c  -o ${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_config_mem_write_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000 -DEXCLUDE_FIRMWARE_ADDRESS_SPACE -DUSER_DEFINED_CDI_LENGTH=1200 -DUSER_DEFINED_FDI_LENGTH=0 -DUSER_DEFINED_PRODUCER_COUNT=10 -DUSER_DEFINED_CONSUMER_COUNT=10 -DUSER_DEFINED_NODE_BUFFER_DEPTH=10 -DUSER_DEFINED_SNIP_BUFFER_DEPTH=20 -DUSER_DEFINED_DATAGRAM_BUFFER_DEPTH=20 -DUSER_DEFINED_BASIC_BUFFER_DEPTH=50 -DUSER_DEFINED_STREAM_BUFFER_DEPTH=2 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
