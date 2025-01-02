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
${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o: ../../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/e78882f1f56bd9b2cc42cc6a7deb4b3342eb9450 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1327033416" 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/a97f6d3d065c153becc9ac3c8e7661985a75b35c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_main_statemachine.o: ../../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/e68eb52aa1831e07db46aa13d550e7937b2e7b71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_login_message_handler.o: ../../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/d2fdce58932b0911d0f82ef5b39357a6401139c2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/f1cb53ff71f74e0653ea5846d4fd977da60fc8f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o: ../../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/91dc6178f6f1482136e563b76817fa4957b0889c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/512018a2f698d005e4956c9183fbab2cb8d3bfa1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o: ../../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/950fc99c93cac1eaa4f44ef645cd160a3fc06613 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/d1a0ddfc95286504ae5fde4516aebeec146b5358 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_mcu.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  .generated_files/flags/default/c1592d31b9a8048d4ea23ef378275a0f38fd63ec .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  -o ${OBJECTDIR}/_ext/861666455/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_can.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  .generated_files/flags/default/17e81418c27370b78e0236afbb7d4983b8d4272f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  -o ${OBJECTDIR}/_ext/861666455/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  .generated_files/flags/default/aae1718e5227f87896f2a961fabb2d2657b75992 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  .generated_files/flags/default/b15f10e5f3067bd30dba42e5447e7633dec63a7f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/6fad1a06df16abc2575d9ba78c4be4e35c971409 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/c089d57148ddb8382b8b09276ab85a86426ce624 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/a5d8d6b495133e540d645dad889dfd84dc25c91 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/c29ed27dc1b567a4a07e40c229d8a7b954566865 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/c8023467e79f9196defadd208a5fe1727c205e8b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_node.o: ../../../../openlcb/openlcb_node.c  .generated_files/flags/default/4c4d6979796533320dc5977b51d238fc6bfc2abe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/b0f604359d0eb2bed40d586852f194534b801591 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/f8e767d4d52ef3789a653361ab5ead733eb7e90e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/f392a0581ea7799da4ea414a590d760f99f81e4f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o: ../../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/273ae659d8d4bb7914b9708d66d4a619dbe71c51 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/clock_distribution.o: ../../../../openlcb/clock_distribution.c  .generated_files/flags/default/f94580e035d8c7a8ca7b6a1f3624aa3f9fe9e61c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/526222337/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/clock_distribution.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/callback_hooks.o: ../../../../openlcb/callback_hooks.c  .generated_files/flags/default/a2a42bae557db94fb54d9576a305fd1a95fb2b6f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/526222337/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/callback_hooks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_event_transport.o: ../../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/cf559131a723a50362e8089fa2626ad88483f9c1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_message_network.o: ../../../../openlcb/protocol_message_network.c  .generated_files/flags/default/32dda6c34d1c63ed5156ffaf8ec71cf4a8c31cb5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o: ../../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/7ef8b4d1e5656428202d6ce09f7b9bb933a20f24 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_snip.o: ../../../../openlcb/protocol_snip.c  .generated_files/flags/default/372bf8039e62c7402acdd14c9bec025a4f19cf5b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/526222337/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_datagram.o: ../../../../openlcb/protocol_datagram.c  .generated_files/flags/default/6d102478400e64ca99f2e5acb7d3bfcadd1ca2b6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o: ../../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/cb170d3f1bb73b02260b2401a6926501e7155634 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o: ../../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/772e1cd0b8b0b366d9d471d2c185768e58d082c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1327033416" 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/5c512fd4be24a10bd18c98ca3b7b7534c492b09c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_main_statemachine.o: ../../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/19aab6664855b903730b62b5ff9ada05bfe987f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_login_message_handler.o: ../../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/1ad19b8ec01987a2ba5940a8c7845fdad81e0743 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/187b631405595f5b26aea0a8d643e4d30cd4246a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o: ../../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/f028fd1a1ea69b8fa65c9ff719d2a90eb7206b95 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/3265c597f882e957df972e9e9e6b9eda4dfa400c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o: ../../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/2f7ce40913f496df3abd74b155613ab2a12fd623 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/762c065873e464704870dd68ba0b37b22bf5c843 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_mcu.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  .generated_files/flags/default/ce94fb150b729bca5ae58f671706d66d37ea1e70 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_mcu.c  -o ${OBJECTDIR}/_ext/861666455/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_can.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  .generated_files/flags/default/b8adb4ff7684fba954fe86eb35424dcaa57fca1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_can.c  -o ${OBJECTDIR}/_ext/861666455/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  .generated_files/flags/default/191b654488cdc5137a354bff4356fa00a31b5924 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  .generated_files/flags/default/cb85580f6a3f483695a387a237c7b53dd414beb4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/5a04d03536b83c81e9184c15b910af7a53c944f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/559d2d44e1aa7453dedb5ad890c8d8da45b3f438 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/a8c5f98f5ef2f152ca55f0f43047d4c7f9f01f9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/e409e0057b921a0cd4d0dbd585c5c467be810dc7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/10ff2ba59653302d1502f6801fe22372f975fb7e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_node.o: ../../../../openlcb/openlcb_node.c  .generated_files/flags/default/dd0d3c248dea6663c6a3f83065d688d80d1a4605 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/68f5157e9f1ebbb32e33989754626038b51f7021 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/2ca574c85d40220e4b5aba25816961a1b095bb26 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/d352e91873b1dd1093ede0828dca9a7f4e9222e9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o: ../../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/88c15561a438471a3e6b03e3bd4bfaf99bd8264 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/clock_distribution.o: ../../../../openlcb/clock_distribution.c  .generated_files/flags/default/44a614b50782cac2a05b7b54cd650cfda7fc8880 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/526222337/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/clock_distribution.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/callback_hooks.o: ../../../../openlcb/callback_hooks.c  .generated_files/flags/default/5ce23613c64b4862394f3d9ad8c62dad04dfe922 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/526222337/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/callback_hooks.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_event_transport.o: ../../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/95fd4888012a2fade436b40bc6bf8c0e4f4c8d18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_message_network.o: ../../../../openlcb/protocol_message_network.c  .generated_files/flags/default/7fd46892377e6a36066ddeef9ff0a570d3c6550 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o: ../../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/997a90619bfd7271281720af6e7eac254c0615b6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_snip.o: ../../../../openlcb/protocol_snip.c  .generated_files/flags/default/6fd14d11a8dae4bcc9b62b4b87b1df5a13e7b939 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/526222337/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_datagram.o: ../../../../openlcb/protocol_datagram.c  .generated_files/flags/default/5c6f1bc35cd331dc6a5bb28cd6a4c00c8aaeff00 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o: ../../../../openlcb/openlcb_gridconnect.c  .generated_files/flags/default/7d1eb1de2f0709d4f5f587c5f892485a97531fc9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DSUPPORT_FIRMWARE_BOOTLOADER -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
