#ifndef INC_SYSTEM_LOGGER_HPP_
#define INC_SYSTEM_LOGGER_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal_uart.h"
#include "cmsis_os.h"
#ifdef __cplusplus
}
#endif

#include<string>
#include <string.h>
#include "DataStructure.hpp"


class SystemLogger{
private:
	osSemaphoreId logMutex;
	UART_HandleTypeDef* huart;
	osMessageQId logQueue;
//	static SystemLogger* instance;
	TaskHandle_t loggerTaskHandle;

	void loggerTask(void* parameter);
	void processLogMessage(const LogMessage& message);
	std::string formatLogMessage(const LogMessage& msg);
public:
	SystemLogger(TaskHandle_t loggerTaskHandle, UART_HandleTypeDef* huart);
//	static SystemLogger* getInstance();
	void init(UART_HandleTypeDef* uart, TaskHandle_t loggerTaskHandle);
	void log(LogLevel level, const std::string& message, const std::string& module = "SYSTEM");
};


#endif /* INC_SYSTEM_LOGGER_HPP_ */
