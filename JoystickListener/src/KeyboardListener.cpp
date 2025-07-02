#include "KeyboardListener.h"
#include "ConsoleLogger.h"
#include "KeyboardUtils.h"
#include <Windows.h>

CKeyboardListener::CKeyboardListener()
    : m_running(false), m_initialized(false), m_silentMode(false) 
{
    m_logger = std::make_shared<ConsoleLogger>();
}

CKeyboardListener::~CKeyboardListener() {
    Stop();
}

void CKeyboardListener::Init() {
    m_initialized = true;
    if (!m_silentMode) {
        m_logger->Log("[CKeyboardListener] Init completed.");
    }
}

void CKeyboardListener::Reset() {
    Stop();
    m_initialized = false;
    if (!m_silentMode) {
        m_logger->Log("[CKeyboardListener] Reset completed.");
    }
}

void CKeyboardListener::Start() {
    if (m_initialized && !m_running) {
        m_running = true;
        m_thread = std::thread(&CKeyboardListener::ListenLoop, this);
        if (!m_silentMode) {
            m_logger->Log("[CKeyboardListener] Listening thread started.");
        }
    }
}

void CKeyboardListener::Stop() {
    bool wasRunning = m_running.exchange(false);
    if (m_thread.joinable() && std::this_thread::get_id() != m_thread.get_id())
    {
        m_thread.join();
    }
    if (wasRunning)
    {
        if (!m_silentMode) {
            m_logger->Log("[CKeyboardListener] Listening thread stopped.");
        }
    }
    m_running = false;
}

void CKeyboardListener::SetLogger(std::shared_ptr<ILogger> logger) {
    if (logger) m_logger = logger;
}

bool CKeyboardListener::IsRunning() const { return m_running.load(); }
bool CKeyboardListener::IsStopped() const { return !m_running.load(); }
bool CKeyboardListener::IsInit() const { return m_initialized.load(); }

void CKeyboardListener::RegisterHandler(int vk, std::function<void(const KeyEvent&)> handler) {
    m_handlers2[vk] = handler;
    m_handlers[vk].push_back(handler);
}

void CKeyboardListener::ClearKeyHistory() {
    m_keyHistory.clear();
}

const std::unordered_map<int, KeyHistory> CKeyboardListener::GetKeyHistory() const {
    return m_keyHistory;
}

std::unordered_map<int, KeyHistory> CKeyboardListener::GetKeyHistoryCopy() const {
    return m_keyHistory;
}

void CKeyboardListener::ListenLoop() {
    if (!m_silentMode) {
        m_logger->Log("[CKeyboardListener] Dinleme baslatildi. ESC ile cikabilirsiniz.");
    }
    bool keyState[256] = { false };

    while (m_running) {
        for (int vk = 1; vk <= 254; ++vk) {
            SHORT state = GetAsyncKeyState(vk);
            bool isCurrentlyPressed = (state & 0x8000) != 0;
            bool wasPreviouslyPressed = keyState[vk];

            bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
            bool ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
            bool alt   = (GetAsyncKeyState(VK_MENU) & 0x8000);

            KeyHistory& hist = m_keyHistory[vk];
            hist.wasPressed  = hist.isPressed;
            hist.wasReleased = hist.isReleased;
            hist.isPressed   = isCurrentlyPressed;
            hist.isReleased  = !isCurrentlyPressed;

            if (isCurrentlyPressed && !wasPreviouslyPressed) {
                keyState[vk] = true;
                KeyEvent evt{ vk, KeyState::Down, shift, ctrl, alt };
                m_keyHistory[vk].pressCount++;
                m_keyHistory[vk].currentHoldCount = 1;
                //m_keyHistory[vk].isPressed = true;
                m_keyHistory[vk].lastState = KeyState::Down;
                m_keyHistory[vk].lastPressedTime = std::chrono::steady_clock::now();

                if (!m_silentMode) {
                    m_logger->Log("[Down] " + GetKeyName(vk) + " (" + std::to_string(vk) + ")");
                }
                auto it = m_handlers.find(vk);
                //if (it != m_handlers2.end()) it->second(evt);
                if (it != m_handlers.end()) {
                    for (auto& handler : it->second)
                        handler(evt);
                }                    
                if (vk == VK_ESCAPE) {
                    if (!m_silentMode) {
                        m_logger->Log("[CKeyboardListener] ESC algilandi, cikiliyor.");
                    }
                    m_running = false;
                    break;
                }
            }
            else if (isCurrentlyPressed && wasPreviouslyPressed) {
                KeyEvent evt{ vk, KeyState::Hold, shift, ctrl, alt };
                m_keyHistory[vk].lastState = KeyState::Hold;
                m_keyHistory[vk].currentHoldCount++;

                std::string msg = "[Hold] " + GetKeyName(vk) + " (" + std::to_string(vk) + ") [Held " + std::to_string(m_keyHistory[vk].currentHoldCount) + "x]";
                if (!m_silentMode) {
                    m_logger->Log(msg);
                }

                auto it = m_handlers.find(vk);
                //if (it != m_handlers2.end()) it->second(evt);
                if (it != m_handlers.end()) {
                    for (auto& handler : it->second)
                        handler(evt);
                }
            }
            else if (!isCurrentlyPressed && wasPreviouslyPressed) {
                keyState[vk] = false;
                KeyEvent evt{ vk, KeyState::Up, shift, ctrl, alt };
                m_keyHistory[vk].isPressed = false;
                m_keyHistory[vk].lastState = KeyState::Up;
                m_keyHistory[vk].currentHoldCount = 0;
                m_keyHistory[vk].lastReleasedTime = std::chrono::steady_clock::now();

                std::string msg = "[Up  ] " + GetKeyName(vk) + " (" + std::to_string(vk) + ")";
                if (!m_silentMode) {
                    m_logger->Log(msg);
                }

                auto it = m_handlers.find(vk);
                //if (it != m_handlers2.end()) it->second(evt);
                if (it != m_handlers.end()) {
                    for (auto& handler : it->second)
                        handler(evt);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    if (!m_silentMode) {
        m_logger->Log("[CKeyboardListener] Dinleme durduruldu.");
    }
}

void CKeyboardListener::SetSilentMode(bool silentMode) {
    m_silentMode = silentMode;
}