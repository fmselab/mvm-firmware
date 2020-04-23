// 
// 
// 

#include "DebugIface.h"

void DebugIfaceClass::Init(verbose_level _vs, HW* _hwi)
{
	hwi = _hwi;
	vsl = _vs;
}
void DebugIfaceClass::SetVerboseLevel(verbose_level _vs)
{
	vsl = _vs;
}
void DebugIfaceClass::DbgPrint(dbg_source source, verbose_level vl, String s)
{
	String msg;
	if (vl <= vsl)
	{
		String src = (source == DBG_CODE ? "CODE" : "KERNEL");
		msg = "[" + src + "] - " + s;
		hwi->PrintLineDebugConsole(msg);
	}
}



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
