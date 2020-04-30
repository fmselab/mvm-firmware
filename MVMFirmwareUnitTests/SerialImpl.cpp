// 
// File: SerialImpl.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 30-Apr-2020 Initial version.
//
// Description:
// Implementation of an Arduino-like 'Serial' class, funnelling the
// test results .
//

#include <ctime>

#include "mvm_fw_unit_test_config.h"
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <poll.h>

#include "Serial.h"

bool
SerialImpl::available()
{
  if (m_ttys == 0) return false;
  pollfd spfd;
  spfd.fd = ::fileno(m_ttys);
  spfd.events = POLLIN|POLLERR|POLLHUP|POLLNVAL;
  spfd.revents = 0;
  int rpoll = ::poll(&spfd, 1, FW_TEST_serial_poll_timeout);
  if ((rpoll >= 1) && (spfd.revents == POLLIN)) return true;

  return false;
}

void
SerialImpl::begin(unsigned long baud, uint32_t config, int8_t rxPin,
             int8_t txPin, bool invert, unsigned long timeout_ms)
{}

size_t
SerialImpl::println(const String &str)
{
  return println(str.c_str());
}

size_t
SerialImpl::println(const char str[])
{
  if (m_ttys == 0) return -1;
  int ret = ::fprintf(m_ttys, "%s\n", str);
  ::fflush(m_ttys);
  return ret;
}

size_t
SerialImpl::println(unsigned long val, int base)
{
  std::ostringstream ostr;
  ostr << std::setbase(base) << val;
  return println(ostr.str().c_str());
}

size_t
SerialImpl::print(const String &str)
{
  return print(str.c_str());
}

size_t
SerialImpl::print(const char str[])
{
  if (m_ttys == 0) return -1;
  int ret = ::fprintf(m_ttys, "%s", str);
  ::fflush(m_ttys);
  return ret;
}

String
SerialImpl::readStringUntil(char end)
{
  char *result = NULL;
  size_t len = 0;
  String sres;
  int ret = ::getdelim(&result, &len, end, m_ttys);
  if ((ret >= 0) && (result != NULL))
   {
    sres = result;
   }
  if (result != NULL) ::free(result);
  return sres;
}

