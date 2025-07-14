#include <sensor_manager.hpp>
#include<string>

void SensorManager::init(SPI_HandleTypeDef* spi1){
	this->hspi1 = spi1;
	this->logger.init();
}

void SensorManager::run(){
	uint8_t data[1];
	if(HAL_SPI_Receive(hspi1, data, sizeof(data), HAL_MAX_DELAY)!= HAL_OK){
		logger.run("Error: receive CLI command failure!!\n");
	} else {
		std::string output_string = "Received: receive data: " + std::to_string(data) + " from sensor!!\n";
		logger.run(output_string);
	}
}


