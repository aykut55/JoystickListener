#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <map>
#include <windows.h>

#include "FileLogger.h"
#include "ConsoleLogger.h"
#include "CompositeLogger.h"
#include "Aircraft.h"

#include "JoystickListener.h"
int mainJoystickListener()
{
    CJoystickListener joystick(0);

    // Bunu öðren
    auto logger = std::make_shared<std::ostream>(std::cout.rdbuf());
    joystick.SetLogger(logger);

    if (!joystick.Init())
    {
        std::cerr << "Joystick init failed.\n";
        return 1;
    }

    joystick.SetAxisHandler([](int x, int y, int z, int pov, std::string povDir) {
        std::stringstream ss;
        ss << "[Axis] ";
        ss << "  X : " << std::setw(6) << x;
        ss << "  Y : " << std::setw(6) << y;
        ss << "  Z : " << std::setw(6) << z;
        ss << "  Pov : " << std::setw(6) << pov;
        ss << "  PovDir : " << std::setw(6) << povDir;
        std::cout << ss.str() << "\n";
        });

    joystick.SetButtonHandler([](int buttonId, bool pressed) {
        std::stringstream ss;
        ss << "[Button] " << buttonId << (pressed ? " pressed" : " released") << "\n";
        std::cout << ss.str() << "\n";
        if (buttonId == 7 && !pressed)
            system("cls");
        });

    joystick.SetButtonHeldHandler([](int buttonId) {
        std::stringstream ss;
        ss << "[Button Held] " << buttonId << " is being held down\n";
        std::cout << ss.str() << "\n";
        });

    joystick.CalibrateCenter();

    joystick.StartListening();

    std::cout << "Joystick listening started. Press ESC to exit.\n";

    while (true)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }
        Sleep(100);
    }

    joystick.StopListening();

    std::cout << "Joystick listening stopped. Exiting.\n";
    return 0;
}

#include "JoystickListenerDI.h"
std::vector<GUID> EnumerateJoysticks()
{
    std::vector<GUID> guids;

    LPDIRECTINPUT8 directInput;
    if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&directInput, NULL)))
    {
        return guids;
    }

    auto enumCallback = [](const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) -> BOOL
        {
            auto guidList = reinterpret_cast<std::vector<GUID>*>(pContext);
            guidList->push_back(pdidInstance->guidInstance);
            std::wcout << L"Found device: " << pdidInstance->tszProductName << std::endl;
            return DIENUM_CONTINUE;
        };

    directInput->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback, &guids, DIEDFL_ATTACHEDONLY);
    directInput->Release();
    return guids;
}

int mainJoystickListenerDI()
{
    auto guids = EnumerateJoysticks();

    if (guids.empty())
    {
        std::cerr << "No DirectInput joystick found.\n";
        return 1;
    }

    CJoystickListenerDI joystick(guids[0]);

    // Bunu öðren
    auto logger = std::make_shared<std::ostream>(std::cout.rdbuf());    
    // joystick.SetLogger(logger);

    if (!joystick.Init())
    {
        std::cerr << "Joystick init failed.\n";
        return 1;
    }

    joystick.SetButtonHandler([](int buttonId, bool pressed) {
        std::stringstream ss;
        ss << "[Button] " << buttonId << (pressed ? " pressed" : " released") << "\n";
        std::cout << ss.str() << "\n";
        if (buttonId == 7 && !pressed)
            system("cls");
        });

    joystick.SetButtonHeldHandler([](int buttonId) {
        std::stringstream ss;
        ss << "[Button Held] " << buttonId << " is being held down\n";
        std::cout << ss.str() << "\n";
        });

    joystick.SetAxisHandler([](int x, int y, int z, int rz, int pov, std::string povDir) {
        std::stringstream ss;
        ss << "[Axis] ";
        ss << "  X : "      << std::setw(6) << x;
        ss << "  Y : "      << std::setw(6) << y;
        ss << "  Z : "      << std::setw(6) << z;
        ss << "  RZ : "     << std::setw(6) << rz;
        ss << "  Pov : "    << std::setw(6) << pov;
        ss << "  PovDir : " << std::setw(6) << povDir;
        std::cout << ss.str() << "\n";
        });

    joystick.CalibrateCenter();
    joystick.StartListening();

    std::cout << "Listening... Press ESC to exit.\n";

    while (true)
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;
        Sleep(100);
    }

    joystick.StopListening();
    return 0;
}

