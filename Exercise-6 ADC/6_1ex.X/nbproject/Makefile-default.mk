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
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=6_1main.c ../../my-libraries/spi.c ../../my-libraries/timer_config.c ../../my-libraries/uart.c ../../my-libraries/adc.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/6_1main.o ${OBJECTDIR}/_ext/2014297768/spi.o ${OBJECTDIR}/_ext/2014297768/timer_config.o ${OBJECTDIR}/_ext/2014297768/uart.o ${OBJECTDIR}/_ext/2014297768/adc.o
POSSIBLE_DEPFILES=${OBJECTDIR}/6_1main.o.d ${OBJECTDIR}/_ext/2014297768/spi.o.d ${OBJECTDIR}/_ext/2014297768/timer_config.o.d ${OBJECTDIR}/_ext/2014297768/uart.o.d ${OBJECTDIR}/_ext/2014297768/adc.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/6_1main.o ${OBJECTDIR}/_ext/2014297768/spi.o ${OBJECTDIR}/_ext/2014297768/timer_config.o ${OBJECTDIR}/_ext/2014297768/uart.o ${OBJECTDIR}/_ext/2014297768/adc.o

# Source Files
SOURCEFILES=6_1main.c ../../my-libraries/spi.c ../../my-libraries/timer_config.c ../../my-libraries/uart.c ../../my-libraries/adc.c



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
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33EP512MU810
MP_LINKER_FILE_OPTION=,--script=p33EP512MU810.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/6_1main.o: 6_1main.c  .generated_files/flags/default/a94a930fadd02317ae90b916787fe7dd7085ac53 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/6_1main.o.d 
	@${RM} ${OBJECTDIR}/6_1main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  6_1main.c  -o ${OBJECTDIR}/6_1main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/6_1main.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/spi.o: ../../my-libraries/spi.c  .generated_files/flags/default/36d6c7e63210f2f7d147d4556bfcd8398cd9823b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/spi.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/spi.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/spi.c  -o ${OBJECTDIR}/_ext/2014297768/spi.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/spi.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/timer_config.o: ../../my-libraries/timer_config.c  .generated_files/flags/default/54a4d5cbe1fe40c4ff2e0fe22e229261f72e92d4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/timer_config.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/timer_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/timer_config.c  -o ${OBJECTDIR}/_ext/2014297768/timer_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/timer_config.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/uart.o: ../../my-libraries/uart.c  .generated_files/flags/default/1d05293672534ba8dcbb7416bf3ea4764c76b4a4 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/uart.c  -o ${OBJECTDIR}/_ext/2014297768/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/uart.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/adc.o: ../../my-libraries/adc.c  .generated_files/flags/default/39105da5e74a626e76093960ae691d8a5daeec12 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/adc.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/adc.c  -o ${OBJECTDIR}/_ext/2014297768/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/adc.o.d"      -g -D__DEBUG   -mno-eds-warn  -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
else
${OBJECTDIR}/6_1main.o: 6_1main.c  .generated_files/flags/default/ca647d55699070e79af2ec96e3a398295ebb5578 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/6_1main.o.d 
	@${RM} ${OBJECTDIR}/6_1main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  6_1main.c  -o ${OBJECTDIR}/6_1main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/6_1main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/spi.o: ../../my-libraries/spi.c  .generated_files/flags/default/8a9af8bf3aa4f188f7cf23c160bd351c295844bf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/spi.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/spi.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/spi.c  -o ${OBJECTDIR}/_ext/2014297768/spi.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/spi.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/timer_config.o: ../../my-libraries/timer_config.c  .generated_files/flags/default/7c857af3346fa25e0e294b0fa2906e7a0f2d082e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/timer_config.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/timer_config.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/timer_config.c  -o ${OBJECTDIR}/_ext/2014297768/timer_config.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/timer_config.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/uart.o: ../../my-libraries/uart.c  .generated_files/flags/default/5d40fe5f741eb0b3e1f4dc0c14708cf4636683cf .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/uart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/uart.c  -o ${OBJECTDIR}/_ext/2014297768/uart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/uart.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
${OBJECTDIR}/_ext/2014297768/adc.o: ../../my-libraries/adc.c  .generated_files/flags/default/22a902eaecb1e275a4a4587bc899a0a9ba62fd56 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/2014297768" 
	@${RM} ${OBJECTDIR}/_ext/2014297768/adc.o.d 
	@${RM} ${OBJECTDIR}/_ext/2014297768/adc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../my-libraries/adc.c  -o ${OBJECTDIR}/_ext/2014297768/adc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/2014297768/adc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -O0 -msmart-io=1 -Wall -msfr-warn=off    -mdfp="${DFP_DIR}/xc16"
	
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
${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG   -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries"     -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	
else
${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -I"../../my-libraries" -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST)  -mdfp="${DFP_DIR}/xc16" 
	${MP_CC_DIR}\\xc16-bin2hex ${DISTDIR}/6_1ex.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf   -mdfp="${DFP_DIR}/xc16" 
	
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
