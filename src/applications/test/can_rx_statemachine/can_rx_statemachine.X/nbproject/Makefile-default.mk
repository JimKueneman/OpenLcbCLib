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
FINAL_IMAGE=${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=traps.c debug.c main.c openlcb_node_definition.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_rx_statemachine.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/mcu_driver.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/openlcb_node_definition.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/585199093/mcu_driver.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o
POSSIBLE_DEPFILES=${OBJECTDIR}/traps.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/openlcb_node_definition.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/216671855/can_utilities.o.d ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d ${OBJECTDIR}/_ext/585199093/mcu_driver.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/openlcb_node_definition.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/585199093/mcu_driver.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o

# Source Files
SOURCEFILES=traps.c debug.c main.c openlcb_node_definition.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_rx_statemachine.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/mcu_driver.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP64GP502
MP_LINKER_FILE_OPTION=,--script=p33EP64GP502.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/5c94ecca3da26b8e8f5cc5eab25fbe6af08600bf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/892b303a92a9fd681b7397aec67907d551244822 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/96ff0f7f153899125503c2d96e9c4da155c368d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/openlcb_node_definition.o: openlcb_node_definition.c  .generated_files/flags/default/b00cdcff50714b3f8300120e9aabfdc5fd4e43d3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/openlcb_node_definition.o.d 
	@${RM} ${OBJECTDIR}/openlcb_node_definition.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  openlcb_node_definition.c  -o ${OBJECTDIR}/openlcb_node_definition.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/openlcb_node_definition.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/8e7b94565655bc1b7ef22dfd79b1144cb202121a .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/68c4397b8e090fbaa200c5a1f26ff536d9d6e0b7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/1c1c7f0c0ae7ecf5460d8d9eec2c7b99ff3da317 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/f2539cf4d427ae1701eb8911f4de4fa0ce92737f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/mcu_driver.o: ../../../../drivers/mcu_driver.c  .generated_files/flags/default/6bb60886668ac4415ac97bfcc2f58598f894b644 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/mcu_driver.c  -o ${OBJECTDIR}/_ext/585199093/mcu_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/mcu_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/7ad29dc351bcce34fbaa3b53b1ff07fcdecd11e6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/ceb7fd3b4b05a9b748bda2c1fe16be7d18e98ee9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/a7fbf8c1ce8dce7c870b938f709b592322dd2801 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/d56e317acb0fd2c940b832e3d3edb5ca45b4ed7e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/2c74a9e29db1de957b29fa9c378c8567ff4f05bf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/9d844f011b73b5408c031d296d8756cdc76dceae .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/35a1d8820e7668c58de069fa2acb01aa0c14da6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/openlcb_node_definition.o: openlcb_node_definition.c  .generated_files/flags/default/4ab17e3ef420aa61c90812ee873eef2751adb658 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/openlcb_node_definition.o.d 
	@${RM} ${OBJECTDIR}/openlcb_node_definition.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  openlcb_node_definition.c  -o ${OBJECTDIR}/openlcb_node_definition.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/openlcb_node_definition.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/2d28b92f91676711b98a22a4321312e119b323e4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/2e58c075caa022a1e8e459f0d2d145949e9ae171 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o: ../../../../drivers/common/can_rx_statemachine.c  .generated_files/flags/default/aac62ca95ad5e54b1e30b42a46766ade91446198 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_rx_statemachine.c  -o ${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_rx_statemachine.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/756fda12db612bb3e5e391eb98194d6142041572 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/mcu_driver.o: ../../../../drivers/mcu_driver.c  .generated_files/flags/default/aee63b285b6c5825dfc41b6af9560a2fcb1c8ba9 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/mcu_driver.c  -o ${OBJECTDIR}/_ext/585199093/mcu_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/mcu_driver.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/73c8aa50bb83028e1b55cca0eaf86041026a58ff .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/549703720611c00e273775f7a920b5ea0339a2ef .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/d7e67d82d8a28f997c16a80abac7aedc6170111f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/a8a3a8a996eea2b001bd78c7711c513e003ffe92 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/can_rx_statemachine.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
