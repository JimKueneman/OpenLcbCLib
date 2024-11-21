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
FINAL_IMAGE=${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=traps.c debug.c main.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/mcu_driver.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/585199093/mcu_driver.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o
POSSIBLE_DEPFILES=${OBJECTDIR}/traps.o.d ${OBJECTDIR}/debug.o.d ${OBJECTDIR}/main.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d ${OBJECTDIR}/_ext/216671855/can_utilities.o.d ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d ${OBJECTDIR}/_ext/585199093/mcu_driver.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/traps.o ${OBJECTDIR}/debug.o ${OBJECTDIR}/main.o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o ${OBJECTDIR}/_ext/216671855/can_utilities.o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o ${OBJECTDIR}/_ext/585199093/mcu_driver.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o

# Source Files
SOURCEFILES=traps.c debug.c main.c ../../../../drivers/common/can_buffer_fifo.c ../../../../drivers/common/can_utilities.c ../../../../drivers/common/can_buffer_store.c ../../../../drivers/mcu_driver.c ../../../../openlcb/openlcb_buffer_fifo.c ../../../../openlcb/openlcb_buffer_store.c ../../../../openlcb/openlcb_utilities.c ../../../../openlcb/openlcb_buffer_list.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP64GP502
MP_LINKER_FILE_OPTION=,--script=p33EP64GP502.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/48f01dfcc47a3662a76e8feb2aeeb49a2e10b783 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/8237daecfc93ad6ab7d3949f95531537335852dc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/e7f9e7ac7987f67d77483a60fe20338436b76038 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/ec975af572a391325e18bfe0b3a159133628fbe5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/9c2bf6a4887666487a148998b754696b89915103 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/e5effc87cffe77ca900c45bf5551d4166f5ec731 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/mcu_driver.o: ../../../../drivers/mcu_driver.c  .generated_files/flags/default/4d20e52dffaa7915e3747c411dce6eb63f2ab4b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/mcu_driver.c  -o ${OBJECTDIR}/_ext/585199093/mcu_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/mcu_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/c88f831965e47f1649b98041970dde5f2c243d44 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/d17ce62602ab417a431bae92dea31ff829631001 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/3a03cd2d49ea859d3537f593ade0f29c9781ac1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/7f543834a1e45b31dcf485a569eaf061f27f139b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_list.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/traps.o: traps.c  .generated_files/flags/default/a69459103af4fc15de85b5cc20ce0738aa750d8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/traps.o.d 
	@${RM} ${OBJECTDIR}/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  traps.c  -o ${OBJECTDIR}/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/traps.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/debug.o: debug.c  .generated_files/flags/default/f778e44ff5ca22c6ee6cfc109d7022ec025d11ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/debug.o.d 
	@${RM} ${OBJECTDIR}/debug.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  debug.c  -o ${OBJECTDIR}/debug.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/debug.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/79ae070c355bf88981046a03d5524ed1bd346bfc .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o: ../../../../drivers/common/can_buffer_fifo.c  .generated_files/flags/default/7f972c91788684e8e488991dda5f29f408c7e41e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_fifo.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_utilities.o: ../../../../drivers/common/can_utilities.c  .generated_files/flags/default/49078bc7c0372161c7ec40ecd5aa83ca9b80033f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_utilities.c  -o ${OBJECTDIR}/_ext/216671855/can_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/216671855/can_buffer_store.o: ../../../../drivers/common/can_buffer_store.c  .generated_files/flags/default/cf40e1f8e4fb436b823cae45b969689f1fefaf3b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/216671855" 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/216671855/can_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/common/can_buffer_store.c  -o ${OBJECTDIR}/_ext/216671855/can_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/216671855/can_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/585199093/mcu_driver.o: ../../../../drivers/mcu_driver.c  .generated_files/flags/default/419d20c50f101aefcd7ac430bdf8967842ce91f7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/585199093" 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o.d 
	@${RM} ${OBJECTDIR}/_ext/585199093/mcu_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../drivers/mcu_driver.c  -o ${OBJECTDIR}/_ext/585199093/mcu_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/585199093/mcu_driver.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o: ../../../../openlcb/openlcb_buffer_fifo.c  .generated_files/flags/default/73856091971e350d1b3d75a575b49bf6ea702c1d .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_fifo.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o: ../../../../openlcb/openlcb_buffer_store.c  .generated_files/flags/default/1c3aa3d12c94af0ca692243c0d72f78772223b5c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_buffer_store.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_buffer_store.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_utilities.o: ../../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/f9aaeafe667ec25b3216dab58e85447a5f4bb562 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/526222337" 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/526222337/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/526222337/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/526222337/openlcb_buffer_list.o: ../../../../openlcb/openlcb_buffer_list.c  .generated_files/flags/default/eb71be48d23b8b897fe3091196499d2598c18c2e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
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
${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/can_utilities.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
