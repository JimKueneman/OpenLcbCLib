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
SOURCEFILES_QUOTED_IF_SPACED=../../../../drivers/25AA1024/25AA1024_driver.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_rx_statemachine.c ../../../../drivers/common/can_tx_statemachine.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/common/can_main_statemachine.c ../../../../drivers/common/can_login_message_handler.c ../../../../drivers/common/can_frame_message_handler.c ../../../../drivers/driver_configuration_memory.c ../../../../drivers/driver_mcu.c ../../../../drivers/driver_100ms_clock.c ../../../../drivers/driver_can.c traps.c debug.c main.c node_parameters.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_node.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c ../../../../openlcb/openlcb_main_statemachine.c ../../../../openlcb/clock_distribution.c ../../../../openlcb/callback_hooks.c ../../../../openlcb/protocol_event_transport.c ../../../../openlcb/protocol_message_network.c ../../../../openlcb/openlcb_tx_driver.c ../../../../openlcb/protocol_snip.c ../../../../openlcb/protocol_datagram.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/can.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o ${OBJECTDIR}/_ext/585199093/driver_mcu.o ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o ${OBJECTDIR}/_ext/585199093/driver_can.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_node.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/526222337/clock_distribution.o ${OBJECTDIR}/_ext/526222337/callback_hooks.o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o ${OBJECTDIR}/_ext/526222337/protocol_snip.o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o ${OBJECTDIR}/_ext/861666455/clock.o ${OBJECTDIR}/_ext/861666455/mcu.o ${OBJECTDIR}/_ext/861666455/configuration_memory.o ${OBJECTDIR}/_ext/861666455/can.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/216671855/can_utilities.o.d ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o.d ${OBJECTDIR}/_ext/585199093/driver_mcu.o.d ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o.d ${OBJECTDIR}/_ext/585199093/driver_can.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/node_parameters.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d ${OBJECTDIR}/_ext/861666455/clock.o.d ${OBJECTDIR}/_ext/861666455/mcu.o.d ${OBJECTDIR}/_ext/861666455/configuration_memory.o.d ${OBJECTDIR}/_ext/861666455/can.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o ${OBJECTDIR}/_ext/585199093/driver_mcu.o ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o ${OBJECTDIR}/_ext/585199093/driver_can.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/node_parameters.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_node.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o ${OBJECTDIR}/_ext/526222337/clock_distribution.o ${OBJECTDIR}/_ext/526222337/callback_hooks.o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o ${OBJECTDIR}/_ext/526222337/protocol_snip.o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o ${OBJECTDIR}/_ext/861666455/clock.o ${OBJECTDIR}/_ext/861666455/mcu.o ${OBJECTDIR}/_ext/861666455/configuration_memory.o ${OBJECTDIR}/_ext/861666455/can.o

