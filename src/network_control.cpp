#include "network_control.h"
#include "network_defines.h"    //Not added to Github on purpose

NetworkControl::NetworkControl()
{
    Serial.begin(115200);

    m_TimeStruct = new tm();
    m_JSON = new String("{}");
}

NetworkControl::~NetworkControl()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    delete m_TimeStruct;
    delete m_JSON;
}

bool NetworkControl::TryConnecting()
{
    int Attempts = 0;
    bool bIsConnected = false;

    WiFi.begin(SSID, PASS);
    while ((WiFi.status() != WL_CONNECTED)) 
    {
        delay(500);

        if(Attempts > m_MaxConnectAttempt)
        {
            break;
        }

        Attempts++;
    }

    if(WiFi.status() == WL_CONNECTED)
    { 
        bIsConnected = true;
    }
    else
    {
        Disconnect();
    }

    return bIsConnected;
}

void NetworkControl::Disconnect()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

tm NetworkControl::GetNTPTime()
{
    configTime(m_GmtOffset_sec, m_DaylightOffset_sec, m_NtpServer);

    tm timeinfo;
    getLocalTime(&timeinfo);

    return timeinfo;
}

void NetworkControl::HttpGETRequest(const char* URL)
{
    WiFiClient Client;
    HTTPClient Http;

    Http.begin(Client, URL);

    int ReponseCode = Http.GET();

    if(ReponseCode > 0)
    {
        *m_JSON = Http.getString();
    }

    Http.end();
}

bool NetworkControl::GetWeatherJSON()
{
    HttpGETRequest(m_WeatherURL);

    if(!m_JSON)
    {
        return false;
    }

    DeserializationError Err = deserializeJson(m_JsonDoc, *m_JSON);
    
    if(Err)
    {
        return false;
    }

    return true;
}

const tm& NetworkControl::GetTime(const int Index)
{
    time_t UnixTime = m_JsonDoc["hourly"]["time"][Index];
    *m_TimeStruct = *localtime(&UnixTime);
    return *m_TimeStruct;
}

const float NetworkControl::GetTemprature(const int Index)
{
    return m_JsonDoc["hourly"]["temperature_2m"][Index];
}

const int NetworkControl::GetWeatherCode(const int Index)
{
    return m_JsonDoc["hourly"]["weather_code"][Index];
}
