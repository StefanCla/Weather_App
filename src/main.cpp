#include <Arduino.h>
#include <map>
#include <thread>
#include <chrono>

#include "screen_control.h"
#include "network_control.h"
#include "time_control.h"
#include "weather_bitmaps.h"

ScreenControl* screen_control = nullptr;
TimeControl* time_control = nullptr;
NetworkControl* network_control = nullptr;

std::map<int, std::string> WeatherMap;
std::map<int, const unsigned char*> WeatherIconMap;

int HourlyIndex = 0;
int WeatherCode = 0;

bool bHasError = false;
bool bShouldScroll = false;
bool bHasReachedScrollEnd = false;

//********************************/
//  General flow:
//  Every quarter of the hour (00:00 - 00:15 - 00:30 - 00:45),
//  We connect to the internet and obtain the NTP time & weather data.
//  NTP time is used to correct the ESP32 clock as accurately as possible, though arguably, every 15 minutes might be overkill. 
//  Weather data is used to display the current and (up to) 6 hours of weather forecast.
// 
//  Details are shown on an SSD1306 I2C SPI display, the details visible are:
//  -Current week day
//  -Current date
//  -Weather icon based on weather code
//  -Weather forecast time - To help indicate what details are associated with what hour of the day
//  -Weather temprature (in celcius)
//  -Weather type based on weather code - To help indicate what weather it will be, if the icon does not provide enough clarity
//
//  If we fail to connect to the internet or obtain the weather JSON, We try again in 10 seconds.
//  We attempt to connect to the internet 10 times, before going on cooldown.
//
//  Hourly weather data is shown every ~10 seconds, meaning it takes a minute before we loop back.
//  We try to reduce screen-updates as much as possible, hence why we delay for 10 seconds if we know all data is static.
//  If we scroll the weather type (as it might be too long to fit on screen), we delay only for a few milliseconds.
//
//  Other stuff:
//  There is a severe lack of nullptr checks in the code.
//  This is on purpose, as all data constructed isn't getting deleted until we exit the program.
//  Which it never does, as the ESP32 goes on infinite-loop after it has been setup regardless.
//  + I was being lazy for this project :)
//
//  From a quick search, it seems that `std::this_thread::sleep_for()` doesn't work accurately on a ESP32,
//  Hence why delays are used instead.
// 
//  Check: https://github.com/StefanCla/Weather_App to see pictures of what this program looks like when running.
//
//  Credits:
//  https://open-meteo.com/ was used to obtain weather data.
//  https://www.ntppool.org/en/ was used to obtain the current NTP time.
//  ArduinoJson by Benoit Blanchon was used to deserialize the weather data.
//  u82g by olikraus was used to get the display up & running
//
//********************************/


//WMO - Sky Type
void SetupWeatherMap()
{
    WeatherMap.insert({0, "Clear Sky"});
    WeatherMap.insert({1, "Mainly Clear"});
    WeatherMap.insert({2, "Partly Cloudy"});
    WeatherMap.insert({3, "Overcast"});
    WeatherMap.insert({45, "Fog"});
    WeatherMap.insert({48, "Rime Fog"});
    WeatherMap.insert({51, "Light Drizzle"});
    WeatherMap.insert({53, "Moderate Drizzle"});
    WeatherMap.insert({55, "Dense Drizzle"});
    WeatherMap.insert({56, "Light Freezing Drizzle"});
    WeatherMap.insert({57, "Dense Freezing Drizzle"});
    WeatherMap.insert({61, "Slight Rain"});
    WeatherMap.insert({63, "Moderate Rain"});
    WeatherMap.insert({65, "Heavy Rain"});
    WeatherMap.insert({66, "Light Freezing Rain"});
    WeatherMap.insert({67, "Heavy Freezing Rain"});
    WeatherMap.insert({71, "Slight Snow Fall"});
    WeatherMap.insert({73, "Moderate Snow Fall"});
    WeatherMap.insert({75, "Heavy Snow Fall"});
    WeatherMap.insert({77, "Snow Grains"});
    WeatherMap.insert({80, "Slight Rain Showers"});
    WeatherMap.insert({81, "Moderate Rain Showers"});
    WeatherMap.insert({82, "Heavy Rain Showers"});
    WeatherMap.insert({85, "Slight Snow Showers"});
    WeatherMap.insert({86, "Heavy Snow Showers"});
    WeatherMap.insert({95, "Thunderstorm"});
    WeatherMap.insert({96, "Thunderstorm w/ Slight Hail"});
    WeatherMap.insert({99, "Thunderstorm w/ Heavy Hail"});
}

