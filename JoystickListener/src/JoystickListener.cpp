#include "JoystickListener.h"

#pragma comment(lib, "winmm.lib")

CJoystickListener::~CJoystickListener()
{
    StopListening();
}

CJoystickListener::CJoystickListener(UINT joystickId)
    : m_joystickId(joystickId), 
    m_running(false), 
    m_initialized(false), 
    m_silentAxis(true), 
    m_silentButton(true), 
    m_silentButtonHeld(true),
    m_pExternalObject(nullptr)
{
    ZeroMemory(&m_joyInfoPrev, sizeof(m_joyInfoPrev));
    m_joyInfoPrev.dwSize = sizeof(JOYINFOEX);
    m_joyInfoPrev.dwFlags = JOY_RETURNALL;
}

bool CJoystickListener::Init()
{
    JOYCAPS caps;
    if (joyGetDevCaps(m_joystickId, &caps, sizeof(caps)) != JOYERR_NOERROR)
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "Joystick ID " << m_joystickId << " not found.\n";
        m_initialized = false;
        return false;
    }

    m_initialized = true;

    if (m_logger && !m_silentButton)
        (*m_logger) << "Joystick ID " << m_joystickId << " initialized.\n";

    return true;
}

void CJoystickListener::Reset()
{
    ZeroMemory(&m_joyInfoPrev, sizeof(m_joyInfoPrev));
    m_joyInfoPrev.dwSize = sizeof(JOYINFOEX);
    m_joyInfoPrev.dwFlags = JOY_RETURNALL;

    if (m_logger && !m_silentButton)
        (*m_logger) << "Joystick reset.\n";
}

void CJoystickListener::Start() {
    if (m_initialized && !m_running) {
        m_running = true;
        m_thread = std::thread(&CJoystickListener::ListenLoop, this);
        if (m_logger && !m_silentButton)
            (*m_logger) << "[CJoystickListener] Listening thread started.\n";
    }
}

void CJoystickListener::Stop() {
    bool wasRunning = m_running.exchange(false);
    if (m_thread.joinable() && std::this_thread::get_id() != m_thread.get_id())
    {
        m_thread.join();
    }
    if (wasRunning)
    {
        if (m_logger && !m_silentButton)
            (*m_logger) << "[CJoystickListener] Listening thread stopped.\n";
    }
    m_running = false;
}

void CJoystickListener::CalibrateCenter()
{
    if (!m_initialized)
        return;

    JOYINFOEX joyInfo;
    joyInfo.dwSize = sizeof(JOYINFOEX);
    joyInfo.dwFlags = JOY_RETURNALL;

    if (joyGetPosEx(m_joystickId, &joyInfo) == JOYERR_NOERROR)
    {
        m_joyInfoPrev.dwXpos = joyInfo.dwXpos;
        m_joyInfoPrev.dwYpos = joyInfo.dwYpos;
        m_joyInfoPrev.dwZpos = joyInfo.dwZpos;
        m_joyInfoPrev.dwPOV = joyInfo.dwPOV;

        if (m_logger && !m_silentAxis)
            (*m_logger) << "Joystick center calibrated.\n";
    }
}

void CJoystickListener::StartListening()
{
    if (m_running)
        return;

    m_running = true;
    m_thread = std::thread(&CJoystickListener::ListenLoop, this);
}

