#include "network_control.h"
#include "network_defines.h"    //Not added to Github on purpose

#include <time.h>

NetworkControl::NetworkControl()
{
    Serial.begin(115200);

    Init();
}

NetworkControl::~NetworkControl()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void NetworkControl::Init()
{
    WiFi.begin(SSID, PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
}

tm NetworkControl::GetNTPTime()
{
    configTime(m_GmtOffset_sec, m_DaylightOffset_sec, m_NtpServer);

    tm timeinfo;
    getLocalTime(&timeinfo);

    return timeinfo;
}

String NetworkControl::HttpGETRequest(const char* URL)
{
    WiFiClient Client;
    HTTPClient Http;

    Http.begin(Client, URL);

    int ReponseCode = Http.GET();

    String JSON = "{}";

    if(ReponseCode > 0)
    {
        JSON = Http.getString();
    }
    else
    {
        //Print error on screen
    }

    Http.end();

    return JSON;
}

bool NetworkControl::GetWeatherJSON()
{
    String JSON = HttpGETRequest(m_WeatherURL3);

    DeserializationError Err = deserializeJson(m_JsonDoc, JSON);
    
    if(Err)
    {
        return false;
    }

    m_Times = m_JsonDoc["minutely_15"]["time"];
    m_Temps = m_JsonDoc["minutely_15"]["temperature_2m"];
    m_Codes = m_JsonDoc["minutely_15"]["weather_code"];

    return true;
}

const float NetworkControl::GetTemprature()
{
    return m_JsonDoc["current"]["temperature_2m"];
}

const int NetworkControl::GetWeatherCode()
{
    return m_JsonDoc["current"]["weather_code"];
}

const float NetworkControl::GetTemprature2(const time_t& UnixTime)
{
    int iteration = 0;
    for(iteration = (m_Times.size() - 1); iteration >= 0; iteration--)
    {
        time_t Time = m_Times[iteration];
        if(UnixTime > Time)
        {
            break;
        }
    }

    float Temp = m_Temps[iteration];
    return Temp;
}

const int NetworkControl::GetWeatherCode2(const time_t& UnixTime)
{
    int iteration = 0;
    for(iteration = (m_Times.size() - 1); iteration >= 0; iteration--)
    {
        time_t Time = m_Times[iteration];
        if(UnixTime > Time)
        {
            break;
        }
    }

    int Code = m_Codes[iteration];
    return Code;
}

const float NetworkControl::GetTemprature3()
{
    return m_JsonDoc["minutely_15"]["temperature_2m"][0];
}

const int NetworkControl::GetWeatherCode3()
{
    return m_JsonDoc["minutely_15"]["weather_code"][0];
}
