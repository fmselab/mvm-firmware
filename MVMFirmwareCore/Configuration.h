#pragma once


typedef enum {
    M_BREATH_FORCED,
    M_BREATH_ASSISTED
} t_assist_mode;

typedef struct
{
    bool run;
    
    uint16_t inhale_ms;
    uint16_t exhale_ms;

    float assist_pressure_delta_trigger;
    t_assist_mode BreathMode;

    float flux_close;
    float target_pressure;
    float target_pressure_auto;
    float target_pressure_assist;
    float respiratory_rate;
    float respiratory_ratio;

    float P;
    float I;
    float D;

    float P2;
    float I2;
    float D2;

    bool pause_inhale;
    bool pause_exhale;

    float pid_limit;

    bool backup_enable;
    float backup_min_rate;

    float pause_lg_timer;
    float pause_lg_p;
    bool pause_lg;

    bool __CONSOLE_MODE = false;
    bool __ADDTimeStamp = false;
    bool __WDENABLE=false;

    bool pcv_trigger_enable=true;
    float pcv_trigger=4;

    float pause_timeout;
    
} t_config;


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
