#ifndef INC_SENSOR_MANAGER_HPP_
#define INC_SENSOR_MANAGER_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal_spi.h"
#ifdef __cplusplus
}
#endif

#include <system_logger.hpp>


class SensorManager{
public:
	void init(SPI_HandleTypeDef* spi1);
	void run();
private:
	SPI_HandleTypeDef* hspi1;
	SystemLogger logger;
};


#endif /* INC_SENSOR_MANAGER_HPP_ */
