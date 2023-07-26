################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/Ant.c" \
"../Sources/BLE.c" \
"../Sources/Events.c" \
"../Sources/IRSensor.c" \
"../Sources/NCP_Helpers.c" \
"../Sources/Operation.c" \
"../Sources/PowerSupply.c" \
"../Sources/SystemInit.c" \
"../Sources/Timing.c" \
"../Sources/TouchSensor.c" \
"../Sources/UART.c" \
"../Sources/boot.c" \
"../Sources/main.c" \
"../Sources/sa_mtb.c" \

C_SRCS += \
../Sources/Ant.c \
../Sources/BLE.c \
../Sources/Events.c \
../Sources/IRSensor.c \
../Sources/NCP_Helpers.c \
../Sources/Operation.c \
../Sources/PowerSupply.c \
../Sources/SystemInit.c \
../Sources/Timing.c \
../Sources/TouchSensor.c \
../Sources/UART.c \
../Sources/boot.c \
../Sources/main.c \
../Sources/sa_mtb.c \

OBJS += \
./Sources/Ant.o \
./Sources/BLE.o \
./Sources/Events.o \
./Sources/IRSensor.o \
./Sources/NCP_Helpers.o \
./Sources/Operation.o \
./Sources/PowerSupply.o \
./Sources/SystemInit.o \
./Sources/Timing.o \
./Sources/TouchSensor.o \
./Sources/UART.o \
./Sources/boot.o \
./Sources/main.o \
./Sources/sa_mtb.o \

OBJS_QUOTED += \
"./Sources/Ant.o" \
"./Sources/BLE.o" \
"./Sources/Events.o" \
"./Sources/IRSensor.o" \
"./Sources/NCP_Helpers.o" \
"./Sources/Operation.o" \
"./Sources/PowerSupply.o" \
"./Sources/SystemInit.o" \
"./Sources/Timing.o" \
"./Sources/TouchSensor.o" \
"./Sources/UART.o" \
"./Sources/boot.o" \
"./Sources/main.o" \
"./Sources/sa_mtb.o" \

C_DEPS += \
./Sources/Ant.d \
./Sources/BLE.d \
./Sources/Events.d \
./Sources/IRSensor.d \
./Sources/NCP_Helpers.d \
./Sources/Operation.d \
./Sources/PowerSupply.d \
./Sources/SystemInit.d \
./Sources/Timing.d \
./Sources/TouchSensor.d \
./Sources/UART.d \
./Sources/boot.d \
./Sources/main.d \
./Sources/sa_mtb.d \

OBJS_OS_FORMAT += \
./Sources/Ant.o \
./Sources/BLE.o \
./Sources/Events.o \
./Sources/IRSensor.o \
./Sources/NCP_Helpers.o \
./Sources/Operation.o \
./Sources/PowerSupply.o \
./Sources/SystemInit.o \
./Sources/Timing.o \
./Sources/TouchSensor.o \
./Sources/UART.o \
./Sources/boot.o \
./Sources/main.o \
./Sources/sa_mtb.o \

C_DEPS_QUOTED += \
"./Sources/Ant.d" \
"./Sources/BLE.d" \
"./Sources/Events.d" \
"./Sources/IRSensor.d" \
"./Sources/NCP_Helpers.d" \
"./Sources/Operation.d" \
"./Sources/PowerSupply.d" \
"./Sources/SystemInit.d" \
"./Sources/Timing.d" \
"./Sources/TouchSensor.d" \
"./Sources/UART.d" \
"./Sources/boot.d" \
"./Sources/main.d" \
"./Sources/sa_mtb.d" \


# Each subdirectory must supply rules for building sources it contributes
Sources/Ant.o: ../Sources/Ant.c
	@echo 'Building file: $<'
	@echo 'Executing target #9 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/Ant.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/Ant.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/BLE.o: ../Sources/BLE.c
	@echo 'Building file: $<'
	@echo 'Executing target #10 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/BLE.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/BLE.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/Events.o: ../Sources/Events.c
	@echo 'Building file: $<'
	@echo 'Executing target #11 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/Events.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/Events.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/IRSensor.o: ../Sources/IRSensor.c
	@echo 'Building file: $<'
	@echo 'Executing target #12 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/IRSensor.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/IRSensor.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/NCP_Helpers.o: ../Sources/NCP_Helpers.c
	@echo 'Building file: $<'
	@echo 'Executing target #13 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/NCP_Helpers.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/NCP_Helpers.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/Operation.o: ../Sources/Operation.c
	@echo 'Building file: $<'
	@echo 'Executing target #14 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/Operation.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/Operation.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/PowerSupply.o: ../Sources/PowerSupply.c
	@echo 'Building file: $<'
	@echo 'Executing target #15 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/PowerSupply.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/PowerSupply.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/SystemInit.o: ../Sources/SystemInit.c
	@echo 'Building file: $<'
	@echo 'Executing target #16 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/SystemInit.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/SystemInit.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/Timing.o: ../Sources/Timing.c
	@echo 'Building file: $<'
	@echo 'Executing target #17 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/Timing.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/Timing.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/TouchSensor.o: ../Sources/TouchSensor.c
	@echo 'Building file: $<'
	@echo 'Executing target #18 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/TouchSensor.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/TouchSensor.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/UART.o: ../Sources/UART.c
	@echo 'Building file: $<'
	@echo 'Executing target #19 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/UART.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/UART.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/boot.o: ../Sources/boot.c
	@echo 'Building file: $<'
	@echo 'Executing target #20 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/boot.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/boot.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/main.o: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #21 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/main.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/main.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/sa_mtb.o: ../Sources/sa_mtb.c
	@echo 'Building file: $<'
	@echo 'Executing target #22 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/sa_mtb.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/sa_mtb.o"
	@echo 'Finished building: $<'
	@echo ' '


