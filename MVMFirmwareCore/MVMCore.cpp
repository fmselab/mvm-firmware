// 
// 
// 



#include "MVMCore.h"
#include <functional>
using namespace std::placeholders; 

void MVMCore::Init()
{
	MVM_HAL.Init();
	
	CMC.Init(this, &sys_s, &Alarms);
	CMC.addHandler_AfterConfigurationSet(std::bind(&MVMCore::ConfigurationChanged_Event, this));

	MVM_HAL.SetInputValve(0);
	MVM_HAL.SetOutputValve(0);
	MVM_HAL.SetAlarmLed(false);
	MVM_HAL.SetAlarmRele(false);
	MVM_HAL.SetBuzzer(false);
	MVM_HAL.addHandler_PLoop(std::bind(&MVMCore::PLoop_Event, this));
	MVM_HAL.addHandler_PPatient(std::bind(&MVMCore::PPatient_Event, this));
	MVM_HAL.addHandler_FlowSens(std::bind(&MVMCore::FlowIn_Event, this));
	MVM_HAL.addHandler_FlowVenturi(std::bind(&MVMCore::FlowVenturi_Event, this));
	MVM_HAL.addHandler_HardwareAlarm(std::bind(&MVMCore::HardwareAlarm_Event, this, _1));
	MVM_SM.addHandler_NewCycle(std::bind(&MVMCore::NewCycle_Event, this));
	MVM_SM.addHandler_Exhale(std::bind(&MVMCore::Exhale_Event, this));
	MVM_SM.addHandler_EndCycle(std::bind(&MVMCore::EndCycle_Event, this));

	MEM_Ppatient_LP = new CircularBuffer(10);
	
	old_delta_ppatient = 0;
	sys_s.pPatient = 0;
	sys_s.pLoop = 0;
	sys_s.FlowIn = 0;
	
	sys_s.batteryPowered = false;
	sys_s.currentBatteryCharge = 100;
	sys_s.in_over_pressure_emergency = false;
	
	Alarms.Init(&MVM_HAL, &sys_s);

	MVM_SM.Init(&MVM_HAL, &Alarms, &CMC.core_config, &sys_s, 10);

	averaged_PPatient = 0;
	
	MVM_HAL.delay_ms(100);
	MVM_HAL.SetInputValve(0);
	MVM_HAL.SetOutputValve(true);
	MVM_HAL.delay_ms(3000);

	MVM_HAL.dbg.DbgPrint(DBG_CODE, DBG_INFO, "Calibrating pressure sensors. Idraulic circuit must be opened");
	float pzero1 = MVM_HAL.ZeroPressureSensor(PS_LOOP);
	float pzero2 = MVM_HAL.ZeroPressureSensor(PS_PATIENT);
	float pzero3 = MVM_HAL.ZeroPressureSensor(PS_VENTURI);
	MVM_HAL.dbg.DbgPrint(DBG_CODE, DBG_INFO, "ZERO -> PLOOP: " + String(pzero1) + " PPATIENT: " + String(pzero2) + " PVENTURI: " + String(pzero3));
	MVM_HAL.SetOutputValve(false);

	last_debug_console_log = MVM_HAL.GetMillis();
	last_alarm_CT = MVM_HAL.GetMillis();
	alarm_enable = false;
}
void MVMCore::Tick()
{
	MVM_HAL.Tick();

	sys_s.pLoop = MVM_HAL.GetPressureValve(0);
	sys_s.pPatient = MVM_HAL.GetPressurePatient(0);
	sys_s.FlowIn = MVM_HAL.GetFlowInspire(0);
	sys_s.FlowVenturi = MVM_HAL.GetFlowVenturi(0);
	
	MVM_HAL.GetInputValvePID(&sys_s.pid_valvein_slow, &sys_s.pid_valvein_fast);

	if (flush_pipe_mode==false)
		MVM_SM.Tick();

	sys_s.last_O2 = MVM_HAL.GetOxygen();
	MVM_HAL.GetPowerStatus(&sys_s.batteryPowered, &sys_s.currentBatteryCharge);

	if (MVM_HAL.Get_dT_millis(last_debug_console_log)>5)
	{
		last_debug_console_log = MVM_HAL.GetMillis();
		if (CMC.core_config.__CONSOLE_MODE)
		{
			MVMDebugPrintLogger();
		}
	}

	if (MVM_HAL.Get_dT_millis(last_alarm_CT) > 5000)
	{
		alarm_enable = true;
		
	}
	
	if (alarm_enable)
		Alarms.Tick();
}

