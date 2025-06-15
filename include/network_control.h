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

    void Init();

    tm GetNTPTime();
    String HttpGETRequest(const char* URL);

    bool GetWeatherJSON();

    const float GetTemprature();
    const int GetWeatherCode();

private:
    //NTP server
    const char* m_NtpServer = "pool.ntp.org";
    const long  m_GmtOffset_sec = 0; //UK
    const int   m_DaylightOffset_sec = 3600;

    //Weather
    const char* m_WeatherURL = "http://api.open-meteo.com/v1/forecast?latitude=50.799&longitude=-1.0913&current=temperature_2m,weather_code&timezone=GMT&forecast_days=1";

    JsonDocument m_JsonDoc;
};