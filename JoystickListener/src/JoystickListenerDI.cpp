#include "JoystickListenerDI.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

CJoystickListenerDI::~CJoystickListenerDI()
{
    StopListening();

    if (m_joystickDevice)
    {
        m_joystickDevice->Unacquire();
        m_joystickDevice->Release();
        m_joystickDevice = nullptr;
    }
    if (m_directInput)
    {
        m_directInput->Release();
        m_directInput = nullptr;
    }
}

CJoystickListenerDI::CJoystickListenerDI(GUID deviceGuid)
    : m_deviceGuid(deviceGuid),
    m_directInput(nullptr),
    m_joystickDevice(nullptr),
    m_running(false),
    m_initialized(false),
    m_silentAxis(true),
    m_silentButton(true),
    m_silentButtonHeld(true),
    m_pExternalObject(nullptr),
    m_normalize(true)
{
    ZeroMemory(&m_joyStatePrev, sizeof(m_joyStatePrev));
}

bool CJoystickListenerDI::Init(void)
{
    HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
        IID_IDirectInput8, (VOID**)&m_directInput, NULL);
    if (FAILED(hr))
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "DirectInput8Create failed.\n";
        return false;
    }

    hr = m_directInput->CreateDevice(m_deviceGuid, &m_joystickDevice, NULL);
    if (FAILED(hr))
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "CreateDevice failed.\n";
        return false;
    }

    hr = m_joystickDevice->SetDataFormat(&c_dfDIJoystick2);
    if (FAILED(hr))
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "SetDataFormat failed.\n";
        return false;
    }

    hr = m_joystickDevice->SetCooperativeLevel(GetConsoleWindow(),
        DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(hr))
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "SetCooperativeLevel failed.\n";
        return false;
    }

    hr = m_joystickDevice->Acquire();
    if (FAILED(hr))
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "Acquire failed.\n";
        return false;
    }

    m_initialized = true;

    if (m_logger && !m_silentButton)
        (*m_logger) << "DirectInput joystick initialized.\n";

    return true;
}

void CJoystickListenerDI::Reset(void)
{
    ZeroMemory(&m_joyStatePrev, sizeof(m_joyStatePrev));
    if (m_logger && !m_silentButton)
        (*m_logger) << "Joystick state reset.\n";
}

void CJoystickListenerDI::Start(void) {
    if (m_initialized && !m_running) {
        m_running = true;
        m_thread = std::thread(&CJoystickListenerDI::ListenLoop, this);
        if (m_logger && !m_silentButton)
            (*m_logger) << "[CJoystickListenerDI] Listening thread started.\n";
    }
}

void CJoystickListenerDI::Stop(void) {
    bool wasRunning = m_running.exchange(false);
    if (m_thread.joinable() && std::this_thread::get_id() != m_thread.get_id())
    {
        m_thread.join();
    }
    if (wasRunning)
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "[CJoystickListenerDI] Listening thread stopped.\n";
    }
    m_running = false;
}

void CJoystickListenerDI::CalibrateCenter(void)
{
    if (!m_initialized) return;

    HRESULT hr = m_joystickDevice->Poll();
    if (FAILED(hr)) return;

    DIJOYSTATE2 state;
    ZeroMemory(&state, sizeof(state));
    hr = m_joystickDevice->GetDeviceState(sizeof(state), &state);
    if (FAILED(hr)) return;

    m_joyStatePrev = state;

    if (m_logger && !m_silentAxis)
        (*m_logger) << "Joystick center calibrated.\n";
}


void CJoystickListenerDI::StartListening(void)
{
    if (!m_initialized || m_running)
        return;

    m_running = true;
    m_thread = std::thread(&CJoystickListenerDI::ListenLoop, this);
}

