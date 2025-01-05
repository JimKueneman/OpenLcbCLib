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
FINAL_IMAGE=${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/callback_hooks.c ../../../openlcb/protocol_snip.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c traps.c local_drivers/_25AA1024/25AA1024_driver.c debug.c main.c node_parameters.c microcap_drivers.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/callback_hooks.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/microcap_drivers.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/callback_hooks.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d ${OBJECTDIR}/_ext/954557028/driver_can.o.d ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/microcap_drivers.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/callback_hooks.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/microcap_drivers.o

# Source Files
SOURCEFILES=../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/callback_hooks.c ../../../openlcb/protocol_snip.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../drivers/driver_mcu.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c traps.c local_drivers/_25AA1024/25AA1024_driver.c debug.c main.c node_parameters.c microcap_drivers.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP256GP504
MP_LINKER_FILE_OPTION=,--script=p33EP256GP504.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/86b31495e8336026842c7ae9d2e5fad731db8674 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/7bd8c8fdcaeaa5069dc408608f13da6f8bc1af80 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/92576b0c2e851613f8eb5ed0db33324cce030942 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/c19778d213219620dc9ab1e065762b1a24d767b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/da3ff1de8160bcd53b28a3ec18a34e04829285bf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/c98bce8959d4d8a855401604a12b8f73c196961e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/26e7eacb0136afd99529134189052657581a0b0a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/callback_hooks.o: ../../../openlcb/callback_hooks.c  .generated_files/flags/default/b3159acc710415aae6b777b1bac795d5248d12a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/156864402/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/callback_hooks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/577cf6eaefbd08971dbb8f0fcd07bbaa639ad484 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/6ac9ffe5263eb96d4572b2a2868a359afbf1e9f3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/637a48d79651d531abbe498754e70c3eda0bd55 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/c04a780ed98aa1033707c06fcec8d6315a7ed594 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/2afbeb858bdb158087b3d4c36dc7adecd8f60551 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/e64d6b1036fa48ce8804f3e6ffa5891d4439658e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/817e06ca575ed2cf01b9fb6918e1bd5db43b001 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/72c67e8798b6969dd9c4f03e87af86f4152350a8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/6ae81802c7949eb277aab4daf85667948140c7c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/e77da1eea2812bf588284666a28ed8bb7c6853c6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/657b83779851f532dd1fdaa29d638def92ac5aff .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/18e1250a1e916edb6e930daf8fde566714df941a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/dd2e5dcd9bd6a301f5cb3d70f7784fd171db16ad .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/d7f19cd626a25e99f22e5d745f0d20695a329de7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/f848efb1ca743a435ca593c998f57d33370105bb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/4a7ac827f7a3ebf9a2ecc3b79f05145c2846019f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/bea7f0063d7e2735c976bf7ed6ab3bee847331ee .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/410b4d034fa348d3b19a240c3f33870e54ae859f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/dbf888798a7d402845f83241f55b34ee670495be .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/3b5e0cb646a90ce2c8554b6cfd103c069d3effb3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/a2d9707a0934b3120f190240bbb29cd4d8877711 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/ef2c3f0ec604cf2738f0d038a9c16c1d48b2b922 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/microcap_drivers.o: microcap_drivers.c  .generated_files/flags/default/f66342a35a4f9ae2a61009463a4d62c6a9f14b8d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/microcap_drivers.o.d 
	@${RM} ${OBJECTDIR}/microcap_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  microcap_drivers.c  -o ${OBJECTDIR}/microcap_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/microcap_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/e2a04495708b0978eb146d218336c22a63afebab .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/ef4a5cc47a2ea3bdc21eb0c3ddc71e625e2a15fb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/a4b13defb49f580b8732dec15fe3385043be580d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/a32de72265462119209b5e340657ec1632c18163 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/81726827f9733d95a23d04d0141a63279c5e530e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/e85225fc9a0604ae729b0db0473b3be61e3719d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/42b20cdfa6f24284441d17ab1ef4c152b2bfed4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/callback_hooks.o: ../../../openlcb/callback_hooks.c  .generated_files/flags/default/224b7136228b77f2e1dec02225d7a92616eea97b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/156864402/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/callback_hooks.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/c33803aeae086e9bff969d88ce861d449a5dd0f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/943c5d9c8b23cfc57e428515a81edccdc4ad6f89 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/4bfd47a6147b5baa496df79e89ada17632312577 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/f3aa8f6a288790ab2b01fec2c91476651235469e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/e265b55c98706f267b3312535692a1e11df8bf84 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/68a78a6923590a5b250a49ee906f83d06ccd5769 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/9b05be9492bc4ec04c0a5dbaedcda1ab7e861c6e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/7c542ccb43bc08cdc0d046039b0e1758c2fbe790 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/2cbf3bfe201c25291ad1c70ba08d251710d79a25 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/89889f6ba7987be60716972c38e4ca6332ea02a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/748f93d6bdb9d8698457f49dd5d2f577ca3d9d88 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/a2de2685c700b5fb950d66b71e6cafafd19c40d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/93fdb131c9808e1a4e0cb447adb26a309b34a928 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/39a183b0d829d998ee64e7a4e429a39e57419982 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/6e149240c0535cf00aa2299544844aa9215d8bf0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/fae48c9513e23247c586e055a0da6ae52ca262ce .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/19aec00ab4ead7cacd22938d9e6316f0736424f0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/1dc8a61483a22bf022194e28b16c3fff0cce75c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/388a3dd22d202f99c0d0362c385af5ba67821665 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/762da89d79ee00203a7b1b311086a38abf7accae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/63e960bafdb93f2091bac2c4b524fe8d6865e520 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/2dab9ce576d45aedd6d001e2c9ce57506e442b46 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/microcap_drivers.o: microcap_drivers.c  .generated_files/flags/default/46ce7335de53252085029446c71e3832b972d3d7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/microcap_drivers.o.d 
	@${RM} ${OBJECTDIR}/microcap_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  microcap_drivers.c  -o ${OBJECTDIR}/microcap_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/microcap_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/MicroCap.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
