#
# File: Makefile
#
# Author: Francesco Prelz Francesco.Prelz@mi.infn.it
#
# Revision history:
#
#  23-Apr-2020 Initial creation.
#
# Description:
# Unix makefile for building the MVM Firmware Unit test framework.
#

MVM_FW_ROOT := ../MVMFirmwareCore
MVM_FW_INCLUDES := -I . -I $(MVM_FW_ROOT) -I rapidjson/include -I exprtk

DEBUG = -g 

CPPFLAGS := $(MVM_FW_INCLUDES) $(CPPFLAGS)
CXXFLAGS := $(DEBUG) $(CXXFLAGS)

                       
MVM_FIRMWARE_OBJECTS = HAL.o \
                       Alarms.o \
                       CircularBuffer.o \
                       ConfigManager.o \
                       DebugIface.o \
                       driver_5525DSO.o \
                       driver_ADS1115.o \
                       driver_OxygenSensor.o \
                       driver_SFM3019.o \
                       driver_Supervisor.o \
                       driver_VenturiFlowMeter.o \
                       hw_ard_esp32.o \
                       MVMCore.o \
                       MVM_StateMachine.o \
                       PressureLoop.o \
                       TidalVolume.o \
                       hw.o \
                       stdlib_noniso.o WString.o

MWM_FW_TEST_OBJECTS =  simulated_fw_board_v4.o \
                       mvm_fw_unit_test_config.o \
                       mvm_fw_unit_test_run.o

MVM_FW_TEST_EXECUTABLES = mvm_fw_unit_test_run

all: $(MVM_FIRMWARE_OBJECTS) $(MWM_FW_TEST_OBJECTS) $(MVM_FW_TEST_EXECUTABLES) ;

mvm_fw_unit_test_run.o: mvm_fw_unit_test_run.cpp quantity_timelines.hpp ;\
   $(COMPILE.C) -c mvm_fw_unit_test_run.cpp


mvm_fw_unit_test_run: $(MWM_FW_TEST_OBJECTS) $(MVM_FIRMWARE_OBJECTS); \
    $(LINK.C) -o $@ $(MWM_FW_TEST_OBJECTS) $(MVM_FIRMWARE_OBJECTS)

%.o: $(MVM_FW_ROOT)/%.cpp ;\
   $(COMPILE.C) -c $< $(MVM_FW_INCLUDES)

clean: ; /bin/rm *.o $(MVM_FW_TEST_EXECUTABLES);
