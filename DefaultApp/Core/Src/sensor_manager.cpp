#include "sensor_manager.hpp"
#include "common_variables.hpp"

/* Note:  HAL_SPI_Transmit or same function only can be used at cpp but not header file hpp)
 *
 */
HAL_StatusTypeDef SPISensor::spiTransmit(uint8_t* data, uint16_t size, uint32_t timeout) {
	selectSensor();
	HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, data, size, timeout);
	deselectSensor();
	return status;
}

HAL_StatusTypeDef SPISensor::spiReceive(uint8_t* data, uint16_t size, uint32_t timeout) {
	selectSensor();
	HAL_StatusTypeDef status = HAL_SPI_Receive(hspi, data, size, timeout);
	deselectSensor();
	return status;
}

HAL_StatusTypeDef SPISensor::spiTransmitReceive(uint8_t* txData, uint8_t* rxData, uint16_t size, uint32_t timeout) {
	selectSensor();
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, txData, rxData, size, timeout);
	deselectSensor();
	return status;
}

bool TemperatureSensor::init() {
     // Initialize temperature sensor
     uint8_t config = 0x01;
     if (spiTransmit(&config, 1) == HAL_OK) {
         isActive = true;
         SystemLogger::getInstance()->log(LogLevel::info, "Temperature sensor initialized", "TEMP_SENSOR");
         return true;
     }
     SystemLogger::getInstance()->log(LogLevel::error, "Temperature sensor initialization failed", "TEMP_SENSOR");
     return false;
 }

 SensorData TemperatureSensor::readData() {
     if (!isActive) return SensorData();

     uint8_t data[2];
     if (spiReceive(data, 2) == HAL_OK) {
         float temperature = ((data[0] << 8) | data[1]) * 0.0625f; // Example conversion
         lastReadTime = HAL_GetTick();
         return SensorData(SensorType::TEMPERATURE, lastReadTime, temperature, sensorId);
     }

     SystemLogger::getInstance()->log(LogLevel::error, "Temperature sensor read failed", "TEMP_SENSOR");
     return SensorData();
 }

 bool TemperatureSensor::selfTest() {
     // Implement self-test logic
     return isActive;
 }

 void TemperatureSensor::reset() {
     isActive = false;
     init();
 }

SensorManager::SensorManager(SPI_HandleTypeDef* spi)
    : hspi(spi), readInterval(1000), isRunning(false) {

	osMessageQDef(sensorDataQueueDef, SENSOR_DATA_QUEUE_SIZE, uint32_t);
	sensorDataQueue = osMessageCreate(osMessageQ(sensorDataQueueDef), NULL);

	osSemaphoreDef(sensoMutexDef);
	sensorMutex = osSemaphoreCreate(osSemaphore(sensoMutexDef), 1);

}

SensorManager::~SensorManager() {
    stop();
    osMessageDelete(sensorDataQueue);
    osSemaphoreRelease(sensorMutex);
}

void StartSensorTask(void const *argument);
void SensorManager::init() {
    // Initialize all sensors
    for (auto& sensor : sensors) {
        sensor->init();
    }

    // Create sensor timer
    sensorTimer = xTimerCreate("SensorTimer", pdMS_TO_TICKS(readInterval),
                              pdTRUE, this, sensorTimerCallback);

    // Create sensor task
    osThreadDef(sensoTaskDef, StartSensorTask, osPriorityNormal, 1, 512);
    sensorTaskId = osThreadCreate(osThread(sensoTaskDef), nullptr);

    SystemLogger::getInstance()->log(LogLevel::info, "Sensor Manager initialized", "SENSOR_MGR");
}

void SensorManager::start() {
    isRunning = true;
    xTimerStart(sensorTimer, 0);
    SystemLogger::getInstance()->log(LogLevel::info, "Sensor Manager started", "SENSOR_MGR");
}

void SensorManager::stop() {
    isRunning = false;
    xTimerStop(sensorTimer, 0);
    SystemLogger::getInstance()->log(LogLevel::info,"Sensor Manager stopped", "SENSOR_MGR");
}

void SensorManager::addSensor(std::unique_ptr<ISensor> sensor) {
    if (xSemaphoreTake(sensorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        sensors.push_back(std::move(sensor));
        xSemaphoreGive(sensorMutex);
        SystemLogger::getInstance()->log(LogLevel::info, "Sensor added", "SENSOR_MGR");
    }
}

void SensorManager::sensorTask(void* parameter) {
    SensorManager* manager = static_cast<SensorManager*>(parameter);

    while (true) {
        if (manager->isRunning) {
            manager->processSensorData();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void SensorManager::sensorTimerCallback(TimerHandle_t xTimer) {
    SensorManager* manager = static_cast<SensorManager*>(pvTimerGetTimerID(xTimer));

    if (xSemaphoreTake(manager->sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (auto& sensor : manager->sensors) {
            if (sensor->getActive()) {
                SensorData data = sensor->readData();
                if (data.isValid) {
                    SensorData* dataPtr = new SensorData(data);  // cấp phát vùng nhớ để truyền vào queue
                    osMessagePut(manager->sensorDataQueue, (uint32_t)dataPtr, 0);
                    manager->notifyObservers(data);
                }
            }
        }
        xSemaphoreGive(manager->sensorMutex);
    }
}

void SensorManager::processSensorData() {
    osEvent evt = osMessageGet(sensorDataQueue, 0);
    if (evt.status == osEventMessage) {
        // Process sensor data
    	SensorData* data = (SensorData*)evt.value.p;
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Sensor %d: %d", data->sensorId, data->value);
        SystemLogger::getInstance()->log(LogLevel::debug, buffer, "SENSOR_DATA");
    }
}

std::vector<SensorData> SensorManager::getAllSensorData() {
    std::vector<SensorData> allData;

    if (osSemaphoreWait(sensorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        for (auto& sensor : sensors) {
            if (sensor->getActive()) {
                SensorData data = sensor->readData();
                if (data.isValid) {
                    allData.push_back(data);
                }
            }
        }
        osSemaphoreRelease(sensorMutex);
    }

    return allData;
}

uint32_t SensorManager::getActiveSensorCount() {
    uint32_t count = 0;

    if (osSemaphoreWait(sensorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        for (auto& sensor : sensors) {
            if (sensor->getActive()) {
                count++;
            }
        }
        osSemaphoreRelease(sensorMutex);
    }

    return count;
}

bool SensorManager::performSelfTest() {
    bool allPassed = true;

    if (osSemaphoreWait(sensorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        for (auto& sensor : sensors) {
            if (!sensor->selfTest()) {
                allPassed = false;
                SystemLogger::getInstance()->log(LogLevel::error,"Sensor self-test failed", "SENSOR_MGR");
            }
        }
        osSemaphoreRelease(sensorMutex);
    }

    return allPassed;
}

void SensorManager::resetAllSensors() {
    if (osSemaphoreWait(sensorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        for (auto& sensor : sensors) {
            sensor->reset();
        }
        osSemaphoreRelease(sensorMutex);
        SystemLogger::getInstance()->log(LogLevel::info, "All sensors reset", "SENSOR_MGR");
    }
}

