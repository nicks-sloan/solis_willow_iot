################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/src/BlynkRpcCRC8.c" \
"../Sources/src/BlynkRpcClient.c" \
"../Sources/src/BlynkRpcClientWeakImpl.c" \
"../Sources/src/BlynkRpcInfra.c" \
"../Sources/src/BlynkRpcInfraUart.c" \
"../Sources/src/BlynkRpcUartFraming.c" \
"../Sources/src/MessageBuffer.c" \
"../Sources/src/MessageWriter.c" \

C_SRCS += \
../Sources/src/BlynkRpcCRC8.c \
../Sources/src/BlynkRpcClient.c \
../Sources/src/BlynkRpcClientWeakImpl.c \
../Sources/src/BlynkRpcInfra.c \
../Sources/src/BlynkRpcInfraUart.c \
../Sources/src/BlynkRpcUartFraming.c \
../Sources/src/MessageBuffer.c \
../Sources/src/MessageWriter.c \

OBJS += \
./Sources/src/BlynkRpcCRC8.o \
./Sources/src/BlynkRpcClient.o \
./Sources/src/BlynkRpcClientWeakImpl.o \
./Sources/src/BlynkRpcInfra.o \
./Sources/src/BlynkRpcInfraUart.o \
./Sources/src/BlynkRpcUartFraming.o \
./Sources/src/MessageBuffer.o \
./Sources/src/MessageWriter.o \

OBJS_QUOTED += \
"./Sources/src/BlynkRpcCRC8.o" \
"./Sources/src/BlynkRpcClient.o" \
"./Sources/src/BlynkRpcClientWeakImpl.o" \
"./Sources/src/BlynkRpcInfra.o" \
"./Sources/src/BlynkRpcInfraUart.o" \
"./Sources/src/BlynkRpcUartFraming.o" \
"./Sources/src/MessageBuffer.o" \
"./Sources/src/MessageWriter.o" \

C_DEPS += \
./Sources/src/BlynkRpcCRC8.d \
./Sources/src/BlynkRpcClient.d \
./Sources/src/BlynkRpcClientWeakImpl.d \
./Sources/src/BlynkRpcInfra.d \
./Sources/src/BlynkRpcInfraUart.d \
./Sources/src/BlynkRpcUartFraming.d \
./Sources/src/MessageBuffer.d \
./Sources/src/MessageWriter.d \

OBJS_OS_FORMAT += \
./Sources/src/BlynkRpcCRC8.o \
./Sources/src/BlynkRpcClient.o \
./Sources/src/BlynkRpcClientWeakImpl.o \
./Sources/src/BlynkRpcInfra.o \
./Sources/src/BlynkRpcInfraUart.o \
./Sources/src/BlynkRpcUartFraming.o \
./Sources/src/MessageBuffer.o \
./Sources/src/MessageWriter.o \

C_DEPS_QUOTED += \
"./Sources/src/BlynkRpcCRC8.d" \
"./Sources/src/BlynkRpcClient.d" \
"./Sources/src/BlynkRpcClientWeakImpl.d" \
"./Sources/src/BlynkRpcInfra.d" \
"./Sources/src/BlynkRpcInfraUart.d" \
"./Sources/src/BlynkRpcUartFraming.d" \
"./Sources/src/MessageBuffer.d" \
"./Sources/src/MessageWriter.d" \


# Each subdirectory must supply rules for building sources it contributes
Sources/src/BlynkRpcCRC8.o: ../Sources/src/BlynkRpcCRC8.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/BlynkRpcCRC8.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/BlynkRpcCRC8.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/BlynkRpcClient.o: ../Sources/src/BlynkRpcClient.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/BlynkRpcClient.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/BlynkRpcClient.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/BlynkRpcClientWeakImpl.o: ../Sources/src/BlynkRpcClientWeakImpl.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/BlynkRpcClientWeakImpl.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/BlynkRpcClientWeakImpl.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/BlynkRpcInfra.o: ../Sources/src/BlynkRpcInfra.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/BlynkRpcInfra.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/BlynkRpcInfra.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/BlynkRpcInfraUart.o: ../Sources/src/BlynkRpcInfraUart.c
	@echo 'Building file: $<'
	@echo 'Executing target #5 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/BlynkRpcInfraUart.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/BlynkRpcInfraUart.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/BlynkRpcUartFraming.o: ../Sources/src/BlynkRpcUartFraming.c
	@echo 'Building file: $<'
	@echo 'Executing target #6 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/BlynkRpcUartFraming.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/BlynkRpcUartFraming.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/MessageBuffer.o: ../Sources/src/MessageBuffer.c
	@echo 'Building file: $<'
	@echo 'Executing target #7 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/MessageBuffer.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/MessageBuffer.o"
	@echo 'Finished building: $<'
	@echo ' '

Sources/src/MessageWriter.o: ../Sources/src/MessageWriter.c
	@echo 'Building file: $<'
	@echo 'Executing target #8 $<'
	@echo 'Invoking: ARM Ltd Windows GCC C Compiler'
	"$(ARMSourceryDirEnv)/arm-none-eabi-gcc" "$<" @"Sources/src/MessageWriter.args" -MMD -MP -MF"$(@:%.o=%.d)" -o"Sources/src/MessageWriter.o"
	@echo 'Finished building: $<'
	@echo ' '


