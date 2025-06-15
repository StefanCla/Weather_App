#include "time_control.h"
#include "network_control.h"

TimeControl::TimeControl(NetworkControl* Network)
{  
    m_NetworkControl = Network;

    m_CurrentTime = new time_t();
    m_CurrentTimeStruct = new tm();

    m_QuarterTime = new time_t();
    m_QuarterTimeStruct = new tm();

    CorrectTime();

    *m_QuarterTime = *m_CurrentTime;
    *m_QuarterTimeStruct = *m_CurrentTimeStruct;

    CalculateNextQuarter();
}

TimeControl::~TimeControl()
{
    delete m_QuarterTimeStruct;
    delete m_QuarterTime;

    delete m_CurrentTimeStruct;
    delete m_CurrentTime;
}

void TimeControl::CalculateNextQuarter()
{
    const float hour = 60.0f;
    const float quart = 15.0f;

    float MinRemainHour = hour - (float)m_CurrentTimeStruct->tm_min;
    float Val = MinRemainHour / quart;

    float whole, frac;
    frac = std::modf(Val, &whole);

    frac += 0.05f;
    int MinRemain = frac * quart;

    if(MinRemain <= 0)
    {
      MinRemain = 15;
    }

    m_TimeSec = (MinRemain * 60) - m_CurrentTimeStruct->tm_sec;

    *m_QuarterTime = *m_CurrentTime + m_TimeSec;
    *m_QuarterTimeStruct = *localtime(m_QuarterTime);
}

void TimeControl::Tick()
{
    *m_CurrentTime = time(NULL);
    *m_CurrentTimeStruct = *localtime(m_CurrentTime);
}

void TimeControl::CorrectTime()
{
    *m_CurrentTimeStruct = m_NetworkControl->GetNTPTime();
    *m_CurrentTime = mktime(m_CurrentTimeStruct);
}