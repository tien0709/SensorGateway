/*
 * system_monitor.h
 *
 *  Created on: Jul 15, 2025
 *      Author: acer
 */

#ifndef INC_SYSTEM_MONITOR_HPP_
#define INC_SYSTEM_MONITOR_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include<stdint.h>
#include "cmsis_os.h"
#ifdef __cplusplus
}
#endif

#include "sensor_manager.hpp"
#include "cli_manager.hpp"
#include "system_logger.hpp"
#include<string>

class SystemMonitor {
private:
    osThreadId watchdogTaskHandle;
    osMutexId systemMutex;
    osTimerId watchdogTimer;// monitor status of system, reset system when failing

    SensorManager* sensorManager;
    CLIManager* cliManager;
    SystemLogger* logger;

    uint32_t errorCount;
    uint32_t lastHeartbeat;

    bool systemHealthy;//status of system

    static void watchdogTask(const void* parameter);
    static void watchdogTimerCallback(const void* parameter);

    void checkSystemHealth();
    void handleSystemError();
    void resetSystem();

public:
    SystemMonitor(SensorManager* sensorMgr, CLIManager* cliMgr);
    ~SystemMonitor();

    void init();
    void start();
    void stop();
    void heartbeat();
    void reportError(const std::string& error);
    bool isSystemHealthy() const { return systemHealthy; }
    uint32_t getErrorCount() const { return errorCount; }
};



#endif /* INC_SYSTEM_MONITOR_HPP_ */
