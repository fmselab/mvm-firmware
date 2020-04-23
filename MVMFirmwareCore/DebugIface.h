// DebugIface.h

#ifndef _DEBUGIFACE_h
#define _DEBUGIFACE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "hw.h"

typedef enum {	DBG_NONE=0, 
				DBG_CRITICAL = 1, 
				DBG_ERROR = 2, 
				DBG_WARNING = 3, 
				DBG_INFO = 4, 
				DBG_VALUE = 5, 
				DBG_ALL = 6} verbose_level;

typedef enum {DBG_CODE, DBG_KERNEL} dbg_source;

class DebugIfaceClass
{
private:
	HW* hwi;
	verbose_level vsl;
 protected:


 public:
	void Init(verbose_level _vs, HW *_hwi);
	void SetVerboseLevel(verbose_level _vs);
	void DbgPrint(dbg_source source, verbose_level vl, String s);

};

extern DebugIfaceClass DebugIface;

#endif



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
