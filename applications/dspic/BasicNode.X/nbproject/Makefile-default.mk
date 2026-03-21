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
FINAL_IMAGE=${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
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
SOURCEFILES_QUOTED_IF_SPACED=openlcb_user_config.c traps.c main.c src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.c src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.c src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.c src/openlcb_c_lib/drivers/canbus/can_main_statemachine.c src/openlcb_c_lib/drivers/canbus/can_config.c src/openlcb_c_lib/drivers/canbus/can_login_message_handler.c src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.c src/openlcb_c_lib/drivers/canbus/can_utilities.c src/openlcb_c_lib/drivers/canbus/alias_mappings.c src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.c src/openlcb_c_lib/drivers/canbus/can_login_statemachine.c src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.c src/openlcb_c_lib/drivers/canbus/can_buffer_store.c src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c src/openlcb_c_lib/openlcb/openlcb_application_train.c src/openlcb_c_lib/openlcb/protocol_train_handler.c src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c src/openlcb_c_lib/openlcb/protocol_train_search_handler.c src/openlcb_c_lib/openlcb/openlcb_node.c src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c src/openlcb_c_lib/openlcb/openlcb_buffer_store.c src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c src/openlcb_c_lib/openlcb/openlcb_utilities.c src/openlcb_c_lib/openlcb/openlcb_float16.c src/openlcb_c_lib/openlcb/protocol_event_transport.c src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c src/openlcb_c_lib/openlcb/openlcb_buffer_list.c src/openlcb_c_lib/openlcb/protocol_snip.c src/openlcb_c_lib/openlcb/protocol_datagram_handler.c src/openlcb_c_lib/openlcb/openlcb_gridconnect.c src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c src/openlcb_c_lib/openlcb/openlcb_config.c src/openlcb_c_lib/openlcb/protocol_message_network.c src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c src/openlcb_c_lib/openlcb/openlcb_application.c src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c src/openlcb_c_lib/utilities/mustangpeak_endian_helper.c src/openlcb_c_lib/utilities/mustangpeak_string_helper.c src/application_drivers/dspic33_can_drivers.c src/application_drivers/dspic33_drivers.c src/application_callbacks/callbacks_config_mem.c src/application_callbacks/callbacks_olcb.c src/application_callbacks/callbacks_can.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/openlcb_user_config.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/main.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o ${OBJECTDIR}/src/application_callbacks/callbacks_can.o
POSSIBLE_DEPFILES=${OBJECTDIR}/openlcb_user_config.o.d ${OBJECTDIR}/traps.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o.d ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o.d ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o.d ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o.d ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o.d ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o.d ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o.d ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o.d ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o.d ${OBJECTDIR}/src/application_callbacks/callbacks_can.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/openlcb_user_config.o ${OBJECTDIR}/traps.o ${OBJECTDIR}/main.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o ${OBJECTDIR}/src/application_callbacks/callbacks_can.o

# Source Files
SOURCEFILES=openlcb_user_config.c traps.c main.c src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.c src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.c src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.c src/openlcb_c_lib/drivers/canbus/can_main_statemachine.c src/openlcb_c_lib/drivers/canbus/can_config.c src/openlcb_c_lib/drivers/canbus/can_login_message_handler.c src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.c src/openlcb_c_lib/drivers/canbus/can_utilities.c src/openlcb_c_lib/drivers/canbus/alias_mappings.c src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.c src/openlcb_c_lib/drivers/canbus/can_login_statemachine.c src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.c src/openlcb_c_lib/drivers/canbus/can_buffer_store.c src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c src/openlcb_c_lib/openlcb/openlcb_application_train.c src/openlcb_c_lib/openlcb/protocol_train_handler.c src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c src/openlcb_c_lib/openlcb/protocol_train_search_handler.c src/openlcb_c_lib/openlcb/openlcb_node.c src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c src/openlcb_c_lib/openlcb/openlcb_buffer_store.c src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c src/openlcb_c_lib/openlcb/openlcb_utilities.c src/openlcb_c_lib/openlcb/openlcb_float16.c src/openlcb_c_lib/openlcb/protocol_event_transport.c src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c src/openlcb_c_lib/openlcb/openlcb_buffer_list.c src/openlcb_c_lib/openlcb/protocol_snip.c src/openlcb_c_lib/openlcb/protocol_datagram_handler.c src/openlcb_c_lib/openlcb/openlcb_gridconnect.c src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c src/openlcb_c_lib/openlcb/openlcb_config.c src/openlcb_c_lib/openlcb/protocol_message_network.c src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c src/openlcb_c_lib/openlcb/openlcb_application.c src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c src/openlcb_c_lib/utilities/mustangpeak_endian_helper.c src/openlcb_c_lib/utilities/mustangpeak_string_helper.c src/application_drivers/dspic33_can_drivers.c src/application_drivers/dspic33_drivers.c src/application_callbacks/callbacks_config_mem.c src/application_callbacks/callbacks_olcb.c src/application_callbacks/callbacks_can.c



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
${OBJECTDIR}/openlcb_user_config.o: openlcb_user_config.c  .generated_files/flags/default/4d7d6dcd118eb62ba396669b1400515028931f2f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/openlcb_user_config.o.d 
	@${RM} ${OBJECTDIR}/openlcb_user_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  openlcb_user_config.c  -o ${OBJECTDIR}/openlcb_user_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/openlcb_user_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/b9e6ac534db2a44e98d12e9bd9454608f4b0c354 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/b79b9c221f129133c16f93868c792b1f76cecb45 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o: src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.c  .generated_files/flags/default/bf8dad40bc98ac7125bb4899721cda07fdb0682b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o: src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.c  .generated_files/flags/default/e74fa725e132c1b8f7f4c747b1581bf23654cb65 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o: src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.c  .generated_files/flags/default/40f368164e5e94ad5c67b1e99f211879aed07a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_main_statemachine.c  .generated_files/flags/default/1617e7730e828984bef6c2f9790dd1165ba0c978 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o: src/openlcb_c_lib/drivers/canbus/can_config.c  .generated_files/flags/default/2c3b2fc3b6f61cea55f7ab598da9c529a9b6f431 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_config.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o: src/openlcb_c_lib/drivers/canbus/can_login_message_handler.c  .generated_files/flags/default/be0f97fa4923ccbe523dd39e0b234b859dfac8b1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_login_message_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o: src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.c  .generated_files/flags/default/645b03d76fed468332d2bc47350c0c606ce1e987 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o: src/openlcb_c_lib/drivers/canbus/can_utilities.c  .generated_files/flags/default/863816486d0e30ef1220e297bf3d078c77dea122 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_utilities.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o: src/openlcb_c_lib/drivers/canbus/alias_mappings.c  .generated_files/flags/default/8530968804e3ddb0527903fed629e8f47db3753c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/alias_mappings.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.c  .generated_files/flags/default/9690b604770740a245b310496eec41177c51fcfe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_login_statemachine.c  .generated_files/flags/default/5ae7b234623d33b92a39f191cf993de8d74e8122 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_login_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.c  .generated_files/flags/default/6d9e270c52e0f53e9fb7fb4397eb5e3e4b782959 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o: src/openlcb_c_lib/drivers/canbus/can_buffer_store.c  .generated_files/flags/default/c4a70e815c094eebb31b397673302a77e0867925 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_buffer_store.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o: src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/36aa623523c39f0b0fc7dc4467bc6b873d85f898 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o: src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c  .generated_files/flags/default/d18832c2a8bd360fa3b2a6b1662468b307c1b778 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o: src/openlcb_c_lib/openlcb/openlcb_application_train.c  .generated_files/flags/default/10ffe09bd7eae61e5df8a635dc4b6e2f183523a0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_application_train.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o: src/openlcb_c_lib/openlcb/protocol_train_handler.c  .generated_files/flags/default/95c90209b37da8f2c2a04624aeb131cda39193cf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_train_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o: src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c  .generated_files/flags/default/17485915091c9567dddab327ace769c4db2f6cf7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o: src/openlcb_c_lib/openlcb/protocol_train_search_handler.c  .generated_files/flags/default/39591cb82cc3597ff7f5b62d5403cce773904054 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_train_search_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o: src/openlcb_c_lib/openlcb/openlcb_node.c  .generated_files/flags/default/3e83d0e7515b83ea14a35ad8a26a577b02d5e5cf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_node.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o: src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c  .generated_files/flags/default/a81d1f3d4c19a4d4fccb6a4e256b036df87e860f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o: src/openlcb_c_lib/openlcb/openlcb_buffer_store.c  .generated_files/flags/default/4aa6ac950c16ed7700adffe07da9e1bce1e6a7a4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o: src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c  .generated_files/flags/default/ee247f503bfda68f7e1e2acbcc59d6d4fcfcebae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o: src/openlcb_c_lib/openlcb/openlcb_utilities.c  .generated_files/flags/default/4bf1376572104f7c42ae80cbe6dc365ae38a7f06 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o: src/openlcb_c_lib/openlcb/openlcb_float16.c  .generated_files/flags/default/39525bc39b33505b555de747581a39f13e9f6c43 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_float16.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o: src/openlcb_c_lib/openlcb/protocol_event_transport.c  .generated_files/flags/default/f70ad04a8c5dcd71caf6068efea8a71f0dfbf49c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o: src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c  .generated_files/flags/default/1d76c889da2933b7d66bccad788238490eadbb14 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o: src/openlcb_c_lib/openlcb/openlcb_buffer_list.c  .generated_files/flags/default/7892ee5255dac9fe8ffeed21f0849302da727a62 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o: src/openlcb_c_lib/openlcb/protocol_snip.c  .generated_files/flags/default/3aad72a4c2bf0f97293c6602f1b8bfdfd39442ec .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_snip.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o: src/openlcb_c_lib/openlcb/protocol_datagram_handler.c  .generated_files/flags/default/6d8cf59ee24d6eb982622a7c380114afb181ac01 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o: src/openlcb_c_lib/openlcb/openlcb_gridconnect.c  .generated_files/flags/default/f7142adb20dd955a9871cc9c1e67d0d5b0151a51 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o: src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/f5e078b2891e06561f51ebb2d3adc10f30367d2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o: src/openlcb_c_lib/openlcb/openlcb_config.c  .generated_files/flags/default/e37d223d396928625977a5f8915454f3a68fdeb9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_config.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o: src/openlcb_c_lib/openlcb/protocol_message_network.c  .generated_files/flags/default/70081f8fd58455e40b6c0b0221ba2cbef0c5f5e1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_message_network.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o: src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c  .generated_files/flags/default/6d75eccc1f0730bb8d48af3964096d6e645404c5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o: src/openlcb_c_lib/openlcb/openlcb_application.c  .generated_files/flags/default/7aaf715a09c8c9089613138d777decca7e7e16bb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_application.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o: src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c  .generated_files/flags/default/c6156087ce88f71d3264f7511a67892f4d8accc5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o: src/openlcb_c_lib/utilities/mustangpeak_endian_helper.c  .generated_files/flags/default/c9675ce304ebd23b9e46976a526abc3318b51196 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/utilities" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/utilities/mustangpeak_endian_helper.c  -o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o: src/openlcb_c_lib/utilities/mustangpeak_string_helper.c  .generated_files/flags/default/41f38e59d825352c4f67e2ac4cab903d99f6c9b5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/utilities" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/utilities/mustangpeak_string_helper.c  -o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o: src/application_drivers/dspic33_can_drivers.c  .generated_files/flags/default/ad533b5047b9b42e531702b1d33ab273ee6fb1ee .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_drivers" 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o.d 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_drivers/dspic33_can_drivers.c  -o ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_drivers/dspic33_drivers.o: src/application_drivers/dspic33_drivers.c  .generated_files/flags/default/bd23667ff6ab8d88ea3a8702f29852914ed9471e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_drivers" 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o.d 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_drivers/dspic33_drivers.c  -o ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_drivers/dspic33_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o: src/application_callbacks/callbacks_config_mem.c  .generated_files/flags/default/18eb6d4e2066692eb20b4e4c948f9e614e650be9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_callbacks" 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o.d 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_callbacks/callbacks_config_mem.c  -o ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o: src/application_callbacks/callbacks_olcb.c  .generated_files/flags/default/a3bdbcb499be6ed4b4ecca54bf1fe5d13f3aed5b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_callbacks" 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o.d 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_callbacks/callbacks_olcb.c  -o ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_callbacks/callbacks_can.o: src/application_callbacks/callbacks_can.c  .generated_files/flags/default/e9b2977aac36a1223f2108248cb32473125c4b90 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_callbacks" 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_can.o.d 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_callbacks/callbacks_can.c  -o ${OBJECTDIR}/src/application_callbacks/callbacks_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_callbacks/callbacks_can.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/openlcb_user_config.o: openlcb_user_config.c  .generated_files/flags/default/4ac8eeee3e7082e3850ef62773046303bc7870fa .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/openlcb_user_config.o.d 
	@${RM} ${OBJECTDIR}/openlcb_user_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  openlcb_user_config.c  -o ${OBJECTDIR}/openlcb_user_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/openlcb_user_config.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/87b1f2aa15dffc285e182eabc9de768bb4cf3f5c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/51c37d452863c14a441d1840d464bfdefb4c1f5f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o: src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.c  .generated_files/flags/default/581ebe698264f69a764bab1d136467bff88515b8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o: src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.c  .generated_files/flags/default/4224c4f64deb5e7eb4e59d252686ca83fd506e82 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o: src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.c  .generated_files/flags/default/67e7d383e8500b3b6ebd8e7a1f400c0a39a735b9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mapping_listener.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_main_statemachine.c  .generated_files/flags/default/3300d30333597540660a3c3dafbf58d365244e52 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o: src/openlcb_c_lib/drivers/canbus/can_config.c  .generated_files/flags/default/5d2ac53d7fc9cd0308c8f768570bcfdbae6660c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_config.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_config.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o: src/openlcb_c_lib/drivers/canbus/can_login_message_handler.c  .generated_files/flags/default/e491172dd60f4729f2e153089fb55c7b3464fd9a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_login_message_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o: src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.c  .generated_files/flags/default/a7b1cc4636b26c26767714376cfea0c21b516366 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_message_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o: src/openlcb_c_lib/drivers/canbus/can_utilities.c  .generated_files/flags/default/f9124fc7eb508f45ac06a3cdf6e97eb8f507f692 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_utilities.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o: src/openlcb_c_lib/drivers/canbus/alias_mappings.c  .generated_files/flags/default/5e2e27971f13e0e9a8eb72aa7443e23c0c076272 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/alias_mappings.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/alias_mappings.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.c  .generated_files/flags/default/6f99b176308714120d9f9d7ffe5dfb80e00a9372 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_login_statemachine.c  .generated_files/flags/default/99010eea9ad3497d075680dcf960c5897d9f7e2a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_login_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o: src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.c  .generated_files/flags/default/39a00a91ece273e3df779ef479ccbc4bda0c636e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_tx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o: src/openlcb_c_lib/drivers/canbus/can_buffer_store.c  .generated_files/flags/default/8199c1ce0e76f5c20e74086509aea01818868d9f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/drivers/canbus/can_buffer_store.c  -o ${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/drivers/canbus/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o: src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/6ea1bc763232dd6408f60c1a43dfd6fd87d4da71 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o: src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c  .generated_files/flags/default/ccc58b809fd7e3e4a33ab8dccb3c64d251045603 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_write_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o: src/openlcb_c_lib/openlcb/openlcb_application_train.c  .generated_files/flags/default/e35d739b6c8c0e43b6fe9e7c173055c8767491d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_application_train.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_train.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o: src/openlcb_c_lib/openlcb/protocol_train_handler.c  .generated_files/flags/default/506a71cd123742459a471098e605974e73251b25 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_train_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o: src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c  .generated_files/flags/default/a4e62ffeb636e77840c3518db4257612dffe7c27 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_operations_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o: src/openlcb_c_lib/openlcb/protocol_train_search_handler.c  .generated_files/flags/default/e936ea722d4a2ffa6e8bae779b039f07cb079ba7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_train_search_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_train_search_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o: src/openlcb_c_lib/openlcb/openlcb_node.c  .generated_files/flags/default/4b8effc9b58ee2efc6443424264ae2b19baa9b35 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_node.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_node.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o: src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c  .generated_files/flags/default/b6873b00467e5827165bda664bc2b846a2a2d8c4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_broadcast_time_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o: src/openlcb_c_lib/openlcb/openlcb_buffer_store.c  .generated_files/flags/default/2597b9c72a0ed6450bec60ce6daa21dfe10a1ca4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o: src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c  .generated_files/flags/default/efa293bef3fb953056373441e46e172e6615244c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_login_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o: src/openlcb_c_lib/openlcb/openlcb_utilities.c  .generated_files/flags/default/ac13789b55aa2d304d32eaf6450d1fdba91b35d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o: src/openlcb_c_lib/openlcb/openlcb_float16.c  .generated_files/flags/default/207748799d4cd333f4d5b9b0b5d79d85149d931f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_float16.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_float16.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o: src/openlcb_c_lib/openlcb/protocol_event_transport.c  .generated_files/flags/default/3aea64ca1720631c31e228c1a63ca1adf9eb0ce5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_event_transport.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_event_transport.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o: src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c  .generated_files/flags/default/5999a07a2eeede580a76bb0d43ec3308b1750dae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_config_mem_read_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o: src/openlcb_c_lib/openlcb/openlcb_buffer_list.c  .generated_files/flags/default/19dbc26a3c50f7e653c26d743e9141837667d84a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o: src/openlcb_c_lib/openlcb/protocol_snip.c  .generated_files/flags/default/c09828e10f1ee61f8556b007781c97f7f6db304c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_snip.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_snip.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o: src/openlcb_c_lib/openlcb/protocol_datagram_handler.c  .generated_files/flags/default/6a3777818bcaa62d4863e693501a4aae187f2943 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_datagram_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_datagram_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o: src/openlcb_c_lib/openlcb/openlcb_gridconnect.c  .generated_files/flags/default/6c27b8c9f426e1a490543c46de9aab58f1011688 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_gridconnect.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_gridconnect.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o: src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c  .generated_files/flags/default/65198be8322564713e0f33eea6071104cd8a91bb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_main_statemachine.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_main_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o: src/openlcb_c_lib/openlcb/openlcb_config.c  .generated_files/flags/default/71725b3c7730be816d8c92c014e7cb9d388d2c86 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_config.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_config.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o: src/openlcb_c_lib/openlcb/protocol_message_network.c  .generated_files/flags/default/d1ebfd51112fa38f3683d07c2af5459b56dbdeb2 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/protocol_message_network.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/protocol_message_network.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o: src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c  .generated_files/flags/default/faae69eabd5b1363feea36f4ee0305139c852d0c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application_broadcast_time.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o: src/openlcb_c_lib/openlcb/openlcb_application.c  .generated_files/flags/default/baa96440797a8b1bb978b9449011b9d03e9f6670 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_application.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_application.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o: src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c  .generated_files/flags/default/553ea3f8302720b6f7f58eddf3fb32e5725ef76e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/openlcb" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.c  -o ${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/openlcb/openlcb_login_statemachine_handler.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o: src/openlcb_c_lib/utilities/mustangpeak_endian_helper.c  .generated_files/flags/default/c88c43a1dcf6c54c81fe7990d4e3e0a3fce9ca6b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/utilities" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/utilities/mustangpeak_endian_helper.c  -o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_endian_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o: src/openlcb_c_lib/utilities/mustangpeak_string_helper.c  .generated_files/flags/default/cdb20a880bf9f501ea04eb96c0a7bb3a3ce7823b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/openlcb_c_lib/utilities" 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o.d 
	@${RM} ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/openlcb_c_lib/utilities/mustangpeak_string_helper.c  -o ${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/openlcb_c_lib/utilities/mustangpeak_string_helper.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o: src/application_drivers/dspic33_can_drivers.c  .generated_files/flags/default/c60a1458a6ec268c242e4c640e1d89f70e304d02 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_drivers" 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o.d 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_drivers/dspic33_can_drivers.c  -o ${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_drivers/dspic33_can_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_drivers/dspic33_drivers.o: src/application_drivers/dspic33_drivers.c  .generated_files/flags/default/87afe7ada282737634aa5584b3560d232826b95b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_drivers" 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o.d 
	@${RM} ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_drivers/dspic33_drivers.c  -o ${OBJECTDIR}/src/application_drivers/dspic33_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_drivers/dspic33_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o: src/application_callbacks/callbacks_config_mem.c  .generated_files/flags/default/9cc8699ef6d30c2fefcdcf16bd465859fcfbd340 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_callbacks" 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o.d 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_callbacks/callbacks_config_mem.c  -o ${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_callbacks/callbacks_config_mem.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o: src/application_callbacks/callbacks_olcb.c  .generated_files/flags/default/1fb9decc760801d42a2dcc411246c09e5f2416ce .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_callbacks" 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o.d 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_callbacks/callbacks_olcb.c  -o ${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_callbacks/callbacks_olcb.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/src/application_callbacks/callbacks_can.o: src/application_callbacks/callbacks_can.c  .generated_files/flags/default/53ef7b7635761a60b69d7ab6a359c075a9d37a5d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/src/application_callbacks" 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_can.o.d 
	@${RM} ${OBJECTDIR}/src/application_callbacks/callbacks_can.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  src/application_callbacks/callbacks_can.c  -o ${OBJECTDIR}/src/application_callbacks/callbacks_can.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/src/application_callbacks/callbacks_can.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -mlarge-code -O0 -I"." -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/BasicNode.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000 -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
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
