#pragma once

#include <windows.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include <map>
#include <memory>

#include "ILogger.h"

class CJoystickListener
{
    const bool UseThrottleButtonAsReversed = true;
public:
    using ButtonHandler = std::function<void(int buttonId, bool pressed)>;
    using ButtonHeldHandler = std::function<void(int buttonId)>;
    using AxisHandler = std::function<void(int x, int y, int z, int pov, std::string povDir)>;

    ~CJoystickListener();
     CJoystickListener(UINT joystickId = 0);

    bool Init();
    void Reset();
    void Start();
    void Stop();
    void CalibrateCenter();

    void StartListening();
    void StopListening();

    bool IsRunning() const;
    bool IsStopped() const;
    bool IsInit() const;

    void SetAxisHandler(AxisHandler handler);
    void SetButtonHandler(ButtonHandler handler);
    void SetButtonHeldHandler(ButtonHeldHandler handler);

    void SetLogger(std::shared_ptr<std::ostream> logger);
    void SetSilentMode(bool silentAxis = true, bool silentButton = true, bool silentButtonHeld = true);

    void  SetExternalObject(void* pObject);
    void* GetExternalObject(void);

private:
    void ListenLoop();

    UINT m_joystickId;
    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;
    std::atomic<bool> m_silentAxis;
    std::atomic<bool> m_silentButton;
    std::atomic<bool> m_silentButtonHeld;

    AxisHandler m_axisHandler;
    ButtonHandler m_buttonHandler;
    ButtonHeldHandler m_buttonHeldHandler;

    JOYINFOEX m_joyInfoPrev;

    //std::shared_ptr<ILogger> m_logger;
    std::shared_ptr<std::ostream> m_logger;

    std::string MapPOV(DWORD pov);

    void* m_pExternalObject;
};