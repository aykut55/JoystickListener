#include "Aircraft.h"

CAircraft::~CAircraft()
{

}

CAircraft::CAircraft() :
    headingDeg(0.0),
    latDeg(0.0),
    lonDeg(0.0),
    altMeter(0.0),
    speedMPS(0.0),
    rollDeg(0.0),
    pitchDeg(0.0),
    yawDeg(0.0),
    throttle(0.0),
    rollCmd(0.0),
    pitchCmd(0.0),
    yawCmd(0.0),
    throttleCmd(0.0),
    iterCount(0),
    isRollingLeft(false), isRollingRight(false), isPitchingUp(false), isPitchingDown(false), 
    isYawingLeft(false), isYawingRight(false), isThrottlingUp(false), isThrottlingDown(false)
{
    currentTime = std::chrono::steady_clock::now();
    lastPrintTime = std::chrono::steady_clock::now();
}

void CAircraft::RollLeft(void)              { rollCmd--;                                            }
void CAircraft::RollRight(void)             { rollCmd++;                                            }
void CAircraft::PitchUp(void)               { pitchCmd++;                                           }
void CAircraft::PitchDown(void)             { pitchCmd--;                                           }
void CAircraft::YawLeft(void)               { yawCmd--;                                             }
void CAircraft::YawRight(void)              { yawCmd++;                                             }
void CAircraft::ThrottleUp(void)            { throttleCmd++;                                        }
void CAircraft::ThrottleDown(void)          { throttleCmd--;                                        }

void CAircraft::StartRollLeft(void)         { isRollingLeft = true;    isRollingRight = false;      }
void CAircraft::StartRollRight(void)        { isRollingRight = true;   isRollingLeft = false;       }
void CAircraft::StopRolling(void)           { isRollingLeft = false;   isRollingRight = false;      }
                                                                                                
void CAircraft::StartPitchUp(void)          { isPitchingUp = true;     isPitchingDown = false;      }
void CAircraft::StartPitchDown(void)        { isPitchingDown = true;   isPitchingUp = false;        }
void CAircraft::StopPitching(void)          { isPitchingUp = false;    isPitchingDown = false;      }
                                                                                                
void CAircraft::StartYawLeft(void)          { isYawingLeft = true;     isYawingRight = false;       }
void CAircraft::StartYawRight(void)         { isYawingRight = true;    isYawingLeft = false;        }
void CAircraft::StopYawing(void)            { isYawingLeft = false;    isYawingRight = false;       }
                                                                                                
void CAircraft::StartThrottleUp(void)       { isThrottlingUp = true;   isThrottlingDown = false;    }
void CAircraft::StartThrottleDown(void)     { isThrottlingDown = true; isThrottlingUp = false;      }
void CAircraft::StopThrottle(void)          { isThrottlingUp = false;  isThrottlingDown = false;    }

// RollCmd PitchCmd YawCmd ThrottleCmd
void    CAircraft::SetRollCmd(double rollCmd)       { this->rollCmd = rollCmd;                      }
double  CAircraft::GetRollCmd(void)                 { return rollCmd;                               }

void    CAircraft::SetPitchCmd(double pitchCmd)     { this->pitchCmd = pitchCmd;                    }
double  CAircraft::GetPitchCmd(void)                { return pitchCmd;                              }

void    CAircraft::SetYawCmd(double yawCmd)         { this->yawCmd = yawCmd;                        }
double  CAircraft::GetYawCmd(void)                  { return yawCmd;                                }

void    CAircraft::SetThrottleCmd(double value)     { this->throttleCmd = value;                    }
double  CAircraft::GetThrottleCmd(void)             { return throttleCmd;                           }

// Roll Pitch Yaw Throttle
void    CAircraft::SetRoll(double rollDeg)          { this->rollDeg = rollDeg;                      }
double  CAircraft::GetRoll(void)                    { return rollDeg;                               }

void    CAircraft::SetPitch(double pitchDeg)        { this->pitchDeg = pitchDeg;                    }
double  CAircraft::GetPitch(void)                   { return pitchDeg;                              }

void    CAircraft::SetYaw(double yawDeg)            { this->yawDeg = yawDeg;                        }
double  CAircraft::GetYaw(void)                     { return yawDeg;                                }

void    CAircraft::SetThrottle(double value)        { this->throttle = value;                       }
double  CAircraft::GetThrottle(void)                { return throttle;                              }

// Heading Latitude Longitude Altitude Speed
void    CAircraft::SetHeading(double headingDeg)    { this->headingDeg = headingDeg;                }
double  CAircraft::GetHeading(void)                 { return headingDeg;                            }

void    CAircraft::SetLatitude(double latDeg)       { this->latDeg = latDeg;                        }
double  CAircraft::GetLatitude(void)                { return latDeg;                                }

void    CAircraft::SetLongitude(double lonDeg)      { this->lonDeg = lonDeg;                        }
double  CAircraft::GetLongitude(void)               { return lonDeg;                                }

void    CAircraft::SetAltitude(double altMeter)     { this->altMeter = altMeter;                    }
double  CAircraft::GetAltitude(void)                { return altMeter;                              }

void    CAircraft::SetSpeed(double speedMPS)        { this->speedMPS = speedMPS;                    }
double  CAircraft::GetSpeed(void)                   { return speedMPS;                              }

