// 
// 
// 

#include "ConfigManager.h"
#include "FwVersion.h"
#include "generic_definitions.h"
#include "UtilsFn.h"
#include "MVMCore.h"

void ConfigManagerClass::Init(void* _core, t_SystemStatus* _sys_s, AlarmClass *_Alarms)
{
    sys_s = _sys_s;
    core = _core;
    Alarms = _Alarms;
    core_config.run = false;
    core_config.inhale_ms = 750;
    core_config.exhale_ms = 1250;
    core_config.BreathMode = M_BREATH_FORCED; //M_BREATH_ASSISTED;//;
    core_config.flux_close = 5;
    core_config.assist_pressure_delta_trigger = 15;
    core_config.target_pressure = 20;
    core_config.target_pressure_auto = core_config.target_pressure;
    core_config.target_pressure_assist = core_config.target_pressure;

    core_config.respiratory_rate = 15;
    core_config.respiratory_ratio = 0.66;
    core_config.inhale_ms = 60000.0 / core_config.respiratory_rate * (1 - core_config.respiratory_ratio);
    core_config.exhale_ms = 60000.0 / core_config.respiratory_rate * (core_config.respiratory_ratio);

    core_config.P = 70;
    core_config.I = 5;
    core_config.D = 0;

    core_config.P2 = 1.0;
    core_config.I2 = 0.1;
    core_config.D2 = 0;

    core_config.pid_limit = 0.95;

    core_config.pause_lg = false;
    core_config.pause_lg_timer = 0;

    core_config.__ADDTimeStamp = true;
    core_config.__CONSOLE_MODE = false;

}