void CJoystickListener::StopListening()
{
    if (!m_running)
        return;

    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

bool CJoystickListener::IsRunning() const
{
    return m_running;
}

bool CJoystickListener::IsStopped() const
{
    return !m_running;
}

bool CJoystickListener::IsInit() const
{
    return m_initialized;
}

void CJoystickListener::SetAxisHandler(AxisHandler handler)
{
    m_axisHandler = handler;
}

void CJoystickListener::SetButtonHandler(ButtonHandler handler)
{
    m_buttonHandler = handler;
}

void CJoystickListener::SetButtonHeldHandler(ButtonHeldHandler handler)
{
    m_buttonHeldHandler = handler;
}

void CJoystickListener::SetLogger(std::shared_ptr<std::ostream> logger)
{
    m_logger = logger;
}

void CJoystickListener::SetSilentMode(bool silentAxis, bool silentButton, bool silentButtonHeld)
{
    m_silentAxis = silentAxis;
    m_silentButton = silentButton;
    m_silentButtonHeld = silentButtonHeld;
}

void CJoystickListener::ListenLoop()
{
    JOYINFOEX joyInfo;
    joyInfo.dwSize = sizeof(JOYINFOEX);
    joyInfo.dwFlags = JOY_RETURNALL;

    m_joyInfoPrev.dwXpos = 0;
    m_joyInfoPrev.dwYpos = 0;
    m_joyInfoPrev.dwZpos = 0;
    m_joyInfoPrev.dwPOV = JOY_POVCENTERED;
    m_joyInfoPrev.dwButtons = 0;

    while (m_running)
    {
        MMRESULT res = joyGetPosEx(m_joystickId, &joyInfo);
        if (res != JOYERR_NOERROR)
        {
            Sleep(50);
            continue;
        }

        // button edge
        if (m_buttonHandler)
        {
            DWORD prevButtons = m_joyInfoPrev.dwButtons;
            DWORD currButtons = joyInfo.dwButtons;

            for (int i = 0; i < 32; ++i)
            {
                bool prevPressed = (prevButtons & (1 << i)) != 0;
                bool currPressed = (currButtons & (1 << i)) != 0;

                if (prevPressed != currPressed)
                {
                    m_buttonHandler(i + 1, currPressed);

                    if (m_logger && !m_silentButton)
                    {
                        (*m_logger) << "[Button] " << (i + 1) << (currPressed ? " pressed" : " released") << "\n";
                    }
                }
            }
        }

        // button held
        if (m_buttonHeldHandler)
        {
            for (int i = 0; i < 32; ++i)
            {
                bool currPressed = (joyInfo.dwButtons & (1 << i)) != 0;

                if (currPressed)
                {
                    m_buttonHeldHandler(i + 1);

                    if (m_logger && !m_silentButton && !m_silentButtonHeld)
                    {
                        (*m_logger) << "[Button Held] " << (i + 1) << " is being held down\n";
                    }
                }
            }
        }

        // axes
        if (m_axisHandler)
        {
            int correctedX = 0;
            int correctedY = 0;
            int correctedZ = 0;
            int correctedRZ = 0;
            int correctedPov = 0;
            std::string povDir = "";
            bool axisChanged = false;

            axisChanged = (
                m_joyInfoPrev.dwXpos != joyInfo.dwXpos ||
                m_joyInfoPrev.dwYpos != joyInfo.dwYpos ||
                m_joyInfoPrev.dwZpos != joyInfo.dwZpos ||
                m_joyInfoPrev.dwPOV != joyInfo.dwPOV
                );

            if (axisChanged)
            {
                correctedX     = joyInfo.dwXpos;
                correctedY     = joyInfo.dwYpos;
                correctedZ     = UseThrottleButtonAsReversed ? (65535 - joyInfo.dwZpos) : (joyInfo.dwZpos);
                correctedPov   = joyInfo.dwPOV;
                povDir         = MapPOV(joyInfo.dwPOV);

                if (m_logger && !m_silentAxis)
                {
                    std::stringstream ss;
                    ss << "[Axis] ";
                    ss << "  X : "      << std::setw(6) << correctedX;
                    ss << "  Y : "      << std::setw(6) << correctedY;
                    ss << "  Z : "      << std::setw(6) << correctedZ;
                    ss << "  Pov : "    << std::setw(6) << correctedPov;
                    ss << "  PovDir : " << std::setw(6) << povDir;
                    (*m_logger) << ss.str() << "\n";
                }

                m_axisHandler(correctedX, correctedY, correctedZ, correctedPov, povDir);
            }
        }

        m_joyInfoPrev = joyInfo;

        Sleep(20);
    }
}

std::string CJoystickListener::MapPOV(DWORD pov)
{
    std::string povName = "Unknown";

    if (pov == JOY_POVCENTERED || pov == 0xFFFF)
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

void CJoystickListener::SetExternalObject(void* pObject)
{
    m_pExternalObject = pObject;
}

void* CJoystickListener::GetExternalObject(void)
{
    return m_pExternalObject;
}