void CAircraft::Update(void)
{
    incIterCount();

    if (isRollingLeft)     rollCmd--;
    if (isRollingRight)    rollCmd++;
    if (isPitchingUp)      pitchCmd++;
    if (isPitchingDown)    pitchCmd--;
    if (isYawingLeft)      yawCmd--;
    if (isYawingRight)     yawCmd++;
    if (isThrottlingUp)    throttleCmd++;
    if (isThrottlingDown)  throttleCmd--;
}

void CAircraft::NeutralizeAll(void)
{
    rollCmd = pitchCmd = yawCmd = throttleCmd = 0;
}

void CAircraft::PrintStatus(void) 
{
    std::cout << "iterCount: " << std::setw(4) << iterCount;
    std::cout << "  [Aircraft Status] ";
    std::cout << "  Roll: " << std::setw(4) << std::fixed << std::setprecision(4) << rollCmd;
    std::cout << "  Pitch: " << std::setw(4) << std::fixed << std::setprecision(4) << pitchCmd;
    std::cout << "  Yaw: " << std::setw(4) << std::fixed << std::setprecision(4) << yawCmd;
    std::cout << "  Throttle: " << std::setw(4) << std::fixed << std::setprecision(4) << throttleCmd;
    std::cout << std::endl;
}

int CAircraft::GetIterCount(void)
{
    return iterCount;
}

void CAircraft::SetIterCount(int value)
{
    iterCount = value;
}

void CAircraft::incIterCount(void)
{
    iterCount++;
}

void CAircraft::PrintFlighData(void)
{
    int iterCount = GetIterCount();

    // Ucagin guncel degerlerini al
    double rollDeg = GetRoll();
    double pitchDeg = GetPitch();
    double yawDeg = GetYaw();
    double throttle = GetThrottle();
    double headingDeg = GetHeading();
    double latDeg = GetLatitude();
    double lonDeg = GetLongitude();
    double altMeter = GetAltitude();
    double speedMPS = GetSpeed();

    // Klavyeden girilen degerleri get methodlarýyla al
    double rollCmd = GetRollCmd();
    double pitchCmd = GetPitchCmd();
    double yawCmd = GetYawCmd();
    double throttleCmd = GetThrottleCmd();

    std::stringstream ss;

#if 1
    ss << "i : " << std::setw(4) << iterCount;
    ss << "  ";
    ss << " RollCmd : " << std::setw(4) << std::fixed << std::setprecision(0) << rollCmd;
    ss << " PitchCmd : " << std::setw(4) << std::fixed << std::setprecision(0) << pitchCmd;
    ss << " YawCmd : " << std::setw(4) << std::fixed << std::setprecision(0) << yawCmd;
    ss << " ThrottleCmd : " << std::setw(4) << std::fixed << std::setprecision(0) << throttleCmd;
    ss << "  ";
    ss << " RollDeg : " << std::setw(4) << std::fixed << std::setprecision(4) << rollDeg;
    ss << " PitchDeg : " << std::setw(4) << std::fixed << std::setprecision(4) << pitchDeg;
    ss << " YawDeg : " << std::setw(4) << std::fixed << std::setprecision(4) << yawDeg;
    ss << " Throttle : " << std::setw(4) << std::fixed << std::setprecision(4) << throttle;
    ss << "  ";
    ss << " HeadingDeg : " << std::setw(4) << std::fixed << std::setprecision(4) << headingDeg;
    ss << " LatDeg : " << std::setw(4) << std::fixed << std::setprecision(4) << latDeg;
    ss << " LonDeg : " << std::setw(4) << std::fixed << std::setprecision(4) << lonDeg;
    ss << " AltMeter : " << std::setw(4) << std::fixed << std::setprecision(4) << altMeter;
    ss << " SpeedMPS : " << std::setw(4) << std::fixed << std::setprecision(4) << speedMPS;
    ss << "  ";
#else
    ss << " i : " << std::left << std::setw(4) << iterCount;
    ss << "  ";
    ss << "  ThrottleCmd : " << std::left << std::setw(4) << std::fixed << std::setprecision(0) << throttleCmd;
    ss << "  ";
    ss << "  R : " << std::setw(4) << std::fixed << std::setprecision(0) << rollCmd;
    ss << "  P : " << std::setw(4) << std::fixed << std::setprecision(0) << pitchCmd;
    ss << "  Y : " << std::setw(4) << std::fixed << std::setprecision(0) << yawCmd;
    ss << "  ";
    ss << "  Throttle : " << std::left << std::setw(4) << std::fixed << std::setprecision(0) << throttle;
    ss << "  ";
    ss << "  R : " << std::setw(4) << std::fixed << std::setprecision(4) << rollDeg;
    ss << "  P : " << std::setw(4) << std::fixed << std::setprecision(4) << pitchDeg;
    ss << "  Y : " << std::setw(4) << std::fixed << std::setprecision(4) << yawDeg;
    ss << "  ";
    ss << "  Heading : " << std::setw(4) << std::fixed << std::setprecision(4) << headingDeg;
    ss << "  Lat : " << std::setw(4) << std::fixed << std::setprecision(4) << latDeg;
    ss << "  Lon : " << std::setw(4) << std::fixed << std::setprecision(4) << lonDeg;
    ss << "  Alt : " << std::setw(4) << std::fixed << std::setprecision(4) << altMeter;
    ss << "  SpeedMPS : " << std::setw(4) << std::fixed << std::setprecision(4) << speedMPS;
    ss << "  ";
#endif

    currentTime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastPrintTime).count() > 20) {
        lastPrintTime = currentTime;
        std::cout << ss.str() << std::endl;
    }
}