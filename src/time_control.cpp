#include "time_control.h"
#include "network_control.h"

TimeControl::TimeControl(NetworkControl* Network)
{  
    m_NetworkControl = Network;

    m_CurrentTime = new time_t();
    m_CurrentTimeStruct = new tm();

    m_QuarterTime = new time_t();
    m_QuarterTimeStruct = new tm();

    m_NextTenSeconds = new time_t();

    CorrectTime();

    *m_QuarterTime = *m_CurrentTime;
    *m_QuarterTimeStruct = *m_CurrentTimeStruct;

    *m_NextTenSeconds = *m_CurrentTime;

    CalculateNextTenSeconds();
    CalculateNextQuarter();
}

TimeControl::~TimeControl()
{
    delete m_NextTenSeconds;

    delete m_QuarterTimeStruct;
    delete m_QuarterTime;

    delete m_CurrentTimeStruct;
    delete m_CurrentTime;
}

void TimeControl::Tick()
{
    *m_CurrentTime = time(NULL);
    *m_CurrentTimeStruct = *localtime(m_CurrentTime);
}

void TimeControl::CalculateNextTenSeconds()
{
    *m_NextTenSeconds = *m_CurrentTime + 10;
}

void TimeControl::CalculateNextQuarter()
{
    const float Hour = 60.0f;
    const float Quarter = 15.0f;
    const int Seconds = 60;

    float MinRemainHour = Hour - static_cast<float>(m_CurrentTimeStruct->tm_min);
    float QuartersRemaining = MinRemainHour / Quarter;

    float Whole, Frac;
    Frac = std::modf(QuartersRemaining, &Whole);

    Frac += 0.05f;  //Round
    int MinRemaining = Frac * Quarter;

    if(MinRemaining <= 0)
    {
      MinRemaining = 15;
    }

    int SecondsTillQuarter = (MinRemaining * Seconds) - m_CurrentTimeStruct->tm_sec;

    *m_QuarterTime = *m_CurrentTime + SecondsTillQuarter;
    *m_QuarterTimeStruct = *localtime(m_QuarterTime);
}

void TimeControl::CorrectTime()
{
    *m_CurrentTimeStruct = m_NetworkControl->GetNTPTime();
    *m_CurrentTime = mktime(m_CurrentTimeStruct);
}