void SetupWeahterIconMap()
{
    WeatherIconMap.insert({0, weather_icon_sun});
    WeatherIconMap.insert({1, weather_icon_mainly_clear});
    WeatherIconMap.insert({2, weather_icon_partially_cloudy});
    WeatherIconMap.insert({3, weather_icon_overcast});
    WeatherIconMap.insert({51, weather_icon_drizzle});
    WeatherIconMap.insert({61, weather_icon_light_rain});
    WeatherIconMap.insert({65, weather_icon_heavy_rain});
}

void setup() {
    screen_control = new ScreenControl();
    network_control = new NetworkControl();
    time_control = new TimeControl(network_control);

    SetupWeatherMap();
    SetupWeahterIconMap();

    screen_control->DisplayMessage("Connecting to WiFi..", 0, 0);
    screen_control->Display();

    if(!network_control->TryConnecting())
    {
        //No need to display error, it will immediately retry on first run of the loop.
        //screen_control->DisplayMessage("An Error has occured.", 0, 16);
        bHasError = true;
    }
    else
    {
        screen_control->DisplayClearScreen();

        time_control->CorrectTime();
        time_control->CalculateNextQuarter();

        if(!network_control->GetWeatherJSON())
        {
            screen_control->DisplayMessage("Failed obtaining JSON.", 0, 0);
            screen_control->DisplayMessage("An Error has occured.", 0, 16);
            bHasError = true;
        }
        else
        {
            bHasError = false;
        }

        network_control->Disconnect();
    }

    screen_control->Display();
}

void loop() {
    time_control->Tick();

    //Turn screen off between 00:00 and 06:00, we (I) dont care about the weather at that time.
    if((time_control->GetCurrentTimeStruct().tm_hour >= 0) &&
        (time_control->GetCurrentTimeStruct().tm_hour <= 6))
    {
        screen_control->m_Display->setContrast(0);
    } 
    else
    {
        screen_control->m_Display->setContrast(1);
    }

    screen_control->DisplayClearScreen();

    if((time_control->GetCurrentTime() >= time_control->GetQuarterTime()) || bHasError)
    {
        screen_control->DisplayMessage("Connecting to WiFi..", 0, 0);
        screen_control->Display();

        if(!network_control->TryConnecting())
        {
            screen_control->DisplayMessage("An Error has occured.", 0, 16);
            screen_control->DisplayMessage("Retrying in 10 sec.", 0, 32);
            bHasError = true;
        }
        else
        {
            screen_control->DisplayClearScreen();

            time_control->CorrectTime();
            time_control->CalculateNextQuarter();

            if(!network_control->GetWeatherJSON())
            {
                screen_control->DisplayMessage("Failed obtaining JSON.", 0, 0);
                screen_control->DisplayMessage("An Error has occured.", 0, 16);
                screen_control->DisplayMessage("Retrying in 10 sec.", 0, 32);

                bHasError = true;
            }
            else
            {
                bHasError = false;
                HourlyIndex = 0;
            }

            network_control->Disconnect();
        }
    }

    if(!bHasError && (time_control->GetCurrentTime() > time_control->GetNext10Sec()))
    {
         time_control->CalculateNext10Sec();

        screen_control->DisplayWeekDay(time_control->GetCurrentTimeStruct());
        screen_control->DisplayDate(time_control->GetCurrentTimeStruct());

        screen_control->DisplayTimeHrMin(network_control->GetTime(HourlyIndex), 64, 16, false);
        screen_control->DisplayTemprature(network_control->GetTemprature(HourlyIndex), 64, 32);
        WeatherCode = network_control->GetWeatherCode(HourlyIndex);

        screen_control->ResetFont();

        int16_t CharWidth = screen_control->GetUTFWidth(WeatherMap[WeatherCode].c_str());
        bShouldScroll = (CharWidth > screen_control->GetMaxTextWidth());

        screen_control->DisplayWeatherIcon(WeatherIconMap[WeatherCode]);  //Not used yet as we dont have icons for all

        //Display up to 6 hours of weather data
        HourlyIndex++;
        if(HourlyIndex > 5)
        {
            HourlyIndex = 0;
        }

        if(!bShouldScroll)
        {
            bHasReachedScrollEnd = screen_control->DisplayWeatherCode(WeatherMap[WeatherCode].c_str(), 64, 48);
            screen_control->Display();
            delay(10000);
        }
        else if(bShouldScroll)
        {
            screen_control->Display();
        }
    }

    if(bShouldScroll)
    {
        bHasReachedScrollEnd = screen_control->DisplayWeatherCode(WeatherMap[WeatherCode].c_str(), 64, 48);
        screen_control->m_Display->updateDisplayArea(8, 6, 8, 2);

        if(bHasReachedScrollEnd)
        {
            delay(3000);
        }
        else
        {
            delay(45);
        }
    }
}