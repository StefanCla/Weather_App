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
    const char* m_Ssid       = "";
    const char* m_Password   = "";

    //NTP server
    const char* m_NtpServer = "pool.ntp.org";
    const long  m_GmtOffset_sec = 0; //UK
    const int   m_DaylightOffset_sec = 3600;

    //Weather
    const char* m_WeatherURL = "";

    JsonDocument m_JsonDoc;
};