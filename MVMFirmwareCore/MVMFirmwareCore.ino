/****************************************************************************
 * MVM Ventialotor V4 Firmware
 *
 * This code is designed to run on the HW V4 of MVM Ventialor
 *
 * This code is distributed as DEMO and has NOT been tested and is NOT approved
 * by quality check of Nuclear Instruments or Elemaster
 *
 * !!!!   This code MUST NOT BE USED on final application    !!!!!
 *
 * Nuclear Instruments and Elemaster do not take responsibility for damage caused
 * by using this code and decline any responsability
 *
 * This code is property of Nuclear Instruments and distributed
 * with fair use license
 *
 ****************************************************************************/

// the setup function runs once when you press reset or power the board

#include "MVMCore.h"
#include <SimpleCLI.h>

MVMCore MVM;

// Create CLI Object
SimpleCLI cli;
// Commands
Command param_set;
Command param_get;

void SetCommandCallback(cmd* c);
void GetCommandCallback(cmd* c);
void CliErrorCallback(cmd_error* e);
void InitConsoleInterface();

void setup() {

    InitConsoleInterface();
    MVM.Init();
}

// the loop function runs over and over again until power down or reset
void loop() {
    MVM.Tick();

    if (MVM.DataAvailableOnUART0()) {
        
        String input = MVM.ReadUART0UntilEOL();
        cli.parse(input);
    }
}


void InitConsoleInterface()
{
    cli.setOnError(CliErrorCallback); // Set error Callback
    param_set = cli.addCommand("set", SetCommandCallback);
    param_set.addPositionalArgument("param", "null");
    param_set.addPositionalArgument("value", "0");

    param_get = cli.addCommand("get", GetCommandCallback);
    param_get.addPositionalArgument("param", "null");
}


 void SetCommandCallback(cmd* c)
{
    Command cmd(c); 

    Argument param = cmd.getArgument("param");
    Argument value = cmd.getArgument("value");

    String strPatam = param.getValue();
    String strValue = value.getValue();

    if (MVM.SetParameter(strPatam, strValue))
        MVM.WriteUART0("valore=OK");
    else
        MVM.WriteUART0("valore=ERROR:Invalid Command Argument");

}
 void GetCommandCallback(cmd* c)
 {
     Command cmd(c);
     Argument param = cmd.getArgument("param");
     String strPatam = param.getValue();
     String r;
     MVM.WriteUART0(MVM.GetParameter(strPatam));
 
 }

void CliErrorCallback(cmd_error* e)
{
    CommandError cmdError(e);
    Serial.print("valore=ERROR: ");
    Serial.println(cmdError.toString());
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