bool MVMCore::DataAvailableOnUART0()
{
	return MVM_HAL.DataAvailableOnUART0();
}

String MVMCore::ReadUART0UntilEOL()
{
	return MVM_HAL.ReadUART0UntilEOL();
}

bool MVMCore::WriteUART0(String s)
{
	return MVM_HAL.WriteUART0(s);
}

bool MVMCore::SetParameter(String p, String v)
{
	return CMC.SetParameter(p, v);
}
String MVMCore::GetParameter(String p)
{
	return CMC.GetParameter(p);
}

void MVMCore::PLoop_Event()
{
	float v = MVM_HAL.GetPressureValve(0);

	//SAFETY
	if (v > PV1_SAFETY_LIMIT)
	{
		Alarms.Action_OverPressureSecurity();
	}

}
void MVMCore::PPatient_Event()
{
	float v = MVM_HAL.GetPressurePatient(0);
	sys_s.pPatient_low_passed = 0.90 * sys_s.pPatient_low_passed + v * 0.1;
	sys_s.pres_peak = sys_s.pPatient_low_passed > sys_s.pres_peak ?
							sys_s.pPatient_low_passed : sys_s.pres_peak;
	MEM_Ppatient_LP->PushData(sys_s.pPatient_low_passed);

	sys_s.PPatient_delta = v - MEM_Ppatient_LP->GetData(5);
	sys_s.PPatient_delta2 = sys_s.PPatient_delta - old_delta_ppatient;
	old_delta_ppatient = sys_s.PPatient_delta;

	

	averaged_PPatient = (MVM_HAL.GetPressurePatient(5) +
		MVM_HAL.GetPressurePatient(6) +
		MVM_HAL.GetPressurePatient(7) +
		MVM_HAL.GetPressurePatient(8) +
		MVM_HAL.GetPressurePatient(9)) / 5.0;

	//SAFETY
	if (v > PV1_SAFETY_LIMIT)
	{
		Alarms.Action_OverPressureSecurity();
	}

	//Serial.println("CALLBACK P: " + String(v));
}

void MVMCore::FlowIn_Event()
{
	float v = MVM_HAL.GetFlowInspire(0);
	sys_s.fluxpeak = sys_s.fluxpeak > v ? sys_s.fluxpeak : v;
	TidalVolumeExt.PushDataSens(v);
	sys_s.TidalVolume = TidalVolumeExt.liveVolume;
	sys_s.Flux = TidalVolumeExt.liveFlux;
	sys_s.GasTemperature = MVM_HAL.GetGasTemperature();
	//Serial.println("CALLBACK F: " + String(v));
}

void MVMCore::FlowVenturi_Event()
{
	float vf;
	vf = MVM_HAL.GetFlowVenturi(0);
	TidalVolumeExt.PushDataVenturi(vf);
	sys_s.VenturiFlux = vf;
	sys_s.dt_veturi_100ms= MVM_HAL.GetPVenturi(5);
	sys_s.VenturiP = MVM_HAL.GetPVenturi(0);
	sys_s.TidalVolume = TidalVolumeExt.liveVolume;
	sys_s.Flux = TidalVolumeExt.liveFlux;
}


