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
FINAL_IMAGE=${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=debug.c main.c node_parameters.c traps.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/callback_hooks.c ../../../openlcb/protocol_snip.c ../../../openlcb/clock_distribution.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../drivers/driver_mcu.c ../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c ../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c ../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c ../../../drivers/dspic/dsPIC33xxxGP5xx/can.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/25AA1024/25AA1024_driver.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/callback_hooks.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/clock_distribution.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/1610978664/clock.o ${OBJECTDIR}/_ext/1610978664/mcu.o ${OBJECTDIR}/_ext/1610978664/configuration_memory.o ${OBJECTDIR}/_ext/1610978664/can.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o
POSSIBLE_DEPFILES=${OBJECTDIR}/debug.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/156864402/callback_hooks.o.d ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d ${OBJECTDIR}/_ext/156864402/clock_distribution.o.d ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d ${OBJECTDIR}/_ext/1610978664/clock.o.d ${OBJECTDIR}/_ext/1610978664/mcu.o.d ${OBJECTDIR}/_ext/1610978664/configuration_memory.o.d ${OBJECTDIR}/_ext/1610978664/can.o.d ${OBJECTDIR}/_ext/954557028/driver_can.o.d ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/156864402/openlcb_node.o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o ${OBJECTDIR}/_ext/156864402/callback_hooks.o ${OBJECTDIR}/_ext/156864402/protocol_snip.o ${OBJECTDIR}/_ext/156864402/clock_distribution.o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o ${OBJECTDIR}/_ext/954557028/driver_mcu.o ${OBJECTDIR}/_ext/1610978664/clock.o ${OBJECTDIR}/_ext/1610978664/mcu.o ${OBJECTDIR}/_ext/1610978664/configuration_memory.o ${OBJECTDIR}/_ext/1610978664/can.o ${OBJECTDIR}/_ext/954557028/driver_can.o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_utilities.o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o

# Source Files
SOURCEFILES=debug.c main.c node_parameters.c traps.c ../../../openlcb/openlcb_buffer_fifo.c ../../../openlcb/openlcb_node.c ../../../openlcb/protocol_datagram.c ../../../openlcb/openlcb_buffer_store.c ../../../openlcb/openlcb_utilities.c ../../../openlcb/protocol_event_transport.c ../../../openlcb/openlcb_buffer_list.c ../../../openlcb/callback_hooks.c ../../../openlcb/protocol_snip.c ../../../openlcb/clock_distribution.c ../../../openlcb/openlcb_gridconnect.c ../../../openlcb/openlcb_main_statemachine.c ../../../openlcb/protocol_message_network.c ../../../openlcb/openlcb_tx_driver.c ../../../drivers/driver_mcu.c ../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c ../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c ../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c ../../../drivers/dspic/dsPIC33xxxGP5xx/can.c ../../../drivers/driver_can.c ../../../drivers/driver_100ms_clock.c ../../../drivers/driver_configuration_memory.c ../../../drivers/common/can_buffer_fifo.c ../../../drivers/common/can_main_statemachine.c ../../../drivers/common/can_login_message_handler.c ../../../drivers/common/can_utilities.c ../../../drivers/common/can_frame_message_handler.c ../../../drivers/common/can_rx_statemachine.c ../../../drivers/common/can_tx_statemachine.c ../../../drivers/common/can_buffer_store.c ../../../drivers/25AA1024/25AA1024_driver.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script=p33EP512GP504.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/10caec6dd5c28e13e653e2a6182af355a479f7f5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/bf3df9f7f1cefbbba4275d030289230665d50eb3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/c41fb4bb03fa5f63a4d52f547856fdc9b1a790ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/c5fca92de7157d707ba48b0f0d5fe3c03c53643c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/c88f831965e47f1649b98041970dde5f2c243d44 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/13d2909da01aa01023c7b9772c23d26015e69611 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/fa34d3d72a6cbb5b6554d6a18cdfd79ac275e069 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/d17ce62602ab417a431bae92dea31ff829631001 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/3a03cd2d49ea859d3537f593ade0f29c9781ac1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/8993d94837fe3103848b2259dc252812e20224fe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/7f543834a1e45b31dcf485a569eaf061f27f139b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/callback_hooks.o: ../../../openlcb/callback_hooks.c  .generated_files/flags/default/3dd9604819849e11cccffbe2e425f7a311db314c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/156864402/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/callback_hooks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/e75ddda04dbefab66876687164f74e88e6d8fe85 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/clock_distribution.o: ../../../openlcb/clock_distribution.c  .generated_files/flags/default/e573c37c95acdaa5e9d00fda8684a32286662287 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/156864402/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/clock_distribution.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/318eb69c9331a4c70a42bc5c4af277837fe27cdc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/c09a06e30d193d4f0e4520a09ad32781636fa573 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/fc8b20199f1cbe0b8d8b813191bd5770976dfb1a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/ebb3ed747c006b62199ac0fd5a99eb7f6a006f7d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/94a437970fcef121fcbe6454cb481ad2cd73d214 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/clock.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  .generated_files/flags/default/52a237ebd901c28f03578a05c019aecc8c331f4f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  -o ${OBJECTDIR}/_ext/1610978664/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/mcu.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  .generated_files/flags/default/879312f7452d4aca77f1b845cc7ab7835975a148 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  -o ${OBJECTDIR}/_ext/1610978664/mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/configuration_memory.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  .generated_files/flags/default/2033e9c84fcd52f76278aabfe06424fa9a265d6a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  -o ${OBJECTDIR}/_ext/1610978664/configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/can.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  .generated_files/flags/default/3ad17e86e769f4db7b7a474a44516bd197d06da5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/can.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  -o ${OBJECTDIR}/_ext/1610978664/can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/36311c5a8e89cb60f4b4fad59935a9c037c45fa6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/aea388ad21f5c58d3a671f72a7be2b00fbb1d6f0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/2637a2d2402896cded8c27c3f94227d457377ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/ec975af572a391325e18bfe0b3a159133628fbe5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/bf516d1ef98e059d48eb87309ecc94b25655f036 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/ab68a9b9e758aa26e3e13d9aa74a8d871360705 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/9c2bf6a4887666487a148998b754696b89915103 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/5a7574d49b4568601286a7fc6730a129c03e7566 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/bd826e6968f0f5729fe8277b83c30b889031ead5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/b0ec8e40eac02d17f7e501d6ed26e7a518d40f80 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/e5effc87cffe77ca900c45bf5551d4166f5ec731 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o: ../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/4486a1ddb8e7eb5cf198db3d9120e3cf742f078e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1712477143" 
	@${RM} ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/f1a1c79f37f1bac426e7c5ac200b71e77e534724 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/de7ddc8d7d4264f9c1f2d9f71e99b8f2b40db6bb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/d1e3740b1bd409212636b9880b3e0e6e4a3a5276 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/e16fb4bfd99579b3eee214ce02a6c44dfc9b0154 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o: ../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/73856091971e350d1b3d75a575b49bf6ea702c1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_node.o: ../../../openlcb/openlcb_node.c  .generated_files/flags/default/b2e4261855bb421ee953a9c8d40ffe84d2642c54 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_datagram.o: ../../../openlcb/protocol_datagram.c  .generated_files/flags/default/850c003e2b2a97c376fc646cdd11699f6237c1b2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/156864402/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o: ../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/1c3aa3d12c94af0ca692243c0d72f78772223b5c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/f9aaeafe667ec25b3216dab58e85447a5f4bb562 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_event_transport.o: ../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/5b26db9b01ff9da94c6a6064fbe6deea24c87eea .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/156864402/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o: ../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/eb71be48d23b8b897fe3091196499d2598c18c2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/callback_hooks.o: ../../../openlcb/callback_hooks.c  .generated_files/flags/default/4066c4ad7fa3ea3f1490edd0fe49c5d901645d18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/156864402/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/callback_hooks.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_snip.o: ../../../openlcb/protocol_snip.c  .generated_files/flags/default/f98f5413586fa5a56f7e91b31a423e1b0e23d0ff .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/156864402/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/clock_distribution.o: ../../../openlcb/clock_distribution.c  .generated_files/flags/default/d52670c22e025a9abb6866657aa9d9e54077940e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/156864402/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/clock_distribution.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o: ../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/a347eac7c0592e58e346794a1e025953b37e907 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o: ../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/f2a461fa61b1728b70b3747e3969655eeadd55d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/protocol_message_network.o: ../../../openlcb/protocol_message_network.c  .generated_files/flags/default/9bf444ef8e2d8e84cbdc081075867da90e022932 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/156864402/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o: ../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/900060cb2f107fe6745d3a8204207f3f93b764df .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_mcu.o: ../../../drivers/driver_mcu.c  .generated_files/flags/default/b80190fc7b160ca527c617a4dd779777cdbeb68b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/954557028/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/clock.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  .generated_files/flags/default/5717287ba6afa817d7084f84581c7db86aebc7d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  -o ${OBJECTDIR}/_ext/1610978664/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/mcu.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  .generated_files/flags/default/28ca9f031628c30a12cd65cda6f1f0aebb124d63 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  -o ${OBJECTDIR}/_ext/1610978664/mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/mcu.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/configuration_memory.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  .generated_files/flags/default/bd5551ffa292bff5f535629901e44f7c740ca00f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  -o ${OBJECTDIR}/_ext/1610978664/configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1610978664/can.o: ../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  .generated_files/flags/default/a89d5d7c68f9ef95461c28e28c5b59feae33d772 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1610978664" 
	@${RM} ${OBJECTDIR}/_ext/1610978664/can.o.d 
	@${RM} ${OBJECTDIR}/_ext/1610978664/can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  -o ${OBJECTDIR}/_ext/1610978664/can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1610978664/can.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_can.o: ../../../drivers/driver_can.c  .generated_files/flags/default/91829649ceb97b3c3ccc35cd9b3a2c7c98556139 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/954557028/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o: ../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/e537d3e9a5f95639def66cf5c47cf2ffb02c8744 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o: ../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/3ddee5f56472ef5096d999d9cd2de6dac611b442 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/954557028" 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/954557028/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o: ../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/7f972c91788684e8e488991dda5f29f408c7e41e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o: ../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/82ae326372ed39d65a2620163fba185e61263095 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o: ../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/1d88faeb9bc07118824e0046248214df73f63558 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_utilities.o: ../../../drivers/common/can_utilities.c  .generated_files/flags/default/49078bc7c0372161c7ec40ecd5aa83ca9b80033f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/1293539906/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o: ../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/ceb3087714af941571c7bab9a5b6880e2225bbfe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o: ../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/e0f06526f8c3ad9cc085656aa19e06a9986657c9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o: ../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/80e15c7147f6d0344f027b834fd45c9a654d2c14 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1293539906/can_buffer_store.o: ../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/cf40e1f8e4fb436b823cae45b969689f1fefaf3b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1293539906" 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/1293539906/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1293539906/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o: ../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/5b43a9a572ff6fa06e85d9eea8aaeccfb97728f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1712477143" 
	@${RM} ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1712477143/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/TurnoutBOSS.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
