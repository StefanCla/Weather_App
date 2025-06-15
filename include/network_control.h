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

    const float GetTemprature2(const time_t& UnixTime);
    const int GetWeatherCode2(const time_t& UnixTime);

    const float GetTemprature3();
    const int GetWeatherCode3();

private:
    //NTP server
    const char* m_NtpServer = "pool.ntp.org";
    const long  m_GmtOffset_sec = 0; //UK
    const int   m_DaylightOffset_sec = 3600;

    //Weather
    const char* m_WeatherURL = "http://api.open-meteo.com/v1/forecast?latitude=50.799&longitude=-1.0913&current=temperature_2m,weather_code&timezone=GMT&forecast_days=1";
    const char* m_WeatherURL2 = "http://api.open-meteo.com/v1/forecast?latitude=50.799&longitude=-1.0913&minutely_15=temperature_2m,weather_code&timezone=Europe%2FLondon&forecast_days=1&forecast_hours=6&forecast_minutely_15=4";
    const char* m_WeatherURL3 = "http://api.open-meteo.com/v1/forecast?latitude=50.799&longitude=-1.0913&minutely_15=temperature_2m,weather_code&timezone=Europe%2FLondon&forecast_days=1&timeformat=unixtime&forecast_hours=6&forecast_minutely_15=4";

    JsonDocument m_JsonDoc;
    JsonArray m_Times;
    JsonArray m_Temps;
    JsonArray m_Codes;
};