/* Include file stub for standalone compilation */

#include <cmath>

#include "WString.h"

// Empty stubs:
#include "Wire.h" 

unsigned long millis();
void delay(uint32_t);

#include "Serial.h"

// Make sure we don't include the definition in the firmware code.
#define _FW_BOARD_NI_V4_h

