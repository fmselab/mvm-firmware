################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
INO_SRCS += \
../MVMFirmwareCore/MVMFirmwareCore.ino 

CPP_SRCS += \
../MVMFirmwareCore/Alarms.cpp \
../MVMFirmwareCore/CircularBuffer.cpp \
../MVMFirmwareCore/ConfigManager.cpp \
../MVMFirmwareCore/DebugIface.cpp \
../MVMFirmwareCore/HAL.cpp \
../MVMFirmwareCore/MVMCore.cpp \
../MVMFirmwareCore/MVM_StateMachine.cpp \
../MVMFirmwareCore/PressureLoop.cpp \
../MVMFirmwareCore/TidalVolume.cpp \
../MVMFirmwareCore/driver_5525DSO.cpp \
../MVMFirmwareCore/driver_ADS1115.cpp \
../MVMFirmwareCore/driver_OxygenSensor.cpp \
../MVMFirmwareCore/driver_SFM3019.cpp \
../MVMFirmwareCore/driver_Supervisor.cpp \
../MVMFirmwareCore/driver_VenturiFlowMeter.cpp \
../MVMFirmwareCore/fw_board_ni_v4.cpp \
../MVMFirmwareCore/fw_board_razzeto_v3.cpp \
../MVMFirmwareCore/hw.cpp \
../MVMFirmwareCore/hw_ard_esp32.cpp 

OBJS += \
./MVMFirmwareCore/Alarms.o \
./MVMFirmwareCore/CircularBuffer.o \
./MVMFirmwareCore/ConfigManager.o \
./MVMFirmwareCore/DebugIface.o \
./MVMFirmwareCore/HAL.o \
./MVMFirmwareCore/MVMCore.o \
./MVMFirmwareCore/MVMFirmwareCore.o \
./MVMFirmwareCore/MVM_StateMachine.o \
./MVMFirmwareCore/PressureLoop.o \
./MVMFirmwareCore/TidalVolume.o \
./MVMFirmwareCore/driver_5525DSO.o \
./MVMFirmwareCore/driver_ADS1115.o \
./MVMFirmwareCore/driver_OxygenSensor.o \
./MVMFirmwareCore/driver_SFM3019.o \
./MVMFirmwareCore/driver_Supervisor.o \
./MVMFirmwareCore/driver_VenturiFlowMeter.o \
./MVMFirmwareCore/fw_board_ni_v4.o \
./MVMFirmwareCore/fw_board_razzeto_v3.o \
./MVMFirmwareCore/hw.o \
./MVMFirmwareCore/hw_ard_esp32.o 

INO_DEPS += \
./MVMFirmwareCore/MVMFirmwareCore.d 

CPP_DEPS += \
./MVMFirmwareCore/Alarms.d \
./MVMFirmwareCore/CircularBuffer.d \
./MVMFirmwareCore/ConfigManager.d \
./MVMFirmwareCore/DebugIface.d \
./MVMFirmwareCore/HAL.d \
./MVMFirmwareCore/MVMCore.d \
./MVMFirmwareCore/MVM_StateMachine.d \
./MVMFirmwareCore/PressureLoop.d \
./MVMFirmwareCore/TidalVolume.d \
./MVMFirmwareCore/driver_5525DSO.d \
./MVMFirmwareCore/driver_ADS1115.d \
./MVMFirmwareCore/driver_OxygenSensor.d \
./MVMFirmwareCore/driver_SFM3019.d \
./MVMFirmwareCore/driver_Supervisor.d \
./MVMFirmwareCore/driver_VenturiFlowMeter.d \
./MVMFirmwareCore/fw_board_ni_v4.d \
./MVMFirmwareCore/fw_board_razzeto_v3.d \
./MVMFirmwareCore/hw.d \
./MVMFirmwareCore/hw_ard_esp32.d 


# Each subdirectory must supply rules for building sources it contributes
MVMFirmwareCore/%.o: ../MVMFirmwareCore/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -Isrc -I"C:\Users\AngeloGargantini\eclipsews\mvm\temp\src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

MVMFirmwareCore/%.o: ../MVMFirmwareCore/%.ino
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -Isrc -I"C:\Users\AngeloGargantini\eclipsews\mvm\temp\src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


