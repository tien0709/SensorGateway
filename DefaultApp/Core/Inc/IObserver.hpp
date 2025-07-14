#ifndef INC_IOBSERVER_HPP_
#define INC_IOBSERVER_HPP_

template<typename T>
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update(const T& data) = 0;
};

template<typename T>
class Observable {
private:
    std::vector<IObserver<T>*> observers;

public:
    void addObserver(IObserver<T>* observer) {
        observers.push_back(observer);
    }

    void removeObserver(IObserver<T>* observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }

    void notifyObservers(const T& data) {
        for (auto observer : observers) {
            observer->update(data);
        }
    }
};



#endif /* INC_IOBSERVER_HPP_ */
