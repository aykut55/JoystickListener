#pragma once
#include "ILogger.h"
#include <iostream>
#include <mutex>

class ConsoleLogger : public ILogger {
public:
    void Log(const std::string& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << message << std::endl;
    }

private:
    std::mutex mutex_;
};