void CJoystickListenerDI::StopListening(void)
{
    if (!m_running)
        return;

    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

bool CJoystickListenerDI::IsRunning(void) const
{
    return m_running;
}

bool CJoystickListenerDI::IsStopped(void) const
{
    return !m_running;
}

bool CJoystickListenerDI::IsInit(void) const
{
    return m_initialized;
}

void CJoystickListenerDI::SetAxisHandler(AxisHandler handler)
{
    m_axisHandler = handler;
}

void CJoystickListenerDI::SetButtonHandler(ButtonHandler handler)
{
    m_buttonHandler = handler;
}

void CJoystickListenerDI::SetButtonHeldHandler(ButtonHeldHandler handler)
{
    m_buttonHeldHandler = handler;
}

void CJoystickListenerDI::SetLogger(std::shared_ptr<std::ostream> logger)
{
    m_logger = logger;
}

void CJoystickListenerDI::SetSilentMode(bool silentAxis, bool silentButton, bool silentButtonHeld)
{
    m_silentAxis = silentAxis;
    m_silentButton = silentButton;
    m_silentButtonHeld = silentButtonHeld;
}

template<typename T>
T clamp(T val, T minVal, T maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

void CJoystickListenerDI::ListenLoop(void)
{
    DIJOYSTATE2 joyState;
    ZeroMemory(&joyState, sizeof(joyState));

    while (m_running)
    {
        HRESULT hr = m_joystickDevice->Poll();
        if (FAILED(hr))
        {
            hr = m_joystickDevice->Acquire();
            while (hr == DIERR_INPUTLOST)
                hr = m_joystickDevice->Acquire();
            continue;
        }

        hr = m_joystickDevice->GetDeviceState(sizeof(DIJOYSTATE2), &joyState);
        if (FAILED(hr))
        {
            continue;
        }

        // Normalize veya ham 
        double xVal = 0.0, yVal = 0.0, zVal = 0.0, rzVal = 0.0;

        if (m_normalize)
        {
            xVal  = (static_cast<float>(joyState.lX) - 32767.5) / 32767.5;
            yVal  = (static_cast<float>(joyState.lY) - 32767.5) / 32767.5;
            zVal  = (static_cast<float>(joyState.lZ)) / 65535.0;
            if (UseThrottleButtonAsRglSlider)
                zVal  = (static_cast<float>(joyState.rglSlider[0])) / 65535.0;                
            rzVal = (static_cast<float>(joyState.lRz) - 32767.5) / 32767.5;

            xVal  = clamp(xVal, -1.0, 1.0);
            yVal  = clamp(yVal, -1.0, 1.0);
            zVal  = clamp(zVal, 0.0, 1.0);
            rzVal = clamp(rzVal, -1.0, 1.0);
        }
        else
        {

        }

        DWORD powRaw = joyState.rgdwPOV[0];
        std::string povDir = MapPOV(powRaw);

        // button edge
        if (m_buttonHandler)
        {
            for (int i = 0; i < 128; ++i)
            {
                bool prevPressed = (m_joyStatePrev.rgbButtons[i] & 0x80) != 0;
                bool currPressed = (joyState.rgbButtons[i] & 0x80) != 0;

                if (prevPressed != currPressed)
                {
                    m_buttonHandler(i + 1, currPressed);
                    if (m_logger && !m_silentButton)
                    {
                        (*m_logger) << "[Button] " << (i + 1) << (currPressed ? " pressed\n" : " released\n");
                    }
                }
            }
        }

        // button held
        if (m_buttonHeldHandler)
        {
            for (int i = 0; i < 128; ++i)
            {
                bool currPressed = (joyState.rgbButtons[i] & 0x80) != 0;
                if (currPressed)
                {
                    m_buttonHeldHandler(i + 1);
                }
            }
        }

        // axes
        if (m_axisHandler)
        {
            double correctedX = 0;
            double correctedY = 0;
            double correctedZ = 0;
            double correctedRZ = 0;
            double correctedPov = 0;
            std::string povDir = "";
            bool axisChanged = false;

            if (UseThrottleButtonAsRglSlider)
            {
                axisChanged =
                    (m_joyStatePrev.lX != joyState.lX) ||
                    (m_joyStatePrev.lY != joyState.lY) ||
                    (m_joyStatePrev.rglSlider[0] != joyState.rglSlider[0]) ||
                    (m_joyStatePrev.lRz != joyState.lRz) ||
                    (m_joyStatePrev.rgdwPOV[0] != joyState.rgdwPOV[0]);

                if (axisChanged)
                {
                    if (m_normalize)
                    {
                        correctedX   = xVal;
                        correctedY   = yVal;
                        correctedZ   = UseThrottleButtonAsReversed ? (1.0 - zVal) : (zVal);
                        correctedRZ  = rzVal;
                        correctedPov = joyState.rgdwPOV[0] == -1 ? 65535 : joyState.rgdwPOV[0];
                        povDir       = MapPOV(joyState.rgdwPOV[0]);
                    }
                    else
                    {
                        correctedX   = joyState.lX;
                        correctedY   = joyState.lY;
                        correctedZ   = joyState.lZ;
                        correctedZ   = UseThrottleButtonAsReversed ? (65535 - joyState.rglSlider[0]) : (joyState.rglSlider[0]);
                        correctedRZ  = joyState.lRz;
                        correctedPov = joyState.rgdwPOV[0] == -1 ? 65535 : joyState.rgdwPOV[0];
                        povDir       = MapPOV(correctedPov);
                    }

                    if (m_logger && !m_silentAxis)
                    {
                        std::stringstream ss;
                        ss << "[Axis] ";
                        ss << "  X : "      << std::setw(6) << correctedX;
                        ss << "  Y : "      << std::setw(6) << correctedY;
                        ss << "  Z : "      << std::setw(6) << correctedZ;
                        ss << "  RZ : "     << std::setw(6) << correctedRZ;
                        ss << "  Pov : "    << std::setw(6) << correctedPov;
                        ss << "  PovDir : " << std::setw(6) << povDir;
                        (*m_logger) << ss.str() << "\n";
                    }

                    m_axisHandler(correctedX, correctedY, correctedZ, correctedRZ, correctedPov, povDir);
                }
            }
            else
            {
                axisChanged =
                    (m_joyStatePrev.lX != joyState.lX) ||
                    (m_joyStatePrev.lY != joyState.lY) ||
                    (m_joyStatePrev.lZ != joyState.lZ) ||
                    (m_joyStatePrev.lRz != joyState.lRz) ||
                    (m_joyStatePrev.rgdwPOV[0] != joyState.rgdwPOV[0]);

                if (axisChanged)
                {
                    if (m_normalize)
                    {
                        correctedX = xVal;
                        correctedY = yVal;
                        correctedZ = UseThrottleButtonAsReversed ? (1.0 - zVal) : (zVal);
                        correctedRZ = rzVal;
                        correctedPov = joyState.rgdwPOV[0];
                        povDir = MapPOV(joyState.rgdwPOV[0]);
                    }
                    else
                    {
                        correctedX   = joyState.lX;
                        correctedY   = joyState.lY;                    
                        correctedZ   = UseThrottleButtonAsReversed ? (65535 - joyState.lZ) : (joyState.lZ);
                        correctedRZ  = joyState.lRz;
                        correctedPov = joyState.rgdwPOV[0] == -1 ? 65535 : joyState.rgdwPOV[0];
                        povDir       = MapPOV(correctedPov);
                    }

                    if (m_logger && !m_silentAxis)
                    {
                        std::stringstream ss;
                        ss << "[Axis] ";
                        ss << "  X : "      << std::setw(6) << correctedX;
                        ss << "  Y : "      << std::setw(6) << correctedY;
                        ss << "  Z : "      << std::setw(6) << correctedZ;
                        ss << "  RZ : "     << std::setw(6) << correctedRZ;
                        ss << "  Pov : "    << std::setw(6) << correctedPov;
                        ss << "  PovDir : " << std::setw(6) << povDir;
                        (*m_logger) << ss.str() << "\n";
                    }

                    m_axisHandler(correctedX, correctedY, correctedZ, correctedRZ, correctedPov, povDir);
                }

            }
        }

        m_joyStatePrev = joyState;

        Sleep(20);
    }
}

std::string CJoystickListenerDI::MapPOV(DWORD pov)
{
    std::string povName = "Unknown";

    if (pov == 0xFFFF || pov == static_cast<DWORD>(-1))
        povName = "Center";

    if (pov >= 0 && pov < 4500)
        povName = "North";

    if (pov >= 4500 && pov < 9000)
        povName = "North-East";

    if (pov >= 9000 && pov < 13500)
        povName = "East";

    if (pov >= 13500 && pov < 18000)
        povName = "South-East";

    if (pov >= 18000 && pov < 22500)
        povName = "South";

    if (pov >= 22500 && pov < 27000)
        povName = "South-West";

    if (pov >= 27000 && pov < 31500)
        povName = "West";

    if (pov >= 31500 && pov < 35999)
        povName = "North-West";

    return povName;
}

void CJoystickListenerDI::SetExternalObject(void* pObject)
{
    m_pExternalObject = pObject;
}

void* CJoystickListenerDI::GetExternalObject(void)
{
    return m_pExternalObject;
}

void CJoystickListenerDI::SetNormalize(bool normalize)
{
    m_normalize = normalize;
}

bool CJoystickListenerDI::GetNormalize(void)
{
    return m_normalize;
}