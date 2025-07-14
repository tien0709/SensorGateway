#ifndef INC_CLI_MANAGER_HPP_
#define INC_CLI_MANAGER_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal_uart.h"
#ifdef __cplusplus
}
#endif

#include <system_logger.hpp>

class CLIManager {
public:
	void init(HAL_UART_StateTypeDef* uart);
	void run();
private:
	HAL_UART_StateTypeDef* huart;
	void processCommand(char* cmd);
	SystemLogger logger;
};



#endif /* INC_CLI_MANAGER_HPP_ */
