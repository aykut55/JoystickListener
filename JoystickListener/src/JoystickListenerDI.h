#pragma once

#include <windows.h>
#include <dinput.h>

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

class CJoystickListenerDI
{
    const bool UseThrottleButtonAsReversed = true;
    const bool UseThrottleButtonAsRglSlider = true;
public:
    using ButtonHandler = std::function<void(int buttonId, bool pressed)>;
    using ButtonHeldHandler = std::function<void(int buttonId)>;
    using AxisHandler = std::function<void(double x, double y, double z, double rz, double pov, std::string povDir)>;

    ~CJoystickListenerDI();
     CJoystickListenerDI(GUID deviceGuid);

    bool Init(void);
    void Reset(void);
    void Start(void);
    void Stop(void);
    void CalibrateCenter(void);

    void StartListening(void);
    void StopListening(void);

    bool IsRunning(void) const;
    bool IsStopped(void) const;
    bool IsInit(void) const;

    void SetAxisHandler(AxisHandler handler);
    void SetButtonHandler(ButtonHandler handler);
    void SetButtonHeldHandler(ButtonHeldHandler handler);

    void SetLogger(std::shared_ptr<std::ostream> logger);
    void SetSilentMode(bool silentAxis = true, bool silentButton = true, bool silentButtonHeld = true);

    void  SetExternalObject(void* pObject);
    void* GetExternalObject(void);

    void SetNormalize(bool normalize);
    bool GetNormalize(void);

private:
    void ListenLoop(void);

    LPDIRECTINPUT8 m_directInput;
    LPDIRECTINPUTDEVICE8 m_joystickDevice;
    GUID m_deviceGuid;

    std::thread m_thread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;
    std::atomic<bool> m_silentAxis;
    std::atomic<bool> m_silentButton;
    std::atomic<bool> m_silentButtonHeld;

    AxisHandler m_axisHandler;
    ButtonHandler m_buttonHandler;
    ButtonHeldHandler m_buttonHeldHandler;

    DIJOYSTATE2 m_joyStatePrev;

    //std::shared_ptr<ILogger> m_logger;
    std::shared_ptr<std::ostream> m_logger;

    std::string MapPOV(DWORD pov);

    void* m_pExternalObject;

    std::atomic<bool> m_normalize;
};
