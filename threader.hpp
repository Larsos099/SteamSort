#pragma once
#include <QObject>
#include <functional>
#include <thread>
constexpr int THREAD_JOIN = 0;
constexpr int THREAD_DETACH = 1;
class Threader : public QObject {
    Q_OBJECT
private:
    int threadingMode{};
public:
    Threader(int threadMode, QObject* parent = nullptr) : QObject(parent), threadingMode(threadMode) {}
    template<typename T>
    void run(std::function<T()> func) {
        if(static_cast<bool>(threadingMode)) {
            std::thread([f = std::move(func)]() mutable {
            f();
            }).detach();
        }
        else {
            std::thread([f = std::move(func)]() mutable {
            f();
            }).join();
        }

    }
};
