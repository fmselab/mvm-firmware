#
# CPPFLAGS = -DLIBVNCSERVER_HAVE_LIBZ -DLIBVNCSERVER_HAVE_LIBJPEG

#CXXFLAGS  = -Wall -Werror -Wextra -pedantic -ansi
CXXFLAGS  = -Wall -O0 -g3
#
# firmware objects
WRAPPER_ROOT := ../MVMFirmwareUnitTests

MVM_FW_ROOT := ../MVMFirmwareCpp/MVMFirmwareCore
MVM_FW_INCLUDES := -I $(MVM_FW_ROOT) -I $(WRAPPER_ROOT)
MVM_FIRMWARE_OBJECTS = Alarms.o \
                       CircularBuffer.o \
                       ConfigManager.o \
                       DebugIface.o \
                       driver_5525DSO.o \
                       driver_ADS1115.o \
                       driver_OxygenSensor.o \
                       driver_SFM3019.o \
                       driver_Supervisor.o \
                       driver_VenturiFlowMeter.o \
                       HAL.o \
                       hw_ard_esp32.o \
                       hw.o \
                       MVMCore.o \
                       MVM_StateMachine.o \
                       PressureLoop.o \
                       TidalVolume.o

#                       fw_board_ni_v4.o \
#                       fw_board_razzeto_v3.o \


%.o: $(MVM_FW_ROOT)/%.cpp ;\
   $(CXX)  $(CXXFLAGS) -c $< $(MVM_FW_INCLUDES)

# test objects

TEST_ROOT := .

TEST_OBJECTS =  testMVM.o 

TEST_INCLUDES := -I $(MVM_FW_ROOT) -I $(WRAPPER_ROOT)


%.o: $(TEST_ROOT)/%.cpp ;\
   $(CXX) $(CXXFLAGS) -c $< $(TEST_INCLUDES)


# link together

TARGET_DIR = ./target
TARGET =	runtests.exe

$(TARGET):	$(MVM_FIRMWARE_OBJECTS) $(WRAPPER_OBJECTS) $(TEST_OBJECTS) 
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MVM_FIRMWARE_OBJECTS)  $(WRAPPER_OBJECTS) $(TEST_OBJECTS) $(LIBS)

all: $(TARGET);

clean: ; /bin/rm *.o *.exe;
