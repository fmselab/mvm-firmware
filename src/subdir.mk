################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/WString.cpp \
../src/fmselab-mvm-firmware.cpp 

C_SRCS += \
../src/stdlib_noniso.c 

OBJS += \
./src/WString.o \
./src/fmselab-mvm-firmware.o \
./src/stdlib_noniso.o 

CPP_DEPS += \
./src/WString.d \
./src/fmselab-mvm-firmware.d 

C_DEPS += \
./src/stdlib_noniso.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -Isrc -I"C:\Users\AngeloGargantini\eclipsews\mvm\temp\src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


