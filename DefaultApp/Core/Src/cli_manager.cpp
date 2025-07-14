#include <cli_manager.hpp>

void CLIManager::run(){
	uint8_t rx_data[8];
	if(HAL_UART_Receive(huart, rx_data, sizeof(rx_data),HAL_MAX_DELAY) != HAL_OK){
		logger.run("Error: receive CLI command failure!!\n");
	}
	processCommand((char*)rx_data);
}

void CLIManager::init(HAL_UART_StateTypeDef* uart){
	this->huart = uart;
	this->logger.init();
}

void CLIManager::processCommand(char* cmd){
    if (strcmp(cmd, "status") == 0) {
    	char* tx_data = "System OK! \n\0";//sizeof shouldnt use with pointer behave of array
    	if(HAL_UART_Transmit(huart, (uint8_t*)tx_data, 12, HAL_MAX_DELAY) != HAL_OK){
    		logger.run("Error: send status to CLI failure!!\n");
    	}
    }  else if (strcmp(cmd, "reset") == 0) {
        NVIC_SystemReset();
		logger.run("Reset: Reset System\n");
    } else {
    	logger.run("Warning: Unknown command\n");
    }
}

