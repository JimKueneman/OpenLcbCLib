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
FINAL_IMAGE=${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../../../openlcb/openlcb_utilities.c local_drivers/_25AA1024/25AA1024_driver.c turnoutboss_bootloader_drivers.c main.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o ${OBJECTDIR}/main.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d ${OBJECTDIR}/main.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/156864402/openlcb_utilities.o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o ${OBJECTDIR}/main.o

# Source Files
SOURCEFILES=../../../openlcb/openlcb_utilities.c local_drivers/_25AA1024/25AA1024_driver.c turnoutboss_bootloader_drivers.c main.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512GP504
MP_LINKER_FILE_OPTION=,--script="p33EP512GP504_TurnoutBossBootloader.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/34f6978304e2b918e057bbb74e21dd64f2290eba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/93d143506d4d8542fc20dd85116349e415f3c8d7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_drivers.o: turnoutboss_bootloader_drivers.c  .generated_files/flags/default/83888937d3a28c7827a706c57b015e4dde8bf44f .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_drivers.c  -o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/f4a86e2912b0347977c4c8d9741f708749215cf6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -mno-eds-warn  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/_ext/156864402/openlcb_utilities.o: ../../../openlcb/openlcb_utilities.c  .generated_files/flags/default/5e962389d4ed35277eb1e640935df6eeb7cda8ef .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/156864402" 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../openlcb/openlcb_utilities.c  -o ${OBJECTDIR}/_ext/156864402/openlcb_utilities.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/156864402/openlcb_utilities.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o: local_drivers/_25AA1024/25AA1024_driver.c  .generated_files/flags/default/196b507a8bd1c09aaa2ebe90cb74fcc756a82ba1 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/local_drivers/_25AA1024" 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d 
	@${RM} ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  local_drivers/_25AA1024/25AA1024_driver.c  -o ${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/local_drivers/_25AA1024/25AA1024_driver.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/turnoutboss_bootloader_drivers.o: turnoutboss_bootloader_drivers.c  .generated_files/flags/default/dfcdd20dca880be25368771379446ef8f6f39adf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d 
	@${RM} ${OBJECTDIR}/turnoutboss_bootloader_drivers.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  turnoutboss_bootloader_drivers.c  -o ${OBJECTDIR}/turnoutboss_bootloader_drivers.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/turnoutboss_bootloader_drivers.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/main.o: main.c  .generated_files/flags/default/d55d2f5b6c88606bf962b732257dbcb4258a4d42 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/main.o.d 
	@${RM} ${OBJECTDIR}/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  main.c  -o ${OBJECTDIR}/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/main.o.d"      -mno-eds-warn  -g -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    p33EP512GP504_TurnoutBossBootloader.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_SIMULATOR=1  -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_SIMULATOR=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   p33EP512GP504_TurnoutBossBootloader.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DFCY=40000000UL -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}/xc16-bin2hex ${DISTDIR}/TurnoutBossBootloader.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
