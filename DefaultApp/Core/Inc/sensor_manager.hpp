#ifndef INC_SENSOR_MANAGER_HPP_
#define INC_SENSOR_MANAGER_HPP_


#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal.h"
#ifdef __cplusplus
}
#endif

#include "DataStructure.hpp"
#include "system_logger.hpp"
#include "IObserver.hpp"
#include<memory>

class ISensor{
	protected:
		uint8_t sensorId;
		SensorType type;
		bool isActive;
		uint32_t lastReadTime;
	public:
		ISensor(uint8_t id, SensorType t) : sensorId(id), type(t), isActive(false), lastReadTime(0) {}
		virtual ~ISensor() = default;

		virtual bool init() = 0;
		virtual SensorData readData() = 0;
		virtual bool selfTest() = 0;
		virtual void reset() = 0;

		uint8_t getId() const { return sensorId; }
		SensorType getType() const { return type; }
		bool getActive() const { return isActive; }
		uint32_t getLastReadTime() const { return lastReadTime; }
};

class SPISensor: public ISensor{
protected:
	SPI_HandleTypeDef* hspi;
	UART_HandleTypeDef* huart;
	GPIO_TypeDef* csPort;
	uint8_t csPin;
public:
    SPISensor(uint8_t id, SensorType type, SPI_HandleTypeDef* spi,
              GPIO_TypeDef* port, uint16_t pin)
        : ISensor(id, type), hspi(spi), csPort(port), csPin(pin) {}
protected:
	void selectSensor() { HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET); }
	void deselectSensor() { HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET); }

	HAL_StatusTypeDef spiTransmit(uint8_t* data, uint16_t size, uint32_t timeout = 1000) ;

	HAL_StatusTypeDef spiReceive(uint8_t* data, uint16_t size, uint32_t timeout = 1000) ;

	HAL_StatusTypeDef spiTransmitReceive(uint8_t* txData, uint8_t* rxData, uint16_t size, uint32_t timeout = 1000) ;
};

class TemperatureSensor : public SPISensor {
public:
    TemperatureSensor(uint8_t id, SPI_HandleTypeDef* spi, GPIO_TypeDef* port, uint16_t pin)
        : SPISensor(id, SensorType::TEMPERATURE, spi, port, pin) {}

    bool init() override ;

    SensorData readData() override ;

    bool selfTest() override ;

    void reset() override;
};

class SensorManager: public Observable<SensorData>{

    std::vector<std::unique_ptr<ISensor>> sensors;
    osMessageQId sensorDataQueue;
    osSemaphoreId sensorMutex;
    osThreadId sensorTaskId;
    TimerHandle_t sensorTimer;
    SPI_HandleTypeDef* hspi;
    uint32_t readInterval;
    bool isRunning;

    static void sensorTask(void* parameter);
    static void sensorTimerCallback(TimerHandle_t xTimer);
    void processSensorData();

public:
    SensorManager(SPI_HandleTypeDef* spi);
    ~SensorManager();

    void init();
    void start();
    void stop();
    void addSensor(std::unique_ptr<ISensor> sensor);
    void removeSensor(uint8_t sensorId);
    std::vector<SensorData> getAllSensorData();
    SensorData getSensorData(uint8_t sensorId);
    void setReadInterval(uint32_t interval);
    uint32_t getActiveSensorCount();
    bool performSelfTest();
    void resetAllSensors();

    QueueHandle_t getSensorDataQueue() const { return sensorDataQueue; }
};


#endif /* INC_SENSOR_MANAGER_HPP_ */
