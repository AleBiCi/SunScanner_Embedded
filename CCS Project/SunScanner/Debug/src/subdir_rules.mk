################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs1260/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/Users/alessandrobianchiceriani/Desktop/Ale Varie DONTTOUCH/UNITN/Terzo Anno/Primo Semestre/Embedded Software/CCS Workspaces/SunScanner/libs" --include_path="/Users/alessandrobianchiceriani/Desktop/Ale Varie DONTTOUCH/UNITN/Terzo Anno/Primo Semestre/Embedded Software/CCS Workspaces/SunScanner" --include_path="/Users/alessandrobianchiceriani/Desktop/Ale Varie DONTTOUCH/UNITN/Terzo Anno/Primo Semestre/Embedded Software/simplelink_msp432p4_sdk_3_40_01_02/source" --include_path="/Users/alessandrobianchiceriani/Desktop/Ale Varie DONTTOUCH/UNITN/Terzo Anno/Primo Semestre/Embedded Software/simplelink_msp432p4_sdk_3_40_01_02/source/third_party/CMSIS/Include" --include_path="/Applications/ti/ccs1260/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=__MSP432P401R__ --define=DeviceFamily_MSP432P401x -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


