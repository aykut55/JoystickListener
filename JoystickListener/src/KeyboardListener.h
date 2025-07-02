#pragma once

#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <functional>
#include <unordered_map>

#include "ILogger.h"
#include "KeyEvent.h"
#include "KeyHistory.h"

class CKeyboardListener {
public:
    CKeyboardListener();
    ~CKeyboardListener();

    void Init();
    void Reset();
    void Start();
    void Stop();
    void SetLogger(std::shared_ptr<ILogger> logger);
    void RegisterHandler(int vk, std::function<void(const KeyEvent&)> handler);

    void ClearKeyHistory();
    const std::unordered_map<int, KeyHistory> GetKeyHistory() const;
    std::unordered_map<int, KeyHistory> GetKeyHistoryCopy() const;

    bool IsRunning() const;
    bool IsStopped() const;
    bool IsInit() const;
    void SetSilentMode(bool silentMode);

private:
    void ListenLoop();

    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;
    std::shared_ptr<ILogger> m_logger;
    std::unordered_map<int, std::vector<std::function<void(const KeyEvent&)>>> m_handlers;
    std::unordered_map<int, std::function<void(const KeyEvent&)>> m_handlers2;
    std::unordered_map<int, KeyHistory> m_keyHistory;
    bool m_silentMode;
};