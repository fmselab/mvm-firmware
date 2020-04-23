#pragma once



typedef struct
{
    float overshoot_avg;
    float overshoot_length_avg;
    float time_to_peak_avg;
    float final_error_avg;
    float t1050_avg;
    float t1090_avg;
    float tpeak_avg;
    float t9010_avg;
    float t9050_avg;
    float peep_avg;
    float t10_avg;
    float flux_peak_avg;
    float flux_t1090_avg;
    float flux_t9010_avg;

    float time_to_peak;
    float overshoot;
    float final_error;
    float t10a;
    float t50a;
    float t90a;
    float t10b;
    float t50b;
    float t90b;
    float overshoot_length;
    float flux_peak;
    float flux_t10a;
    float flux_t90a;
    float flux_t10b;
    float flux_t90b;

    int mean_cnt;
    uint32_t start_time;
    int phase;

} t_stat_param;

typedef struct
{
    float current_pressure_setpoint=0;
	float pLoop = 0;
	float pPatient = 0;
    float pPatient_low_passed = 0;
	float FlowIn = 0;
	float FlowVenturi = 0;
    

    float last_O2 = 21.7;
    float last_peep = 0;
    float last_bpm = 0;
    float averaged_bpm = 0;
    float GasTemperature = 0;

    bool batteryPowered = false;
    float currentBatteryCharge = 100;

    float currentP_Peak = 0;
    float currentF_Peak = 0;
    float currentTvIsnp = 0;
    float currentTvEsp = 0;
    float currentVM = 0;
    float TidalVolume = 0;
    float VenturiFlux = 0;
    float VenturiP = 0;
    float Flux = 0;


    float PPatient_delta = 0;
    float PPatient_delta2 = 0;
    float PPatient_lowpass = 0;

    int dbg_trigger = 0;
    float dgb_peaktime = 0;
    float fluxpeak = 0;
    float pres_peak = 0;
    
    float dt_veturi_100ms = 0;

    float pid_valvein_slow = 0;
    float pid_valvein_fast = 0;

    t_stat_param __stat_param;

    uint32_t ALARM_FLAG = 0;
    uint32_t WARNING_FLAG = 0;

    bool in_over_pressure_emergency = false;
} t_SystemStatus;