# Source Files
SOURCEFILES=../../../../drivers/25AA1024/25AA1024_driver.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_rx_statemachine.c ../../../../drivers/common/can_tx_statemachine.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/common/can_main_statemachine.c ../../../../drivers/common/can_login_message_handler.c ../../../../drivers/common/can_frame_message_handler.c ../../../../drivers/driver_configuration_memory.c ../../../../drivers/driver_mcu.c ../../../../drivers/driver_100ms_clock.c ../../../../drivers/driver_can.c traps.c debug.c main.c node_parameters.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_node.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c ../../../../openlcb/openlcb_main_statemachine.c ../../../../openlcb/clock_distribution.c ../../../../openlcb/callback_hooks.c ../../../../openlcb/protocol_event_transport.c ../../../../openlcb/protocol_message_network.c ../../../../openlcb/openlcb_tx_driver.c ../../../../openlcb/protocol_snip.c ../../../../openlcb/protocol_datagram.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c ../../../../drivers/dspic/dsPIC33xxxGP5xx/can.c



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
${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o: ../../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/f16f2bb75b0bf87624726ebc001df1acfd105c18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1327033416" 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/e8cd147a7159417d7721b3eb163acb5be5c98017 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/73ce1d3e29a82b64761b201e01b1f68143b38526 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/f929b426aca577ec22eb1af7a9f86f42cc85bb1f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o: ../../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/599974958b265ff1137d29afe30827b40da1cf2b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/ecc29a412794ef68a2cab0cee01f26b9a8f185c1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_main_statemachine.o: ../../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/122c51bc61ad7222e96c70c36eb40103cf11497d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_login_message_handler.o: ../../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/26bedfa28b05df1985245bd3c971630dfb4c79f8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o: ../../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/19842d701595523fdd6e551355f8c9a52e1a5a60 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o: ../../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/1ebc35e7913288531d84d4453e3c3e46d32f36a9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_mcu.o: ../../../../drivers/driver_mcu.c  .generated_files/flags/default/6d22b5e0c2e9caaefb40085b9d2a58fb2ef333f0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/585199093/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o: ../../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/62a0f2913f1281b6a355c790021426d3f930aaa7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_can.o: ../../../../drivers/driver_can.c  .generated_files/flags/default/20df306c284a947b4466cd3871859356b923de79 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/585199093/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/fe178d7ebe2276f65406bd9e48c67715019e6390 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/19f8255cff1f9b7a142aa9d5f7fddb65021e768 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/93b66ac7e737822abe042a7660667b047a0bb85 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/55e07f2f10f1e64b1f4ac4165416c33fa71913e9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/a5705873b6469a65cffde5da31dfaed185ab671e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_node.o: ../../../../openlcb/openlcb_node.c  .generated_files/flags/default/fb124d0dbf2ee03017eab16aa2ac50e79de34e59 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/7e759ff1dd98b0b10011c81b6beaa6aa05ffe8ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/73e4cd12e2eaef467993bd354123741da274dcb1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/eb662ca31ebff0aad332a16ec105868735c59caa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o: ../../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/33029b2efa1d9de12610a4f4c3837d967e0bada2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/clock_distribution.o: ../../../../openlcb/clock_distribution.c  .generated_files/flags/default/8fd297171a72718d5655b36dff2841af458ea653 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/526222337/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/clock_distribution.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/callback_hooks.o: ../../../../openlcb/callback_hooks.c  .generated_files/flags/default/4d69b95a7e2cf94f8537202436800607eefcde8b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/526222337/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/callback_hooks.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_event_transport.o: ../../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/8ee988a293454ecfe10bab980199298630a5862a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_message_network.o: ../../../../openlcb/protocol_message_network.c  .generated_files/flags/default/c4a23b2c856ba076c6ffd9c16777c688e023e1de .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o: ../../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/14b503a43ae71e986265b3516337f642c9eb53d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_snip.o: ../../../../openlcb/protocol_snip.c  .generated_files/flags/default/8374901b50c8d867812c031e1f3f3b00dc89f26b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/526222337/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_datagram.o: ../../../../openlcb/protocol_datagram.c  .generated_files/flags/default/35a473b152fb1ae7880966183db67b39d2a174f6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/clock.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  .generated_files/flags/default/57ed26f9489eceac2e48de2183956136ccc23137 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  -o ${OBJECTDIR}/_ext/861666455/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/clock.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/mcu.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  .generated_files/flags/default/8dbf6a9d54fa4b9dc6f4ed9340da8d5bd688bf2c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  -o ${OBJECTDIR}/_ext/861666455/mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/mcu.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/configuration_memory.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  .generated_files/flags/default/d974d33075062282336355f8ecdd9005843d8091 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  -o ${OBJECTDIR}/_ext/861666455/configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/configuration_memory.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/can.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  .generated_files/flags/default/af83d85e4d7b22d048b3460342a71389f5852f65 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/can.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  -o ${OBJECTDIR}/_ext/861666455/can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o: ../../../../drivers/25AA1024/25AA1024_driver.c  .generated_files/flags/default/95970a9c3c000a504a5cf418a7fdf38981842a8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1327033416" 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1327033416/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/aaa64fffe52777867c9155a6d78de246508dd2d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/cba6b5b45e5e4616816eecd7e9f1539cf9b54c1a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/cb0057dd71b7c6db654d35a65d4c18dd543e81b4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o: ../../../../drivers/common/can_tx_statemachine.c  .generated_files/flags/default/6f1d1933876aaa24e46e7fcefb371d9276bb4336 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_tx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/897af9bac2ff85ff2e93828914687deef2486360 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_main_statemachine.o: ../../../../drivers/common/can_main_statemachine.c  .generated_files/flags/default/858c9d8c6dca4ad8fbf7db1654bfc8b6380b3425 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_main_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_login_message_handler.o: ../../../../drivers/common/can_login_message_handler.c  .generated_files/flags/default/fcbfb00139d1ad510ae1f6673a6602c39040c7b5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_login_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o: ../../../../drivers/common/can_frame_message_handler.c  .generated_files/flags/default/9c8340319fcf547ae7b705cd517a891019594b72 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_frame_message_handler.c  -o ${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_frame_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o: ../../../../drivers/driver_configuration_memory.c  .generated_files/flags/default/d2f09fa335ab87823e58306ce95e5afa20bd55f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_configuration_memory.c  -o ${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_mcu.o: ../../../../drivers/driver_mcu.c  .generated_files/flags/default/4f68bdfbc741ea5067f05cac106765c50f1cb07f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_mcu.c  -o ${OBJECTDIR}/_ext/585199093/driver_mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_mcu.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o: ../../../../drivers/driver_100ms_clock.c  .generated_files/flags/default/12f592c6ea65ec779a8587d0187571a1992d032f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_100ms_clock.c  -o ${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_100ms_clock.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/driver_can.o: ../../../../drivers/driver_can.c  .generated_files/flags/default/eaa823da13d91736bfcd7b01b85e480539c6135 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/driver_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/driver_can.c  -o ${OBJECTDIR}/_ext/585199093/driver_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/driver_can.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/60e07f026d002d288971d486b9d267e2f0fce0d9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/1f1c4accc3d2fe44410988039b8d4197706112e5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/da033affde7bde8946f7f9946bcad995c080104f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/node_parameters.o: node_parameters.c  .generated_files/flags/default/6168b45b2631254301da60b59f1027b9bd80b5e3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/node_parameters.o.d 
	@${RM} ${OBJECTDIR}/node_parameters.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  node_parameters.c  -o ${OBJECTDIR}/node_parameters.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/node_parameters.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/a3754d45dd226cd0bd960354ddedbf3964bac88a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_node.o: ../../../../openlcb/openlcb_node.c  .generated_files/flags/default/a46559be01918295b80999fa5ca86242b96b5712 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_node.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/7f025d9a6ac41009a90f3e55135e0cbb6ac5f51f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/4a80e68735111b4d8c2c55b80bc96913a7fd1ec .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/cc9f8369cfad55052b4ebd087a93418298e3bcf7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o: ../../../../openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/c581bc32d4ff989525ef2f039940ec619c86c39a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/clock_distribution.o: ../../../../openlcb/clock_distribution.c  .generated_files/flags/default/163b34281ddd4fa90882b611959deb2949209c73 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/clock_distribution.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/clock_distribution.c  -o ${OBJECTDIR}/_ext/526222337/clock_distribution.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/clock_distribution.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/callback_hooks.o: ../../../../openlcb/callback_hooks.c  .generated_files/flags/default/ff5c242b7ce7f19879dd6704359489a345950f85 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/callback_hooks.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/callback_hooks.c  -o ${OBJECTDIR}/_ext/526222337/callback_hooks.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/callback_hooks.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_event_transport.o: ../../../../openlcb/protocol_event_transport.c  .generated_files/flags/default/f61031ff648896eed6d9b6a732cdaa041040d151 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/_ext/526222337/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_message_network.o: ../../../../openlcb/protocol_message_network.c  .generated_files/flags/default/bbcafbc9ccb317e65c343e3904170677f2038b9c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_message_network.c  -o ${OBJECTDIR}/_ext/526222337/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o: ../../../../openlcb/openlcb_tx_driver.c  .generated_files/flags/default/b7d718bd7767e22148a15069d3d702b63b7fef8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_tx_driver.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_tx_driver.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_snip.o: ../../../../openlcb/protocol_snip.c  .generated_files/flags/default/246fe49fb2c51a626b42ee39558e48ee5783fbfc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_snip.c  -o ${OBJECTDIR}/_ext/526222337/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/protocol_datagram.o: ../../../../openlcb/protocol_datagram.c  .generated_files/flags/default/a6aa69cdd91ea12930fe4c57b42c96c5040605cd .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/protocol_datagram.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/protocol_datagram.c  -o ${OBJECTDIR}/_ext/526222337/protocol_datagram.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/protocol_datagram.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/clock.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  .generated_files/flags/default/306a9765179db2ccfc76bb0f97e7f38ea4bed756 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/clock.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/clock.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/clock.c  -o ${OBJECTDIR}/_ext/861666455/clock.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/clock.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/mcu.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  .generated_files/flags/default/832aa4ed1e947fbab3dfa5ed804dfae0bf71f041 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/mcu.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/mcu.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/mcu.c  -o ${OBJECTDIR}/_ext/861666455/mcu.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/mcu.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/configuration_memory.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  .generated_files/flags/default/5d98dde7ea966d4d16cdafdadcf167802ccfd915 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/configuration_memory.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/configuration_memory.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/configuration_memory.c  -o ${OBJECTDIR}/_ext/861666455/configuration_memory.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/configuration_memory.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/861666455/can.o: ../../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  .generated_files/flags/default/1ff90e72d9b4b4735a234c31fe907e6ae8a3afb0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/861666455" 
	@${RM} ${OBJECTDIR}/_ext/861666455/can.o.d 
	@${RM} ${OBJECTDIR}/_ext/861666455/can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/dspic/dsPIC33xxxGP5xx/can.c  -o ${OBJECTDIR}/_ext/861666455/can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/861666455/can.o.d"      -mno-eds-warn  -g -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/main_statemachine.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -D_USE_TEMPLATES_ -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
