#include <system_logger.hpp>


void SystemLogger::loggerTask(void* parameter){
   // Logger* logger = static_cast<Logger*>(parameter);
	LogMessage msg;

	while (true) {
		osEvent evt = osMessageGet(this->logQueue, osWaitForever);
		if (evt.status == osEventMessage) {
			processLogMessage(evt.value.p);
		}
	}
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

SystemLogger::SystemLogger(TaskHandle_t loggerTaskHandle, UART_HandleTypeDef* huart){
	//initial Task
	this->loggerTaskHandle = loggerTaskHandle;

	//initial Uart handle
	this->huart = huart;

	//initial Mesage queue handle
	osMessageQDef(logQueueDef, 10, char*);
	this->logQueue = osMessageCreate(osMessageQ(logQueueDef), NULL);

	//initial binary semaphore handle
	osSemaphoreDef(logMutexDef);
	this->logMutex = osSemaphoreCreate(osSemaphore(logMutexDef), 1);
}
//	static SystemLogger::Logger* getInstance(){
//		if(this->instance == NULL){
//			instance = new Logger();
//		} return instance;
//	}
void SystemLogger::init(UART_HandleTypeDef* uart, TaskHandle_t loggerTaskHandle){
	this->huart = uart;
	this->loggerTaskHandle = loggerTaskHandle;
}
void SystemLogger::log(LogLevel level, const std::string& message, const std::string& module){
	LogMessage mgs(level, message, module);
}



