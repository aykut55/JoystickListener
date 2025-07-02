#pragma once
#include <string>

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void Log(const std::string& message) = 0;
};