bool ConfigManagerClass::SetParameter(String p, String v)
{
    if (callback_BeforeConfigurationSet)
    {
        callback_BeforeConfigurationSet();
    }
    bool bres = false;
    String strPatam = p;

    //Serial.println("CMD: " +  param.getValue() + " " +  value.getValue());

    if (strPatam == "run") {
        int numberValue = v.toInt();

        if (numberValue < 1)
            core_config.run = false;
        else
            core_config.run = true;

        bres = true;
    }

    if (strPatam == "mode") {
        int numberValue = v.toInt();
        if (numberValue == 0) {
            //Forced Mode
            core_config.BreathMode = M_BREATH_FORCED;
        }
        else {
            //Assisted Mode
            core_config.BreathMode = M_BREATH_ASSISTED;
        }
        bres = true;
    }

    if (strPatam == "rate") {
        float numberValue = v.toFloat();
        core_config.respiratory_rate = numberValue;
        core_config.inhale_ms = 60000.0 / core_config.respiratory_rate * (1 - core_config.respiratory_ratio);
        core_config.exhale_ms = 60000.0 / core_config.respiratory_rate * (core_config.respiratory_ratio);
        bres = true;
    }

    if (strPatam == "ratio") {
        float numberValue = v.toFloat();
        core_config.respiratory_ratio = numberValue;
        core_config.inhale_ms = 60000.0 / core_config.respiratory_rate * (1 - core_config.respiratory_ratio);
        core_config.exhale_ms = 60000.0 / core_config.respiratory_rate * (core_config.respiratory_ratio);
        bres = true;
    }

    if (strPatam == "assist_ptrigger") {
        float numberValue = v.toFloat();
        core_config.assist_pressure_delta_trigger = numberValue;
        bres = true;
    }

    if (strPatam == "assist_flow_min") {
        float numberValue = v.toFloat();
        core_config.flux_close = numberValue;
        bres = true;
    }

    if (strPatam == "ptarget") {
        float numberValue = v.toFloat();
        core_config.target_pressure_auto = numberValue;
        bres = true;
    }

    if (strPatam == "pressure_support") {
        float numberValue = v.toFloat();
        core_config.target_pressure_assist = numberValue;
        bres = true;
    }

    if (strPatam == "pid_p") {
        float numberValue = v.toFloat();
        core_config.P = numberValue;
        bres = true;
    }

    if (strPatam == "pid_i") {
        float numberValue = v.toFloat();
        core_config.I = numberValue;
        bres = true;
    }

    if (strPatam == "pid_d") {
        float numberValue = v.toFloat();
        core_config.D = numberValue;
        bres = true;
    }

    if (strPatam == "pid_p2") {
        float numberValue = v.toFloat();
        core_config.P2 = numberValue;
        bres = true;
    }

    if (strPatam == "pid_i2") {
        float numberValue = v.toFloat();
        core_config.I2 = numberValue;
        bres = true;
    }

    if (strPatam == "pid_d2") {
        float numberValue = v.toFloat();
        core_config.D2 = numberValue;
        bres = true;
    }

    if (strPatam == "pause_inhale") {
        int numberValue = v.toInt();
        core_config.pause_inhale = numberValue;
        core_config.pause_timeout = 500 ;
        bres = true;
    }

    if (strPatam == "pause_lg") {
        int numberValue = v.toInt();
        core_config.pause_lg = numberValue ? true : false;
        bres = true;
    }

    if (strPatam == "pause_lg_time") {
        int numberValue = v.toFloat();
        core_config.pause_lg_timer = numberValue * 1000.0;
        bres = true;
    }

    if (strPatam == "pause_lg_p") {
        int numberValue = v.toFloat();
        core_config.pause_lg_p = numberValue;
        bres = true;
    }

    if (strPatam == "pause_exhale") {
        int numberValue = v.toInt();
        core_config.pause_exhale = numberValue;
        core_config.pause_timeout = 500;
        bres = true;
    }

    if (strPatam == "pid_limit") {
        float numberValue = v.toFloat();
        core_config.pid_limit = numberValue;
        bres = true;
    }

    if (strPatam == "alarm_snooze") {
        int numberValue = v.toInt();
        if (numberValue == 29)
        {
            Alarms->SetAlarmGUI(false);
        }
        else
            Alarms->ResetAlarm();
        bres = true;
    }

    if (strPatam == "alarm") {
        int numberValue = v.toInt();
        Alarms->SetAlarmGUI(numberValue != 0 ?true:false);
        bres = true;
    }

    if (strPatam == "watchdog_reset") {
        int numberValue = v.toInt();
        Alarms->ResetWatchDog();
        sys_s->ALARM_FLAG = sys_s->ALARM_FLAG & (~GenerateFlag(__ERROR_WDOG_PI));
        bres = true;
    }

    if (strPatam == "console") {
        int numberValue = v.toInt();
        core_config.__CONSOLE_MODE = numberValue != 0 ? true : false;
        bres = true;
    }

    if (strPatam == "timestamp") {
        int numberValue = v.toInt();
        core_config.__ADDTimeStamp = numberValue != 0 ? true : false;
        bres = true;
    }

    if (strPatam == "pcv_trigger_enable") {
        int numberValue = v.toInt();
        core_config.pcv_trigger_enable = numberValue ? true : false;
        Serial.println("valore=OK");
    }
    if (strPatam == "pcv_trigger") {
        float numberValue = v.toFloat();
        core_config.pcv_trigger = numberValue;
        Serial.println("valore=OK");
    }


    if (strPatam == "wdenable") {
        int numberValue = v.toInt();
        core_config.__WDENABLE = numberValue != 0 ? true : false;
        Alarms->ResetWatchDog();
        Alarms->EnableWatchDog(core_config.__WDENABLE);
        bres = true;
    }

    if (strPatam == "backup_enable") {
        int numberValue = v.toInt();
        core_config.backup_enable = numberValue ? true : false;
        bres = true;
    }

    if (strPatam == "backup_min_time") {
        float numberValue = v.toFloat();
        numberValue = numberValue < 1 ? 1 : numberValue;
        core_config.backup_min_rate = numberValue;
        bres = true;
    }

    if (strPatam == "stats_clear") {
        //ResetStatsBegin();
        bres = true;
    }

    if (strPatam == "flush_pipe") {
        float numberValue = v.toFloat();
        bool enable = numberValue < 1 ? false:true;
        ((MVMCore*)core)->FlushPipes(enable, numberValue);
        bres = true;
    }
    

    if (callback_AfterConfigurationSet)
    {
        callback_AfterConfigurationSet();
    }

    return bres;
}
String ConfigManagerClass::GetParameter(String p)
{
    if (callback_BeforeConfigurationGet)
        callback_BeforeConfigurationGet();
    String strPatam = p;

    //Serial.println("CMD: " +  param.getValue() + " " +  value.getValue());

    if (strPatam == "pressure") {
        return "valore=" + String(sys_s->pLoop);
   
    }

    if (strPatam == "ppressure") {
        return "valore=" + String(sys_s->pPatient);

    }

    if (strPatam == "flow") {
        return "valore=" + String(sys_s->FlowIn);

    }

    if (strPatam == "o2") {
        return "valore=" + String(sys_s->last_O2);
        
    }

    if (strPatam == "bpm") {
        return "valore=" + String(sys_s->last_bpm);
    }

    if (strPatam == "backup") {
        return "valore=" + String(0);

    }

    if (strPatam == "tidal") {
        return "valore=" + String(sys_s->TidalVolume);
    }

    if (strPatam == "peep") {
        return "valore=" + String(sys_s->last_peep);
    }

    if (strPatam == "temperature") {
        return "valore=" + String(sys_s->GasTemperature);
    }

    if (strPatam == "power_mode") {
        return "valore=" + String(sys_s->batteryPowered ? 1 : 0);
    }

    if (strPatam == "battery") {
        return "valore=" + String(sys_s->currentBatteryCharge);
    }

    if (strPatam == "version") {
        return "valore=" + String(_FIRMWARE_VERSION_);
    }

    if (strPatam == "alarm") {
        return "valore=" + String(sys_s->ALARM_FLAG);
    }

    if (strPatam == "warning") {
        return "valore=" + String(sys_s->WARNING_FLAG);
    }

    if (strPatam == "run") {
        return "valore=" + String(core_config.run ? 1 : 0);
    }

    if (strPatam == "mode") {
        return "valore=" + String(core_config.BreathMode == M_BREATH_ASSISTED ? 1 : 0);
    }
    if (strPatam == "rate") {
        return "valore=" + String(core_config.respiratory_rate);
    }
    if (strPatam == "ratio") {
        return "valore=" + String(core_config.respiratory_ratio);
    }
    if (strPatam == "assist_ptrigger") {
        return "valore=" + String(core_config.assist_pressure_delta_trigger);
    }
    if (strPatam == "assist_flow_min") {
        return "valore=" + String(core_config.flux_close);
    }
    if (strPatam == "ptarget") {
        return  "valore=" + String(core_config.target_pressure_auto);
    }
    if (strPatam == "pressure_support") {
        return  "valore=" + String(core_config.target_pressure_assist);
    }
    if (strPatam == "backup_enable") {
        return  "valore=" + String(core_config.backup_enable ? 1 : 0);

    }
    if (strPatam == "backup_min_rate") {
        return  "valore=" + String(core_config.backup_min_rate);
    }

    if (strPatam == "pause_lg") {
        return  "valore=" + String(core_config.pause_lg);
    }

    if (strPatam == "pause_lg_time") {
        return  "valore=" + String(core_config.pause_lg_timer / 1000.0);
    }

    if (strPatam == "pause_lg_p") {
        return  "valore=" + String(core_config.pause_lg_p);
    }
    if (strPatam == "all") {
        return "valore=" + String(sys_s->pPatient) 
            + "," + String(sys_s->Flux)
            + "," + String(sys_s->last_O2)
            + "," + String(sys_s->last_bpm)
            + "," + String(sys_s->TidalVolume) 
            + "," + String(sys_s->last_peep)
            + "," + String(sys_s->GasTemperature) 
            + "," + String(sys_s->batteryPowered ? 1 : 0) 
            + "," + String(sys_s->currentBatteryCharge)
            + "," + String(sys_s->currentP_Peak)
            + "," + String(sys_s->currentTvIsnp*1000.0)
            + "," + String(sys_s->currentTvEsp * 1000.0)
            + "," + String(sys_s->currentVM);
    }

    if (strPatam == "calib") {
        float zeros[4];
        int count=4;
        ((MVMCore*)core)->ZeroSensors(zeros,&count);
        String outval ="valore=";
        for (int i = 0; i < count; i++)
        {
            if (i != count - 1)
                outval += String(zeros[i]) + ",";
            else
                outval += String(zeros[i]);
        }
        return outval;
    }

    if (strPatam == "calib_o2") {
        ((MVMCore*)core)->CalibrateOxygenSensor();
        return "valore=OK";
    }

    if (strPatam == "calibv") {
        /*if (fabs(tidal_volume_c.ExpVolumeVenturi) > 0)
            tidal_volume_c.AutoZero = fabs(tidal_volume_c.InspVolumeVenturi) / fabs(tidal_volume_c.ExpVolumeVenturi);

        Serial.println("valore=" + String(tidal_volume_c.InspVolumeVenturi) + "," + String(tidal_volume_c.ExpVolumeVenturi) + "," + String(tidal_volume_c.AutoZero));
    
*/    }

    if (strPatam == "stats") {
        /*if (__stat_param.mean_cnt > 0) {
            float overshoot_avg = __stat_param.overshoot_avg / __stat_param.mean_cnt;
            float overshoot_length_avg = __stat_param.overshoot_length_avg / __stat_param.mean_cnt;
            float final_error_avg = __stat_param.final_error_avg / __stat_param.mean_cnt;
            float t1050_avg = __stat_param.t1050_avg / __stat_param.mean_cnt;
            float t1090_avg = __stat_param.t1090_avg / __stat_param.mean_cnt;
            float tpeak_avg = __stat_param.tpeak_avg / __stat_param.mean_cnt;
            float t9010_avg = __stat_param.t9010_avg / __stat_param.mean_cnt;
            float t9050_avg = __stat_param.t9050_avg / __stat_param.mean_cnt;
            float peep_avg = __stat_param.peep_avg / __stat_param.mean_cnt;
            float t10_avg = __stat_param.t10_avg / __stat_param.mean_cnt;
            float time_to_peak_avg = __stat_param.time_to_peak_avg / __stat_param.mean_cnt;
            float flux_peak_avg = __stat_param.flux_peak_avg / __stat_param.mean_cnt;
            float flux_t1090_avg = __stat_param.flux_t1090_avg / __stat_param.mean_cnt;
            float flux_t9010_avg = __stat_param.flux_t9010_avg / __stat_param.mean_cnt;

            Serial.println("valore=overshoot_avg:" + String(overshoot_avg)
                + ",overshoot_length_avg:" + String(overshoot_length_avg)
                + ",final_error:" + String(final_error_avg)
                + ",t1050_avg:" + String(t1050_avg)
                + ",t1090_avg:" + String(t1090_avg)
                + ",tpeak_avg:" + String(tpeak_avg)
                + ",t9010_avg:" + String(t9010_avg)
                + ",t9050_avg:" + String(t9050_avg)
                + ",peep_avg:" + String(peep_avg)
                + ",t10_avg:" + String(t10_avg)
                + ",time_to_peak_avg:" + String(time_to_peak_avg)
                + ",flux_peak_avg:" + String(flux_peak_avg)
                + ",flux_t1090_avg:" + String(flux_t1090_avg)
                + ",flux_t9010_avg:" + String(flux_t9010_avg));
        }
        else {
            Serial.println("valore=no_data");
        }*/
    }

    if (strPatam == "get_fp") {
        return  "valore=" + String(sys_s->FlowIn) + "," + String(sys_s->VenturiP);
    }

    if (strPatam == "venturi_scan") {
        ((MVMCore*)core)->DOVenturiMeterScan();
        return  "valore=OK";
    }

    return "valore=ERROR:Invalid Command Argument";
}

uint32_t ConfigManagerClass::GenerateFlag(int alarm_code)
{
    return (1 << alarm_code);
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
