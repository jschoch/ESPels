#ifndef COMPONENTS_PERFMON_INCLUDE_PERFMON_H_
#define COMPONENTS_PERFMON_INCLUDE_PERFMON_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

extern int8_t cpu0;
extern int8_t cpu1;

esp_err_t perfmon_start();

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_PERFMON_INCLUDE_PERFMON_H_ */














