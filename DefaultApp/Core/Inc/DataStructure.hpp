#ifndef INC_DATASTRUCTURE_HPP_
#define INC_DATASTRUCTURE_HPP_


enum class SensorType {
	TEMPERATURE,
	HUMIDITY,
	PRESSURE,
	LIGHT
};

enum class LogLevel {
	debug,
	info,
	warning,
	error
};

enum class SystemState{
    IDLE,
    RUNNING,
    ERROR,
    MAINTENANCE
};

struct SensorData {
	SensorType type;
	uint32_t timestamp;
	float value;
	uint8_t sensorId;
	bool isValid;

	SensorData():type(SensorType::TEMPERATURE), timestamp(0), value(0.0), sensorId(0), isValid(false){}
    SensorData(SensorType t, uint32_t ts, float v, uint8_t id)
        : type(t), timestamp(ts), value(v), sensorId(id), isValid(true) {}
};

struct LogMessage{
	LogLevel level;
	uint32_t timestamp;
	std::string message;
	std::string module;

    LogMessage() : level(LogLevel::INFO), timestamp(0) {}
    LogMessage(LogLevel l, const std::string& msg, const std::string& mod)
        : level(l), timestamp(HAL_GetTick()), message(msg), module(mod) {}
};

struct CLICommand{
	std::string command;
	std::vector<std::string> parameters;
	uint32_t timestamp;

	CLICommand() : timestamp(0) {}
	CLICommand(const std::string& cmd) : command(cmd), timestamp(HAL_GetTick()) {}
};

struct SystemStatus{
	SystemState state;
	uint32_t upTime;
	uint32_t freeHeap;
	uint32_t totalSensor;
	uint32_t activeSensor;
	uint32_t errorCount;
	float cpuUsage;

	 SystemStatus() : state(SystemState::IDLE), uptime(0), freeHeap(0),
	                    totalSensors(0), activeSensors(0), errorCount(0), cpuUsage(0.0f) {}
};



#endif /* INC_DATASTRUCTURE_HPP_ */