void MVMCore::NewCycle_Event()
{
	MVM_HAL.CorrectZeroPressureSensor(PS_VENTURI, sys_s.dt_veturi_100ms);

	TidalVolumeExt.DoNewCycle();
	float dTact = (float)MVM_HAL.Get_dT_millis(last_respiratory_act);
	last_respiratory_act = MVM_HAL.GetMillis();

	if (dTact > 0)
	{
		sys_s.last_bpm = 60000.0 / dTact;
	}
	else
	{
		sys_s.last_bpm = 0;
	}
	sys_s.averaged_bpm = 0.8 * sys_s.averaged_bpm + 0.2 * sys_s.last_bpm;

	sys_s.last_peep = averaged_PPatient;
	sys_s.pres_peak = 0;
	sys_s.fluxpeak = 0;
	
	Alarms.TransitionNewCycleEvent();
}
void MVMCore::Exhale_Event()
{
	TidalVolumeExt.DoExhale();
	sys_s.currentTvIsnp = TidalVolumeExt.currentTvIsnp;
	sys_s.currentVM = sys_s.currentTvIsnp * sys_s.last_bpm;
	sys_s.currentF_Peak = TidalVolumeExt.currentFluxPeak;
	sys_s.currentP_Peak = sys_s.pres_peak;
	Alarms.TransitionInhaleExhale_Event();
}
void MVMCore::EndCycle_Event()
{
	TidalVolumeExt.DoEndCycle();
	sys_s.currentTvEsp = TidalVolumeExt.currentTvEsp;

	Alarms.TransitionEndCycleEvent();
}


void MVMCore::ConfigurationChanged_Event()
{
	MVM_HAL.ConfigureInputValvePID(CMC.core_config.P,
		CMC.core_config.I,
		CMC.core_config.D,
		CMC.core_config.P2,
		CMC.core_config.I2,
		CMC.core_config.D2,
		CMC.core_config.pid_limit
	);
}


void MVMCore::HardwareAlarm_Event(t_ALARM alarm_code)
{
	Alarms.TriggerAlarm(alarm_code);
}


void MVMCore::ZeroSensors(float *sensors, int *count)
{
	float pzero1 = MVM_HAL.ZeroPressureSensor(PS_LOOP);
	float pzero2 = MVM_HAL.ZeroPressureSensor(PS_PATIENT);
	float pzero3 = MVM_HAL.ZeroPressureSensor(PS_VENTURI);

	if (*count >= 3)
	{
		sensors[0] = pzero1;
		sensors[1] = pzero2;
		sensors[2] = pzero3;
		*count = 3;
	}
}

void MVMCore::CalibrateOxygenSensor()
{
	MVM_HAL.CalibrateOxygenSensorInAir();
	//TODO
}

bool MVMCore::FlushPipes(bool run, float valve_percent)
{
	if (!CMC.core_config.run)
	{
		MVM_HAL.FlushPipes(run, valve_percent);
		flush_pipe_mode = run;
		return true;
	}
	else
	{
		MVM_HAL.FlushPipes(false, 0);
		flush_pipe_mode = false;
		return false;
	}

	//TODO
}

void MVMCore::DOVenturiMeterScan()
{
	MVM_HAL.DOVenturiMeterScan();
}


void MVMCore::MVMDebugPrintLogger()
{
	float pid_slow, pid_fast;
	float InputValveSetPoint;
	float OutputValveSetPoint;
	MVM_HAL.GetInputValvePID(&pid_slow, &pid_fast);
	InputValveSetPoint = MVM_HAL.GetInputValve();
	OutputValveSetPoint = MVM_HAL.GetOutputValve() * 100;

	String ts = CMC.core_config.__ADDTimeStamp ? String((uint32_t)MVM_HAL.GetMillis()) + "," : "";
	/*
	MVM_HAL.WriteUART0(ts+ 
		String(sys_s.FlowIn) + "," +
		String(sys_s.pLoop) + "," +
		String(sys_s.pPatient) + "," +
		String(pid_fast) + "," +
		String(pid_slow) + "," +
		String(OutputValveSetPoint) + "," +
		String(sys_s.VenturiFlux) + "," +
		String(sys_s.Flux) + "," +
		String(sys_s.TidalVolume) + "," +
		String(sys_s.PPatient_delta2 * 10)
		);
	*/

	MVM_HAL.WriteUART0(
		String(sys_s.pLoop) + "," +
		String(sys_s.pPatient) + "," +
		String(pid_fast) + "," +
		String(pid_slow) 
	);
	
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
