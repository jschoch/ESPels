#pragma once



// this seems like a horrible hack
#ifdef UNIT_TEST

// if unit testing....
// don't include, this will get included from 'mocks.h'

#else

#include "config.h"
#include <driver/rmt.h>
#include <esp_log.h>


namespace rmtStepper {
    struct Config{
        gpio_num_t stepPin;
        gpio_num_t dirPin;
        rmt_channel_t channel;
        bool invert_step_pin = false;
    };

    

    struct State {
        volatile bool dir = true;
        int pos = 0;
        Config config;
        rmt_item32_t items[2];
        rmt_config_t rconfig;
        int64_t dir_change_timer;
        bool dir_has_changed = false;
        bool olddir = false;

        void step(){
            RMT.conf_ch[config.channel].conf1.mem_rd_rst = 1;
            RMT.conf_ch[config.channel].conf1.tx_start   = 1;
            if(dir){
                pos++;
            }else{
                pos--;
            }
        }
        bool setDir(bool newdir){
            //ESP_LOGE("thing","setDir called current dir: %d  arg dir: %d",dir,newdir);
            olddir = dir;
            if(dir != newdir){


                // TODO need  to be able to invert
                //digitalWrite(config.dirPin, newdir ^ config.invert_step_pin);
#ifndef useFAS
                // XOR the dir for the inversion bool
                dir_has_changed = true;
                digitalWrite(config.dirPin, newdir);
                dir_change_timer = esp_timer_get_time();
#endif
                dir = newdir;
            }
            return olddir == newdir;
        }
        // return true means we changed it and need to wait
        bool setDirNow(bool newdir){
            if(dir == newdir){
                return false;
            }else{

                digitalWrite(config.dirPin, newdir);
                olddir = dir;
                dir = newdir;
                return true;
            }
        }

        void init(){
            rconfig.rmt_mode = RMT_MODE_TX;
            rconfig.channel = config.channel;  // 
            rconfig.gpio_num = config.stepPin;
            rconfig.mem_block_num = 2;  //???
            rconfig.tx_config.loop_en = false;
            rconfig.tx_config.carrier_en = false;
            rconfig.tx_config.carrier_freq_hz      = 0;
            rconfig.tx_config.idle_output_en = true;
            rconfig.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
            rconfig.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
            rconfig.clk_div = 20; //// 80MHx / 80 = 1MHz 0r 1uS per count
            rconfig.tx_config.carrier_duty_percent = 50;
            rmt_set_source_clk(rconfig.channel, RMT_BASECLK_APB);
            rconfig.tx_config.idle_level = config.invert_step_pin ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
            
            items[0].duration0 = 1;
            items[0].duration1 = 4 * 3;  // this is a setting pulse_microseconds->get();
            
            items[1].duration0 = 0;
            items[1].duration1 = 0;  
            items[0].level0              = rconfig.tx_config.idle_level;
            items[0].level1              = !rconfig.tx_config.idle_level;

            rmt_config(&rconfig);
            rmt_fill_tx_items(rconfig.channel, &items[0], rconfig.mem_block_num, 0);
            digitalWrite(config.dirPin,dir);
        }
    };
}
#endif
