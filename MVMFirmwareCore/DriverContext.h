#pragma once
#include "hw.h"
#include "DebugIface.h"
typedef struct
{
	HW* hwi;
	DebugIfaceClass* dbg;
} DriverContext;

//                  #     # ### 
//                  ##    #  #  
//                  # #   #  #  
//                  #  #  #  #  
//                  #   # #  #  
//                  #    ##  #  
//                  #     # ### 
//
// Nuclear Instruments 2020 - All rights reserved
// Any commercial use of this code is forbidden
// Contact info@nuclearinstruments.eu
