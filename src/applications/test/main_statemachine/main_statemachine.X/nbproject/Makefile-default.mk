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
FINAL_IMAGE=${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../../../../drivers/25AA1024/25AA1024_driver.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_main_statemachine.c ../../../../drivers/common/can_login_message_handler.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_frame_message_handler.c ../../../../drivers/common/can_rx_statemachine.c ../../../../drivers/common/can_tx_statemachine.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c traps.c debug.c main.c node_parameters.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_node.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c ../../../../openlcb/openlcb_main_statemachine.c ../../../../openlcb/clock_distribution.c ../../../../openlcb/callback_hooks.c ../../../../openlcb/protocol_event_transport.c ../../../../openlcb/protocol_message_network.c ../../../../openlcb/openlcb_tx_driver.c ../../../../openlcb/protocol_snip.c ../../../../openlcb/protocol_datagram.c ../../../../openlcb/openlcb_gridconnect.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/861666455/driver_mcu.o ${OBJECTDIR}/_ext/861666455/driver_can.o ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_node.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/526222337/clock_distribution.o ${OBJECTDIR}/_ext/526222337/callback_hooks.o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o ${OBJECTDIR}/_ext/526222337/protocol_snip.o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d ${OBJECTDIR}/_ext/216671855/can_utilities.o.d ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d ${OBJECTDIR}/_ext/861666455/driver_mcu.o.d ${OBJECTDIR}/_ext/861666455/driver_can.o.d ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/861666455/driver_mcu.o ${OBJECTDIR}/_ext/861666455/driver_can.o ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_node.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/526222337/clock_distribution.o ${OBJECTDIR}/_ext/526222337/callback_hooks.o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o ${OBJECTDIR}/_ext/526222337/protocol_snip.o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o

# Source Files
SOURCEFILES=../../../../drivers/25AA1024/25AA1024_driver.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_main_statemachine.c ../../../../drivers/common/can_login_message_handler.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_frame_message_handler.c ../../../../drivers/common/can_rx_statemachine.c ../../../../drivers/common/can_tx_statemachine.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c traps.c debug.c main.c node_parameters.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_node.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c ../../../../openlcb/openlcb_main_statemachine.c ../../../../openlcb/clock_distribution.c ../../../../openlcb/callback_hooks.c ../../../../openlcb/protocol_event_transport.c ../../../../openlcb/protocol_message_network.c ../../../../openlcb/openlcb_tx_driver.c ../../../../openlcb/protocol_snip.c ../../../../openlcb/protocol_datagram.c ../../../../openlcb/openlcb_gridconnect.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP256GP502
MP_LINKER_FILE_OPTION=,--script=p33EP256GP502.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o: ../../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/f0766942e8c4970e78ceb5dfaa208125ce12606 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1327033416" 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/4d378ce42bdd78881dd466f7b6daa8ab5c0d71cf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_main_statemachine.o: ../../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/c3fd1926929e1a13e2516b3ba58cff515a6839b6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_login_message_handler.o: ../../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/651157aaf8ee0ff870f86525f63c7036846435a9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/5842c7085f622ff520fc639d830803d152507fb2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o: ../../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/fd45db8e75c407db0b5acb26303223d29b7a212d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/a26e6b4fd6be86339f052b1cd8dd6367dc4a035c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o: ../../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/74ef4716418a6f7e2087ce3a6fdf3d71fab67679 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/6561edfb1a1319ae14f327d7f20424b029ed0d98 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_mcu.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  .generated_files/flags/default/9591e32f7ab10c287ec29f1c163d0694bdd6d74a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  -o ${OBJECTDIR}/_ext/861666455/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_can.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  .generated_files/flags/default/6d09a293d5047a11a66e3b1ced0a59bf47a03d0a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  -o ${OBJECTDIR}/_ext/861666455/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  .generated_files/flags/default/78fdaf277a2dbb0026a9f790e024a79363554ac0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  .generated_files/flags/default/8103ded4472a8c6f680b7bcdd30b2dcfd32e3425 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/342022b7853b985ae67cda5a046bd5ac846a9c83 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/407bba20a272d4412f933a06e253a269f402ef2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/2dee31badc74ebcb1be09ff02c1a8cb0f0934f6b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/19bbbe9afbd7d5948f48c7c12a089dfd96cca367 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/50365ab6fc178705e409cd4d0353fd3438fba074 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_node.o: ../../../../openlcb/openlcb_node.c  .generated_files/flags/default/6dfda5565519afdc78803a1b06017568d0fdda3a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/11fe9b9cc563fe82891b22227b8bd7657dced7f5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/e65a06294195913970082fbebfb0d33b8cae0f59 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/e71a6646b74672d830905a41bb75f9b11b02a220 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o: ../../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/e8bcf6166df3367a68df4a2c3f3336ce1cb76a72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/clock_distribution.o: ../../../../openlcb/clock_distribution.c  .generated_files/flags/default/2f295e50f69c8bc723fcc466de6b23be6c7c547 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/526222337/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/clock_distribution.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/callback_hooks.o: ../../../../openlcb/callback_hooks.c  .generated_files/flags/default/a28058b2b1e17e67f6020288b9a3b88c3d92e71a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/526222337/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/callback_hooks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_event_transport.o: ../../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/af809fb3d27feb67296d25a39cb1eed4197a8546 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_message_network.o: ../../../../openlcb/protocol_message_network.c  .generated_files/flags/default/166b893d7da280ccb8d0541fcdd133e6b2b29589 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o: ../../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/5a3a2dfdf65af00606f27e6fe33656f2d70a9d87 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_snip.o: ../../../../openlcb/protocol_snip.c  .generated_files/flags/default/c1a27b1b365eb26b2e90d3cc3b5b448f8bd526e3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/526222337/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_datagram.o: ../../../../openlcb/protocol_datagram.c  .generated_files/flags/default/50de6ad1ab1dd4f571555ba0ac7f0617d0b268d0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o: ../../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/1fdc4dcbe222aa68901d710a129718687e41e522 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o: ../../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/deb4984ff702471503c815f1bc789a18a7e45e91 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1327033416" 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/3a95e6d19ba78a235cf130d5acdca0945beca0b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_main_statemachine.o: ../../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/b7c7c4ce74a514439f67e55b4af9eafde0b38f91 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_login_message_handler.o: ../../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/7b9713645f54faf717e5163c8f5cd8387473fb71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/50829b5e9fa2a06d8b870379bbae95d55673baa9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o: ../../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/6d23648cd3f7b138d498abbd0236476a006c060a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/cbd6cfd21f7fb81a5a0cf2de0837afcdd20b033c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o: ../../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/c268729e8f963754991f66dc43bca8f6a7c1824a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/b267be88bccbd68466130aee6177f62ffb3beaf3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_mcu.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  .generated_files/flags/default/a2979a0786359564de9707efaa7d0299271641ac .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  -o ${OBJECTDIR}/_ext/861666455/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_can.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  .generated_files/flags/default/83f3c78f6c73565f86428e6650221d6feb5fb2dc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  -o ${OBJECTDIR}/_ext/861666455/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  .generated_files/flags/default/6b03df01dd828aa184c244ee3cb0a5431d95395e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  .generated_files/flags/default/b5335ac7c540283ba199444c92802a18fbaace25 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/ec915f8ff4f593973c3f7619b2f52c98ec87cca5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/e855117e42c637434062b98ec935bead165d2dc4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/d64c1b1694fc984d2701acaeba194758e8868bd9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/902322e5ceece3b0c02f7442a194e7cdeb69b93 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/88b2a9d5add0ee6824c6ff6b4e9813202de56c30 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_node.o: ../../../../openlcb/openlcb_node.c  .generated_files/flags/default/ede233ea29978040408c9ea61561997d52226cfb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/8c6c70ed9de2e1b349081d0422c6893430019e54 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/2f1bf6d1dc2e53922efa027b1306356b805babbe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/3b741439721d5f27beb1d366bcef34a595956e07 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o: ../../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/f08928c8522aff0975b0fa3691094987bd307ec .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/clock_distribution.o: ../../../../openlcb/clock_distribution.c  .generated_files/flags/default/be2ce7f8230a9fcc2a3358c9706b6c93a2419221 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/526222337/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/clock_distribution.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/callback_hooks.o: ../../../../openlcb/callback_hooks.c  .generated_files/flags/default/bcdaf821e7a1ca6d8d3e1f22945ce8cb8738612a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/526222337/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/callback_hooks.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_event_transport.o: ../../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/5a528d7ec65bca2c77c48ed547ce86a286421929 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_message_network.o: ../../../../openlcb/protocol_message_network.c  .generated_files/flags/default/8b0d64e700b0977d7adc40ac5b6141d33f5ceec4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o: ../../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/115e145b327cb304e355567720f4bed69d4405b4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_snip.o: ../../../../openlcb/protocol_snip.c  .generated_files/flags/default/6e1e4dd3675cef00280a8e52f335f5f59c064459 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/526222337/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_datagram.o: ../../../../openlcb/protocol_datagram.c  .generated_files/flags/default/77631220c4807334ab035f559a7e7623e5a8e517 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o: ../../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/b383308583c4bc3c85fcfc8d553c0a1c389d7cfd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DdsPIC33EPxxxGP50x -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
