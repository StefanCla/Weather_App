#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <WString.h>
#include <ArduinoJson.h>

class NetworkControl
{
public:
    NetworkControl();
    ~NetworkControl();

    bool TryConnecting();
    void Disconnect();

    tm GetNTPTime();
    void HttpGETRequest(const char* URL);

    bool GetWeatherJSON();

    const tm& GetTime(const int Index);
    const float GetTemprature(const int Index);
    const int GetWeatherCode(const int Index);

private:
    const int m_MaxConnectAttempt = 10;

    //NTP server
    const char* m_NtpServer = "pool.ntp.org";
    const long  m_GmtOffset_sec = 0; //UK
    const int   m_DaylightOffset_sec = 3600;

    //Weather
    const char* m_WeatherURL = "http://api.open-meteo.com/v1/forecast?latitude=50.799&longitude=-1.0913&hourly=temperature_2m,weather_code&timezone=Europe%2FLondon&timeformat=unixtime&forecast_hours=6&cell_selection=nearest";

    JsonDocument m_JsonDoc;
    String* m_JSON = nullptr;

    tm* m_TimeStruct = nullptr;

};