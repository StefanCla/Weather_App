#include "network_control.h"
#include "network_defines.h"    //Not added to Github on purpose
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

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
    String JSON = HttpGETRequest(m_WeatherURL);

    DeserializationError Err = deserializeJson(m_JsonDoc, JSON);
    
    if(Err)
    {
        return false;
    }

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