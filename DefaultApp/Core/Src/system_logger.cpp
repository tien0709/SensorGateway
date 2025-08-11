#include "system_logger.hpp"

SystemLogger::SystemLogger(){

	instance = nullptr;

	//initial Uart handle
	this->huart = nullptr;


	//initial Message queue handle
	osMessageQDef(logQueueDef, 10, char*);
	this->logQueue = osMessageCreate(osMessageQ(logQueueDef), NULL);

	//initial binary semaphore handle
	osSemaphoreDef(logMutexDef);
	this->logMutex = osSemaphoreCreate(osSemaphore(logMutexDef), 1);
}

SystemLogger* SystemLogger:: getInstance(){
	if(instance == NULL){
		instance = new SystemLogger();
	} return instance;
}


void SystemLogger::loggerTask(const void* parameter){
    SystemLogger* logger = static_cast<SystemLogger*>(const_cast<void*>(parameter));
//	LogMessage msg;

	while (true) {
		osEvent evt = osMessageGet(logger->logQueue, osWaitForever);
		if (evt.status == osEventMessage) {
			logger->processLogMessage(*((LogMessage*)evt.value.p));
		}
	}
}

void SystemLogger::init(UART_HandleTypeDef* uart){
	this->huart = uart;

	osThreadDef(loggerThreadDef, loggerTask, osPriorityNormal, 0, 128);
	this->loggerTaskHandle = osThreadCreate(osThread(loggerThreadDef), this);

}


void SystemLogger::processLogMessage(const LogMessage& message){
	if(huart == NULL) return;
	if(osSemaphoreWait(this->logMutex, pdMS_TO_TICKS(1000)) == osOK){
		std::string formattedMsg = formatLogMessage(message);
        HAL_UART_Transmit(huart,
                          (uint8_t*)formattedMsg.c_str(),
                          formattedMsg.length(),
                          HAL_MAX_DELAY);
		osSemaphoreRelease(this->logMutex);
	}
}

std::string SystemLogger::formatLogMessage(const LogMessage& msg){
    std::string levelStr;
    switch (msg.level) {
        case LogLevel::debug:    levelStr = "DEBUG"; break;
        case LogLevel::info:     levelStr = "INFO"; break;
        case LogLevel::warning:  levelStr = "WARN"; break;
        case LogLevel::error:    levelStr = "ERROR"; break;
        default: break;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "[%lu] [%s] [%s] %s\r\n",
             msg.timestamp, levelStr.c_str(), msg.module.c_str(), msg.message.c_str());
    return std::string(buffer);
}

void SystemLogger::log(LogLevel level, const std::string& message, const std::string& module){
	LogMessage mgs(level, message, module);

	osStatus status = osMessagePut(this->logQueue, (uint32_t)&mgs, osWaitForever);
	if (status != osOK) {
	    // assuming queue always not full
	}
}