int mainJoystickListenerWithAircraft()
{
    CAircraft aircraft;

    auto logger = std::make_shared<ConsoleLogger>();

    auto listener = std::make_shared<CJoystickListener>(0);
    //listener->SetLogger(logger);
    listener->SetExternalObject(&aircraft);

    if (!listener->Init())
    {
        std::cerr << "Joystick init failed.\n";
        return 1;
    }

    listener->SetButtonHandler([=](int buttonId, bool pressed) {
        std::stringstream ss;
        ss << "[Button] " << buttonId << (pressed ? " pressed" : " released") << "\n";
        //std::cout << ss.str() << "\n";
        if (buttonId == 7 && !pressed)
            system("cls");
        });

    listener->SetButtonHeldHandler([=](int buttonId) {
        std::stringstream ss;
        ss << "[Button Held] " << buttonId << " is being held down\n";
        //std::cout << ss.str() << "\n";
        });

    listener->SetAxisHandler([=](int x, int y, int z, int pov, std::string povDir) {
        std::stringstream ss;
        ss << "[Axis] ";
        ss << "  X : " << std::setw(6) << x;
        ss << "  Y : " << std::setw(6) << y;
        ss << "  Z : " << std::setw(6) << z;
        ss << "  Pov : " << std::setw(6) << pov;
        ss << "  PovDir : " << std::setw(6) << povDir;
        //std::cout << ss.str() << "\n";

        CAircraft* pAircraft = (CAircraft*)listener->GetExternalObject();
        if (pAircraft)
        {
            double normalizerCoeff = 1 / 65535.0;
            pAircraft->SetRollCmd(x * normalizerCoeff);
            pAircraft->SetPitchCmd(y * normalizerCoeff);
            pAircraft->SetThrottleCmd(z * normalizerCoeff);
            pAircraft->SetYawCmd(0);
        }

        });

    listener->CalibrateCenter();

    listener->Start();

    auto lastPrint = std::chrono::steady_clock::now();

    while (listener->IsRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }

        aircraft.Update();

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPrint).count() > 20) {
            lastPrint = now;
        }

        aircraft.PrintStatus();
    }

    listener->Stop();


    return 0;
}

int mainJoystickListenerDIWithAircraft()
{
    CAircraft aircraft;

    auto guids = EnumerateJoysticks();

    if (guids.empty())
    {
        std::cerr << "No DirectInput joystick found.\n";
        return 1;
    }

    auto logger = std::make_shared<ConsoleLogger>();

    auto listener = std::make_shared<CJoystickListenerDI>(guids[0]);
    //listener->SetLogger(logger);
    listener->SetExternalObject(&aircraft);

    if (!listener->Init())
    {
        std::cerr << "Joystick init failed.\n";
        return 1;
    }

    listener->SetButtonHandler([=](int buttonId, bool pressed) {
        std::stringstream ss;
        ss << "[Button] " << buttonId << (pressed ? " pressed" : " released") << "\n";
        //std::cout << ss.str() << "\n";
        if (buttonId == 7 && !pressed)
            system("cls");
        });

    listener->SetButtonHeldHandler([=](int buttonId) {
        std::stringstream ss;
        ss << "[Button Held] " << buttonId << " is being held down\n";
        //std::cout << ss.str() << "\n";
        });

    listener->SetAxisHandler([=](int x, int y, int z, int rz, int pov, std::string povDir) {
        std::stringstream ss;
        ss << "[Axis] ";
        ss << "  X : " << std::setw(6) << x;
        ss << "  Y : " << std::setw(6) << y;
        ss << "  Z : " << std::setw(6) << z;
        ss << "  RZ : " << std::setw(6) << rz;
        ss << "  Pov : " << std::setw(6) << pov;
        ss << "  PovDir : " << std::setw(6) << povDir;
        //std::cout << ss.str() << "\n";

        CAircraft* pAircraft = (CAircraft*)listener->GetExternalObject();
        if (pAircraft)
        {
            double normalizerCoeff = 1 / 65535.0;
            pAircraft->SetRollCmd(x * normalizerCoeff);
            pAircraft->SetPitchCmd(y * normalizerCoeff);
            pAircraft->SetThrottleCmd(z * normalizerCoeff);
            pAircraft->SetYawCmd(rz * normalizerCoeff);
        }
        });

    listener->CalibrateCenter();

    listener->Start();

    auto lastPrint = std::chrono::steady_clock::now();

    while (listener->IsRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }

        aircraft.Update();

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPrint).count() > 20) {
            lastPrint = now;
        }

        aircraft.PrintStatus();
    }

    listener->Stop();


    return 0;
}

int main()
{
    // Roll + Pitch + Throttle, CAircraft
    // return mainJoystickListenerWithAircraft();

    // Roll + Pitch + Yaw + Throttle, CAircraft
    return mainJoystickListenerDIWithAircraft();

    // Roll + Pitch + Yaw + Throttle, 
    // return mainJoystickListenerDI();
 
    // Roll + Pitch + Throttle, 
    // return mainJoystickListener();
}