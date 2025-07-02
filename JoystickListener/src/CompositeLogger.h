#pragma once
#include "ILogger.h"
#include <vector>
#include <memory>
#include <mutex>

class CompositeLogger : public ILogger {
public:
    void AddLogger(std::shared_ptr<ILogger> logger) {
        std::lock_guard<std::mutex> lock(mutex_);
        loggers_.push_back(logger);
    }

    void Log(const std::string& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& logger : loggers_) {
            logger->Log(message);
        }
    }

private:
    std::vector<std::shared_ptr<ILogger>> loggers_;
    std::mutex mutex_;
};