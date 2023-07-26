################################################################################
# Automatically-generated file. Do not edit!
################################################################################

#Progress monitor hint: 69
first : all
-include ../makefile.init

# This file contains definitions of environment variables used in the makefiles and .args files if exist.
-include makefile.local

RM := "$(GNU_Make_Install_DirEnv)/rm" -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include Sources/src/subdir.mk
-include Sources/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include Generated_Code/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
SolisIoT_SKU_31_Ver0111.elf \

EXECUTABLES_QUOTED += \
"SolisIoT_SKU_31_Ver0111.elf" \

EXECUTABLES_OS_FORMAT += \
SolisIoT_SKU_31_Ver0111.elf \

SECONDARY_FLASH += \
SolisIoT_SKU_31_Ver0111.hex \

SECONDARY_FLASH_QUOTED += \
"SolisIoT_SKU_31_Ver0111.hex" \

SECONDARY_FLASH_OS_FORMAT += \
SolisIoT_SKU_31_Ver0111.hex \

SECONDARY_LIST += \
SolisIoT_SKU_31_Ver0111.lst \

SECONDARY_LIST_QUOTED += \
"SolisIoT_SKU_31_Ver0111.lst" \

SECONDARY_LIST_OS_FORMAT += \
SolisIoT_SKU_31_Ver0111.lst \

SECONDARY_SIZE += \
SolisIoT_SKU_31_Ver0111.siz \

SECONDARY_SIZE_QUOTED += \
"SolisIoT_SKU_31_Ver0111.siz" \

SECONDARY_SIZE_OS_FORMAT += \
SolisIoT_SKU_31_Ver0111.siz \


# All Target
all: warning-messages explicit-dependencies SolisIoT_SKU_31_Ver0111.elf secondary-outputs

# Tool invocations
SolisIoT_SKU_31_Ver0111.elf: $(OBJS) $(USER_OBJS) ../Project_Settings/Linker_Files/ProcessorExpert.ld
	@echo 'Building target: $@'
	@echo 'Executing target #70 $@'
	@echo 'Invoking: ARM Ltd Windows GCC C Linker'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc"  $(USER_OBJS_QUOTED) $(LIBS_QUOTED) @"SolisIoT_SKU_31_Ver0111.args" -o"SolisIoT_SKU_31_Ver0111.elf"
	@echo 'Finished building target: $@'
	@echo ' '

SolisIoT_SKU_31_Ver0111.hex: SolisIoT_SKU_31_Ver0111.elf
	@echo 'Executing target #71 $@'
	@echo 'Invoking: ARM Ltd Windows GNU Create Flash Image'
	"$(ARMSourceryDirEnv)/arm-none-eabi-objcopy"  -O srec SolisIoT_SKU_31_Ver0111.elf "SolisIoT_SKU_31_Ver0111.hex"
	@echo 'Finished building: $@'
	@echo ' '

SolisIoT_SKU_31_Ver0111.lst: SolisIoT_SKU_31_Ver0111.elf
	@echo 'Executing target #72 $@'
	@echo 'Invoking: ARM Ltd Windows GNU Create Listing'
	"$(ARMSourceryDirEnv)/arm-none-eabi-objdump"  -h -S SolisIoT_SKU_31_Ver0111.elf >"SolisIoT_SKU_31_Ver0111.lst"
	@echo 'Finished building: $@'
	@echo ' '

SolisIoT_SKU_31_Ver0111.siz: SolisIoT_SKU_31_Ver0111.elf
	@echo 'Executing target #73 $@'
	@echo 'Invoking: ARM Ltd Windows GNU Print Size'
	"$(ARMSourceryDirEnv)/arm-none-eabi-size"  --format=berkeley SolisIoT_SKU_31_Ver0111.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(S_DEPS_QUOTED) $(SECONDARY_LIST_QUOTED) $(SECONDARY_FLASH_QUOTED) "./*/*.o" "./*/*/*.o"  $(S_UPPER_DEPS_QUOTED) $(ASM_DEPS_QUOTED) $(SECONDARY_SIZE_QUOTED) $(ASM_UPPER_DEPS_QUOTED) $(SX_DEPS_QUOTED) $(EXECUTABLES_QUOTED) "./*/*.d" "./*/*/*.d"  $(ProjDirPath)/FLASH/SolisIoT_SKU_31_Ver0111.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents explicit-dependencies warning-messages
.SECONDARY:

-include ../makefile.targets

explicit-dependencies: ../Project_Settings/Linker_Files/ProcessorExpert.ld SolisIoT_SKU_31_Ver0111.elf SolisIoT_SKU_31_Ver0111.elf SolisIoT_SKU_31_Ver0111.elf 

warning-messages: 

