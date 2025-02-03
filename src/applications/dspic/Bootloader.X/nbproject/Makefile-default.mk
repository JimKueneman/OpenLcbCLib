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
SOURCEFILES_QUOTED_IF_SPACED=local_drivers/_25AA1024/25AA1024_driver.c src/drivers/common/can_buffer_fifo.c src/drivers/common/can_main_statemachine.c src/drivers/common/can_login_message_handler.c src/drivers/common/can_utilities.c src/drivers/common/can_frame_message_handler.c src/drivers/common/can_rx_statemachine.c src/drivers/common/can_tx_statemachine.c src/drivers/common/can_buffer_store.c src/drivers/ecan1_helper.c src/drivers/driver_mcu.c src/drivers/driver_can.c src/drivers/driver_100ms_clock.c src/drivers/driver_configuration_memory.c src/openlcb/openlcb_buffer_fifo.c src/openlcb/openlcb_node.c src/openlcb/protocol_datagram.c src/openlcb/openlcb_buffer_store.c src/openlcb/openlcb_utilities.c src/openlcb/protocol_event_transport.c src/openlcb/openlcb_buffer_list.c src/openlcb/protocol_snip.c src/openlcb/application_callbacks.c src/openlcb/openlcb_gridconnect.c src/openlcb/openlcb_main_statemachine.c src/openlcb/application.c src/openlcb/protocol_message_network.c src/openlcb/openlcb_tx_driver.c debug.c bootloader_drivers.c main.c node_parameters.c uart_handler.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o ${OBJECTDIR}/src/drivers/common/can_utilities.o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o ${OBJECTDIR}/src/drivers/ecan1_helper.o ${OBJECTDIR}/src/drivers/driver_mcu.o ${OBJECTDIR}/src/drivers/driver_can.o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o ${OBJECTDIR}/src/openlcb/openlcb_node.o ${OBJECTDIR}/src/openlcb/protocol_datagram.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o ${OBJECTDIR}/src/openlcb/protocol_snip.o ${OBJECTDIR}/src/openlcb/application_callbacks.o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o ${OBJECTDIR}/src/openlcb/application.o ${OBJECTDIR}/src/openlcb/protocol_message_network.o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/bootloader_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o
POSSIBLE_DEPFILES=${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d ${OBJECTDIR}/src/drivers/common/can_utilities.o.d ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d ${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d ${OBJECTDIR}/src/drivers/ecan1_helper.o.d ${OBJECTDIR}/src/drivers/driver_mcu.o.d ${OBJECTDIR}/src/drivers/driver_can.o.d ${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d ${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d ${OBJECTDIR}/src/openlcb/openlcb_node.o.d ${OBJECTDIR}/src/openlcb/protocol_datagram.o.d ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d ${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d ${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d ${OBJECTDIR}/src/openlcb/protocol_snip.o.d ${OBJECTDIR}/src/openlcb/application_callbacks.o.d ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d ${OBJECTDIR}/src/openlcb/application.o.d ${OBJECTDIR}/src/openlcb/protocol_message_network.o.d ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/bootloader_drivers.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/uart_handler.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o ${OBJECTDIR}/src/drivers/common/can_utilities.o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o ${OBJECTDIR}/src/drivers/ecan1_helper.o ${OBJECTDIR}/src/drivers/driver_mcu.o ${OBJECTDIR}/src/drivers/driver_can.o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o ${OBJECTDIR}/src/openlcb/openlcb_node.o ${OBJECTDIR}/src/openlcb/protocol_datagram.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o ${OBJECTDIR}/src/openlcb/protocol_snip.o ${OBJECTDIR}/src/openlcb/application_callbacks.o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o ${OBJECTDIR}/src/openlcb/application.o ${OBJECTDIR}/src/openlcb/protocol_message_network.o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/bootloader_drivers.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/uart_handler.o

# Source Files
SOURCEFILES=local_drivers/_25AA1024/25AA1024_driver.c src/drivers/common/can_buffer_fifo.c src/drivers/common/can_main_statemachine.c src/drivers/common/can_login_message_handler.c src/drivers/common/can_utilities.c src/drivers/common/can_frame_message_handler.c src/drivers/common/can_rx_statemachine.c src/drivers/common/can_tx_statemachine.c src/drivers/common/can_buffer_store.c src/drivers/ecan1_helper.c src/drivers/driver_mcu.c src/drivers/driver_can.c src/drivers/driver_100ms_clock.c src/drivers/driver_configuration_memory.c src/openlcb/openlcb_buffer_fifo.c src/openlcb/openlcb_node.c src/openlcb/protocol_datagram.c src/openlcb/openlcb_buffer_store.c src/openlcb/openlcb_utilities.c src/openlcb/protocol_event_transport.c src/openlcb/openlcb_buffer_list.c src/openlcb/protocol_snip.c src/openlcb/application_callbacks.c src/openlcb/openlcb_gridconnect.c src/openlcb/openlcb_main_statemachine.c src/openlcb/application.c src/openlcb/protocol_message_network.c src/openlcb/openlcb_tx_driver.c debug.c bootloader_drivers.c main.c node_parameters.c uart_handler.c



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
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/602239313607bb3843e0bc0430d8b06ef7022dcc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o: src/drivers/common/can_buffer_fifo.c  .generated_files/flags/default/d82142f5a0c0b466983bdc889e01488734687b42 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_main_statemachine.o: src/drivers/common/can_main_statemachine.c  .generated_files/flags/default/c00906ceea069441bbe49c5c0b88ac05d21dd33f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_login_message_handler.o: src/drivers/common/can_login_message_handler.c  .generated_files/flags/default/d7476c64b643b426f1c98694928e9924d2fb20dd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_utilities.o: src/drivers/common/can_utilities.c  .generated_files/flags/default/d38971edc84c66e974ffd8b1d2a66e4357915b96 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_utilities.c  -o ${OBJECTDIR}/src/drivers/common/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o: src/drivers/common/can_frame_message_handler.c  .generated_files/flags/default/ce023d1fb2c822672acb3f1e66a2bdf3386bad6c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o: src/drivers/common/can_rx_statemachine.c  .generated_files/flags/default/e12d427d91dd10bfb10803afd798fb8d60214679 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o: src/drivers/common/can_tx_statemachine.c  .generated_files/flags/default/9a7c41ae8ba201a3392e85f1be3341418f141571 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_store.o: src/drivers/common/can_buffer_store.c  .generated_files/flags/default/7e2b1594d43615f54fa1806c7f514f723ced26d2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/ecan1_helper.o: src/drivers/ecan1_helper.c  .generated_files/flags/default/ddd8010d7e3517c2104a51ec6236f1bf36fa0288 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/ecan1_helper.c  -o ${OBJECTDIR}/src/drivers/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/ecan1_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_mcu.o: src/drivers/driver_mcu.c  .generated_files/flags/default/5e9a5b3c6e8b38739a91264b145b5b60c1940c26 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_mcu.c  -o ${OBJECTDIR}/src/drivers/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_can.o: src/drivers/driver_can.c  .generated_files/flags/default/bbdd57a70062630f18fa7031daf613b40bfcc2a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_can.c  -o ${OBJECTDIR}/src/drivers/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_100ms_clock.o: src/drivers/driver_100ms_clock.c  .generated_files/flags/default/9eaa22e4f025426fbf0159e35b03e8b054a74939 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_configuration_memory.o: src/drivers/driver_configuration_memory.c  .generated_files/flags/default/929fe64459702b715a54dc706ae35e3ce39d1d7f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o: src/openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/33ae7ae054214448a326f322c9b034aab9494df0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_node.o: src/openlcb/openlcb_node.c  .generated_files/flags/default/f3d074bb6c96dcd785a47f6998a564cd5ec0a7dd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_node.c  -o ${OBJECTDIR}/src/openlcb/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_datagram.o: src/openlcb/protocol_datagram.c  .generated_files/flags/default/40f76ac98c337eedb1ad2099a5887759f5e7b771 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_datagram.c  -o ${OBJECTDIR}/src/openlcb/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o: src/openlcb/openlcb_buffer_store.c  .generated_files/flags/default/53d2e79aae7ef367ab6298037da4b54df8c8225 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_utilities.o: src/openlcb/openlcb_utilities.c  .generated_files/flags/default/8c9d202fb8789dee36f907a199bce865ebf52d79 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_event_transport.o: src/openlcb/protocol_event_transport.c  .generated_files/flags/default/b85fd3c4ccfd9bfc19bc71eeda836f5c763f68ea .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o: src/openlcb/openlcb_buffer_list.c  .generated_files/flags/default/2a9b940c3da8579574f25aec9d80dc90a524711e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_snip.o: src/openlcb/protocol_snip.c  .generated_files/flags/default/1ec096319a2716f32dee3cca6f1bd7b7b08edb65 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_snip.c  -o ${OBJECTDIR}/src/openlcb/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application_callbacks.o: src/openlcb/application_callbacks.c  .generated_files/flags/default/2f8ef42416247c544054a29c5e491c7d8ced7398 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application_callbacks.c  -o ${OBJECTDIR}/src/openlcb/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application_callbacks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o: src/openlcb/openlcb_gridconnect.c  .generated_files/flags/default/6166fd2c516ac1aa0063a2dcfa9256a102b53750 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o: src/openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/b2458d0745be5422a2568dd2dd38d2463b5b2ad3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application.o: src/openlcb/application.c  .generated_files/flags/default/21f8a376124343246c2c84ce800190cb6dd91c1b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application.c  -o ${OBJECTDIR}/src/openlcb/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_message_network.o: src/openlcb/protocol_message_network.c  .generated_files/flags/default/ce9e4e61a08d3fad8d2d741dfe278c8d832c5feb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_message_network.c  -o ${OBJECTDIR}/src/openlcb/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o: src/openlcb/openlcb_tx_driver.c  .generated_files/flags/default/b1f9239775995fbd276dd2157c3b6fc12f46dff4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/3a6dc0892b7696507ced5382ea72d4594498600a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/bootloader_drivers.o: bootloader_drivers.c  .generated_files/flags/default/f9a1add8f0495811e895755bb1185d7ddc847df3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bootloader_drivers.c  -o ${OBJECTDIR}/bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/bootloader_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/bc9e805115a2d4b33b613084bf3769f6e79f1014 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/2e39249467da48d3f3f08fc6a11b489c629f586b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/a3d185be033993610aa1fe271f88d952354d6e1e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/32d8f35ad65ff1ba0a0a12454b876e2b440383f4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o: src/drivers/common/can_buffer_fifo.c  .generated_files/flags/default/2f510f8e3a2a1a349969b717b1ea79839623f1de .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_fifo.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_main_statemachine.o: src/drivers/common/can_main_statemachine.c  .generated_files/flags/default/245c6e5ad33e4b9ad3ecf38db8e3104ee216d32b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_main_statemachine.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_login_message_handler.o: src/drivers/common/can_login_message_handler.c  .generated_files/flags/default/219992cb9027d5e6dae071b842b3c2ebf29f8082 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_login_message_handler.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_utilities.o: src/drivers/common/can_utilities.c  .generated_files/flags/default/5510fb04a3043622a97668d67403aa2792b43c26 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_utilities.c  -o ${OBJECTDIR}/src/drivers/common/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_utilities.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o: src/drivers/common/can_frame_message_handler.c  .generated_files/flags/default/6ed9f5287a3b6639bea91045e0725d492311d721 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_frame_message_handler.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o: src/drivers/common/can_rx_statemachine.c  .generated_files/flags/default/fb64d1a93e3f926671545b563b444c6fd7b24bb2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_rx_statemachine.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o: src/drivers/common/can_tx_statemachine.c  .generated_files/flags/default/ab02d5d5b1573bff3d155f619bfcbe8d77ff32db .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_tx_statemachine.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/common/can_buffer_store.o: src/drivers/common/can_buffer_store.c  .generated_files/flags/default/586973e5394566caf264b0a0ec82ff2724655ba7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers/common" 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/common/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/src/drivers/common/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/common/can_buffer_store.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/ecan1_helper.o: src/drivers/ecan1_helper.c  .generated_files/flags/default/d2722a54d0c494102504d394e75b3e59f2bede46 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/ecan1_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/ecan1_helper.c  -o ${OBJECTDIR}/src/drivers/ecan1_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/ecan1_helper.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_mcu.o: src/drivers/driver_mcu.c  .generated_files/flags/default/c4b901d3afee8a01aa052527c09082d550cb9d75 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_mcu.c  -o ${OBJECTDIR}/src/drivers/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_mcu.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_can.o: src/drivers/driver_can.c  .generated_files/flags/default/16275febd9d2d48047215e1d65e68c8bf59e6d1c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_can.c  -o ${OBJECTDIR}/src/drivers/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_can.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_100ms_clock.o: src/drivers/driver_100ms_clock.c  .generated_files/flags/default/8f35b9f024eeca42d1615a536d74970baff6bfe3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/src/drivers/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_100ms_clock.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/drivers/driver_configuration_memory.o: src/drivers/driver_configuration_memory.c  .generated_files/flags/default/e5418751541ae66f2ad9428a402196244f6e074c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/drivers" 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/src/drivers/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/src/drivers/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/drivers/driver_configuration_memory.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o: src/openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/80d9f32e48651dc2a350080b4347b622d2eb70a8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_node.o: src/openlcb/openlcb_node.c  .generated_files/flags/default/2e512e6d013591ca717c94b8604d244507cb57f8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_node.c  -o ${OBJECTDIR}/src/openlcb/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_node.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_datagram.o: src/openlcb/protocol_datagram.c  .generated_files/flags/default/b39732a6fa91eccf553d74ea160736dde684c2a5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_datagram.c  -o ${OBJECTDIR}/src/openlcb/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_datagram.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o: src/openlcb/openlcb_buffer_store.c  .generated_files/flags/default/d512c4f5f314355f834832aa49f3b0334d6d5676 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_store.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_utilities.o: src/openlcb/openlcb_utilities.c  .generated_files/flags/default/593f08272896cc34e899dcb5482987e3aa192336 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/src/openlcb/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_utilities.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_event_transport.o: src/openlcb/protocol_event_transport.c  .generated_files/flags/default/78e6b0c46be51761a01c152fc63ac8b2b582be4d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/src/openlcb/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_event_transport.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o: src/openlcb/openlcb_buffer_list.c  .generated_files/flags/default/e49663cced63def0aef1f3eada5341775dbfc1c8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_buffer_list.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_snip.o: src/openlcb/protocol_snip.c  .generated_files/flags/default/ed491a23527d663d1165e68d047c44a90ca067b2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_snip.c  -o ${OBJECTDIR}/src/openlcb/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_snip.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application_callbacks.o: src/openlcb/application_callbacks.c  .generated_files/flags/default/66e2046330c6043cf8fef1565a59e6c7487e1270 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application_callbacks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application_callbacks.c  -o ${OBJECTDIR}/src/openlcb/application_callbacks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application_callbacks.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o: src/openlcb/openlcb_gridconnect.c  .generated_files/flags/default/8e72c3db410327aae0b1497c2f6042ba0bc8a9ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_gridconnect.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o: src/openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/946942fe7035e96f07f799c4e052dbc5bfea6851 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_main_statemachine.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/application.o: src/openlcb/application.c  .generated_files/flags/default/94901cbed865c950e475b49d25e62e862c4dedcf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/application.c  -o ${OBJECTDIR}/src/openlcb/application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/application.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/protocol_message_network.o: src/openlcb/protocol_message_network.c  .generated_files/flags/default/82699e8ce846ff3dfd3afc0ed34e22be72fb0eb9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/protocol_message_network.c  -o ${OBJECTDIR}/src/openlcb/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/protocol_message_network.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o: src/openlcb/openlcb_tx_driver.c  .generated_files/flags/default/5d3371fbf567311de7f0345ef33cdcb3e7d8bb1f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb/openlcb_tx_driver.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/ddfe6c65227dab23e08a466c7074427d1b7b9afa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/bootloader_drivers.o: bootloader_drivers.c  .generated_files/flags/default/b0937a24fc8767f07ed33cccb7c2c3196d78bbd6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  bootloader_drivers.c  -o ${OBJECTDIR}/bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/bootloader_drivers.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/510724dea5b7efd81e53f7a2178d0aad029566d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/4be6fde0627ae5babadcbe0c3fac3476eb1074e6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/uart_handler.o: uart_handler.c  .generated_files/flags/default/799e10490e459471b1dc3df4db5b143458a002f2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/uart_handler.o.d 
	@${RM} ${OBJECTDIR}/uart_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  uart_handler.c  -o ${OBJECTDIR}/uart_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/uart_handler.o.d"      -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mchp-stack-usage     -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/Bootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mchp-stack-usage -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
