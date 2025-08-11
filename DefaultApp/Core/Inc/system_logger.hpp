#ifndef INC_SYSTEM_LOGGER_HPP_
#define INC_SYSTEM_LOGGER_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal.h"
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
	static SystemLogger* instance;//singleton parten=>> assure only one SystemLogger existing in system
	//and can access from everywhere
	osThreadId loggerTaskHandle;

	static void loggerTask(const void* parameter);//must be static for task of thread
	void processLogMessage(const LogMessage& message);
	std::string formatLogMessage(const LogMessage& msg);
public:
	SystemLogger();
	SystemLogger(UART_HandleTypeDef* huart);
	static SystemLogger* getInstance();
	void init(UART_HandleTypeDef* uart);
	void log(LogLevel level, const std::string& message, const std::string& module = "SYSTEM");
};


#endif /* INC_SYSTEM_LOGGER_HPP_ */
