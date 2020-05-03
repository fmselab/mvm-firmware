//
// File: test_qtl.cpp
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 23-Apr-2020 Initial version.
//
// Description:
// Quantity timelines JSON config test.
//

#include "quantity_timelines.hpp"

#include <iostream>

int 
main (int argc, char *argv[]) 
{
  if (argc < 2)
   {
    std::cerr << "Usage: " << argv[0] << " <JSON config> <Param1> <Param2> ..."
              << std::endl;
    return 1;
   }
  const char *json_conf = argv[1];

  quantity_timelines<float> qtl(json_conf);
  if (!qtl.parse_ok())
   {
    std::cerr << argv[0] << ": JSON parse error. Exiting." << std::endl;
    return 1;
   }

  for (int i=2; i<argc; ++i)
   {
    for (qtl_ms_t t = 0; t < 20000; t++)
     {
      std::cout << "t == " << t << " " << argv[i] << " == " << qtl.value(argv[i], t) << std::endl;
     }
   }
  return 0;
}

