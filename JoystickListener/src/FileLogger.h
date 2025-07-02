#pragma once
#include "ILogger.h"
#include <fstream>
#include <mutex>

class FileLogger : public ILogger {
public:
    FileLogger(const std::string& filename) : file_(filename, std::ios::app) {}

    void Log(const std::string& message) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (file_.is_open()) {
            file_ << message << std::endl;
        }
    }

private:
    std::ofstream file_;
    std::mutex mutex_;
};