#include "myperfmon.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
//#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include <perfmon.h>

static const char *TAG = "perfmon";

static uint64_t idle0Calls;
static uint64_t idle1Calls;

int8_t cpu0 = 0;
int8_t cpu1 = 1;

#if defined(CONFIG_ESP32_DEFAULT_CPU_FREQ_240)
static const uint64_t MaxIdleCalls = 1855000;
#elif defined(CONFIG_ESP32_DEFAULT_CPU_FREQ_160)
static const uint64_t MaxIdleCalls = 1233100;
#else
#error "Unsupported CPU frequency"
#endif

// Table with dedicated performance counters
//  TODO: figure out how to use perfmon
static uint32_t pm_check_table[] = {
    XTPERF_CNT_CYCLES, XTPERF_MASK_CYCLES, // total cycles
    XTPERF_CNT_INSN, XTPERF_MASK_INSN_ALL, // total instructions
    XTPERF_CNT_D_LOAD_U1, XTPERF_MASK_D_LOAD_LOCAL_MEM, // Mem read
    XTPERF_CNT_D_STORE_U1, XTPERF_MASK_D_STORE_LOCAL_MEM, // Mem write
    XTPERF_CNT_BUBBLES, XTPERF_MASK_BUBBLES_ALL &(~XTPERF_MASK_BUBBLES_R_HOLD_REG_DEP),  // wait for other reasons
    XTPERF_CNT_BUBBLES, XTPERF_MASK_BUBBLES_R_HOLD_REG_DEP,           // Wait for register dependency
    XTPERF_CNT_OVERFLOW, XTPERF_MASK_OVERFLOW,               // Last test cycle
};
//

#define TOTAL_CALL_AMOUNT 200
#define PERFMON_TRACELEVEL -1 // -1 - will ignore trace level

bool exec_test_function(){
    int i = 1+1;
    return true;
}

void pukeStats(){
    //
    ESP_LOGI(TAG, "Start");
    ESP_LOGI(TAG, "Start test with printing all available statistic");
    xtensa_perfmon_config_t pm_config = {};
    pm_config.counters_size = sizeof(xtensa_perfmon_select_mask_all) / sizeof(uint32_t) / 2;
    pm_config.select_mask = xtensa_perfmon_select_mask_all;
    pm_config.repeat_count = TOTAL_CALL_AMOUNT;
    pm_config.max_deviation = 1;
    pm_config.call_function = exec_test_function;
    pm_config.callback = xtensa_perfmon_view_cb;
    pm_config.callback_params = stdout;
    pm_config.tracelevel = PERFMON_TRACELEVEL;
    xtensa_perfmon_exec(&pm_config);

    /*
    ESP_LOGI(TAG, "Start test with user defined statistic");
    pm_config.counters_size = sizeof(pm_check_table) / sizeof(uint32_t) / 2;
    pm_config.select_mask = pm_check_table;
    pm_config.repeat_count = TOTAL_CALL_AMOUNT;
    pm_config.max_deviation = 1;
    pm_config.call_function = exec_test_function;
    pm_config.callback = xtensa_perfmon_view_cb;
    pm_config.callback_params = stdout;
    pm_config.tracelevel = PERFMON_TRACELEVEL;

    xtensa_perfmon_exec(&pm_config);
    */

    ESP_LOGI(TAG, "The End");
    //
}

static bool idle_task_0()
{
	idle0Calls += 1;
	return false;
}

static bool idle_task_1()
{
	idle1Calls += 1;
	return false;
}

static void perfmon_task(void *args)
{
	while (1)
	{
		float idle0 = idle0Calls;
		float idle1 = idle1Calls;
		idle0Calls = 0;
		idle1Calls = 0;

		cpu0 = 100.f -  idle0 / MaxIdleCalls * 100.f;
		cpu1 = 100.f - idle1 / MaxIdleCalls * 100.f;

        /*
		ESP_LOGI(TAG, "Core 0 at %d%%", cpu0);
		ESP_LOGI(TAG, "Core 1 at %d%%", cpu1);
        */
        //pukeStats();
		// TODO configurable delay
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

esp_err_t perfmon_start()
{
	ESP_ERROR_CHECK(esp_register_freertos_idle_hook_for_cpu(idle_task_0, 0));
	ESP_ERROR_CHECK(esp_register_freertos_idle_hook_for_cpu(idle_task_1, 1));
	// TODO calculate optimal stack size
	xTaskCreate(perfmon_task, "perfmon", 2048, NULL, 1, NULL);
    return ESP_OK;
}
