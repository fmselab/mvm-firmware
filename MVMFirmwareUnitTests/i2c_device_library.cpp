//
// File: i2c_device_library.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 27-Apr-2020 Initial version.
//
// Description:
// Implementation of simulated I2C devices. As the time for development
// is short only commands that are actually used ade implemented. We try
// not to depend on any information coded in the Firmware itself though.
//

#include "i2c_device_library.h"

int
mvm_fw_unit_test_TE_MS5525DSO::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  return -1;
};

int
mvm_fw_unit_test_SENSIRION_SFM3019::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  return -1;
};

int
mvm_fw_unit_test_TI_ADS1115::handle_command(uint8_t cmd,
                 uint8_t *wbuffer, int wlength, uint8_t *rbuffer, int rlength)
{
  return -1;
};

