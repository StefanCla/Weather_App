#include <cmath>
#include <time.h>

class NetworkControl;

class TimeControl
{
public:
    TimeControl(NetworkControl* Network);
    ~TimeControl();

    void Tick();
    void CalculateNext10Sec();
    void CalculateNextQuarter();

    inline void SetCurrentTime(time_t currentTime) { *m_CurrentTime = currentTime; }
    inline const time_t& GetCurrentTime() const { return *m_CurrentTime; }
    inline const tm& GetCurrentTimeStruct() const { return * m_CurrentTimeStruct; }

    inline const time_t& GetQuarterTime() const { return *m_QuarterTime; }
    inline const tm& GetQuaterTimeStruct() const { return *m_QuarterTimeStruct; }

    inline const time_t& GetNext10Sec() const { return *m_Next10Sec; }

    inline const float GetTimeSec() const { return m_TimeSec; }

    void CorrectTime();

private:
    NetworkControl* m_NetworkControl = nullptr;

    time_t* m_CurrentTime = nullptr;
    tm* m_CurrentTimeStruct = nullptr;

    time_t* m_QuarterTime = nullptr;
    tm* m_QuarterTimeStruct = nullptr;

    time_t* m_Next10Sec = nullptr;

    int m_TimeSec = -1;
};