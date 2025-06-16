#include <cmath>
#include <time.h>

class NetworkControl;

class TimeControl
{
public:
    TimeControl(NetworkControl* Network);
    ~TimeControl();

    void Tick();
    void CalculateNextQuarter();

    inline void SetCurrentTime(time_t currentTime) { *m_CurrentTime = currentTime; }
    inline const time_t& GetCurrentTime() const { return *m_CurrentTime; }
    inline const tm& GetCurrentTimeStruct() const { return * m_CurrentTimeStruct; }

    inline const time_t& GetQuarterTime() const { return *m_QuarterTime; }
    inline const tm& GetQuaterTimeStruct() const { return *m_QuarterTimeStruct; }

    inline float GetTimeSec() const { return m_TimeSec; }

    void CorrectTime();

private:
    NetworkControl* m_NetworkControl = nullptr;

    time_t* m_CurrentTime = nullptr;
    tm* m_CurrentTimeStruct = nullptr;

    time_t* m_QuarterTime = nullptr;
    tm* m_QuarterTimeStruct = nullptr;

    int m_TimeSec = -1;
};