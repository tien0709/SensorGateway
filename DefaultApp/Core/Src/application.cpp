#include "Application.hpp"

Application::Application(SPI_HandleTypeDef* spi, UART_HandleTypeDef* uartCLI, UART_HandleTypeDef* uartLog)
    : hspi(spi), huartCLI(uartCLI), huartLog(uartLog), isInitialized(false), isRunning(false) {
}

Application::~Application() {
    stop();
}

void Application::init() {
    if (isInitialized) return;

    initializeHardware();
    initializeComponents();
    registerSensors();

    isInitialized = true;
}

void Application::initializeHardware() {
    // Hardware initialization would be done in main.c
    // This is just a placeholder for any additional setup
}

void Application::initializeComponents() {
    // Create logger first
    logger = std::unique_ptr<SystemLogger>(SystemLogger::getInstance());
    logger->init(huartLog);

    // Create configuration manager
    configManager = std::make_unique<ConfigManager>();
    configManager->init();

    // Create data storage
    dataStorage = std::make_unique<DataStorage>();

    // Create sensor manager
    sensorManager = std::make_unique<SensorManager>(hspi);
    sensorManager->init();

    // Create CLI manager
    cliManager = std::make_unique<CLIManager>(huartCLI, sensorManager.get());
    cliManager->init();

    // Create system monitor
    systemMonitor = std::make_unique<SystemMonitor>(sensorManager.get(), cliManager.get());
    systemMonitor->init();

    // Set up observer relationships
    //Design Pattern Observer.
    sensorManager->addObserver(cliManager.get());//climanger pointer receive inform when sensor manager have a changing

    logger->log(LogLevel::info, "Application components initialized", "APP");
}

void Application::registerSensors() {
    // Register temperature sensors
	//auto type
    auto tempSensor1 = std::make_unique<TemperatureSensor>(1, hspi, GPIOA, GPIO_PIN_4);
    auto tempSensor2 = std::make_unique<TemperatureSensor>(2, hspi, GPIOA, GPIO_PIN_5);

    sensorManager->addSensor(std::move(tempSensor1));
    sensorManager->addSensor(std::move(tempSensor2));

    logger->log(LogLevel::info, "Sensors registered", "APP");
}

void Application::run() {
    if (!isInitialized) {
        init();
    }

    startComponents();
    isRunning = true;

    logger->log(LogLevel::info, "Application started", "APP");

    // Main application loop runs in FreeRTOS tasks
    // This function returns immediately
}

void Application::startComponents() {
    sensorManager->start();
    systemMonitor->start();

    // Set system configuration
    const SystemConfig& config = configManager->getConfig();
    sensorManager->setReadInterval(config.sensorReadInterval);
}

void Application::stop() {
    if (!isRunning) return;

    logger->log(LogLevel::info, "Stopping application", "APP");

    systemMonitor->stop();
    sensorManager->stop();

    isRunning = false;
}

void Application::handleUARTInterrupt(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size) {
    if (huart == huartCLI) {
        cliManager->handleUARTData(data, size);
    }
}

void Application::handleSPIInterrupt(SPI_HandleTypeDef* hspi) {
    // Handle SPI interrupts if needed
}


