#ifndef INC_DATA_BUFFER_HPP_
#define INC_DATA_BUFFER_HPP_


template<typename T>
class CircularBuffer {
private:
    std::vector<T> buffer;
    size_t head;
    size_t tail;
    size_t count;
    size_t capacity;
    SemaphoreHandle_t bufferMutex;

public:
    CircularBuffer(size_t size) : head(0), tail(0), count(0), capacity(size) {
        buffer.resize(size);
        bufferMutex = xSemaphoreCreateMutex();
    }

    ~CircularBuffer() {
        vSemaphoreDelete(bufferMutex);
    }

    bool push(const T& item) {
        if (xSemaphoreTake(bufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            buffer[head] = item;
            head = (head + 1) % capacity;

            if (count < capacity) {
                count++;
            } else {
                tail = (tail + 1) % capacity; // Overwrite oldest
            }

            xSemaphoreGive(bufferMutex);
            return true;
        }
        return false;
    }

    bool pop(T& item) {
        if (xSemaphoreTake(bufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (count > 0) {
                item = buffer[tail];
                tail = (tail + 1) % capacity;
                count--;
                xSemaphoreGive(bufferMutex);
                return true;
            }
            xSemaphoreGive(bufferMutex);
        }
        return false;
    }

    size_t size() const {
        return count;
    }

    bool empty() const {
        return count == 0;
    }

    bool full() const {
        return count == capacity;
    }

    std::vector<T> getAll() {
        std::vector<T> result;
        if (xSemaphoreTake(bufferMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            result.reserve(count);
            size_t index = tail;
            for (size_t i = 0; i < count; i++) {
                result.push_back(buffer[index]);
                index = (index + 1) % capacity;
            }
            xSemaphoreGive(bufferMutex);
        }
        return result;
    }
};

class DataStorage {
private:
    CircularBuffer<SensorData> sensorDataBuffer;
    CircularBuffer<LogMessage> logBuffer;
    SemaphoreHandle_t storageMutex;

    static const size_t SENSOR_BUFFER_SIZE = 1000;
    static const size_t LOG_BUFFER_SIZE = 500;

public:
    DataStorage() : sensorDataBuffer(SENSOR_BUFFER_SIZE), logBuffer(LOG_BUFFER_SIZE) {
        storageMutex = xSemaphoreCreateMutex();
    }

    ~DataStorage() {
        vSemaphoreDelete(storageMutex);
    }

    void storeSensorData(const SensorData& data) {
        sensorDataBuffer.push(data);
    }

    void storeLogMessage(const LogMessage& msg) {
        logBuffer.push(msg);
    }

    std::vector<SensorData> getSensorHistory(uint32_t maxEntries = 0) {
        std::vector<SensorData> history = sensorDataBuffer.getAll();
        if (maxEntries > 0 && history.size() > maxEntries) {
            history.erase(history.begin(), history.end() - maxEntries);
        }
        return history;
    }

    std::vector<LogMessage> getLogHistory(uint32_t maxEntries = 0) {
        std::vector<LogMessage> history = logBuffer.getAll();
        if (maxEntries > 0 && history.size() > maxEntries) {
            history.erase(history.begin(), history.end() - maxEntries);
        }
        return history;
    }

    void clearSensorHistory() {
        // Clear by creating new buffer
        sensorDataBuffer = CircularBuffer<SensorData>(SENSOR_BUFFER_SIZE);
    }

    void clearLogHistory() {
        logBuffer = CircularBuffer<LogMessage>(LOG_BUFFER_SIZE);
    }
};



#endif /* INC_DATA_BUFFER_HPP_ */
