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

std::map<int, std::pair<std::string, const unsigned char*>> WeatherDataMap;

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
//  Check: https://github.com/StefanCla/Weather_App to see pictures of what this program looks like when running.
//
//  Credits:
//  https://open-meteo.com/ was used to obtain weather data.
//  https://www.ntppool.org/en/ was used to obtain the current NTP time.
//  ArduinoJson by Benoit Blanchon was used to deserialize the weather data.
//  u82g by olikraus was used to get the display up & running.
//  Aseprite was used to create the weather icons.
//  https://javl.github.io/image2cpp/ was used to convert weather icons to bitmaps.
//
//********************************/

//WMO - Weather Type / Weather Icon
void SetupWeatherDataMap()
{
    WeatherDataMap.insert({0, {"Clear Sky", weather_icon_clear_sky }});
    WeatherDataMap.insert({1, {"Mainly Clear", weather_icon_mainly_clear}});
    WeatherDataMap.insert({2, {"Partly Cloudy", weather_icon_partly_cloudy}});
    WeatherDataMap.insert({3, {"Overcast", weather_icon_overcast}});
    WeatherDataMap.insert({45, {"Fog", weather_icon_fog}});
    WeatherDataMap.insert({48, {"Rime Fog", weather_icon_fog}});  //Same icon as fog on purpose
    WeatherDataMap.insert({51, {"Light Drizzle", weather_icon_slight_drizzle}});
    WeatherDataMap.insert({53, {"Moderate Drizzle", weather_icon_moderate_drizzle}});
    WeatherDataMap.insert({55, {"Dense Drizzle", weather_icon_heavy_drizzle}});
    WeatherDataMap.insert({56, {"Light Freezing Drizzle", weather_icon_light_freezing_drizzle}});
    WeatherDataMap.insert({57, {"Dense Freezing Drizzle", weather_icon_dense_freezing_drizzle}});
    WeatherDataMap.insert({61, {"Slight Rain", weather_icon_slight_rain}});
    WeatherDataMap.insert({63, {"Moderate Rain", weather_icon_moderate_rain}});
    WeatherDataMap.insert({65, {"Heavy Rain", weather_icon_heavy_rain}});
    WeatherDataMap.insert({66, {"Light Freezing Rain", weather_icon_light_freezing_rain}});
    WeatherDataMap.insert({67, {"Heavy Freezing Rain", weather_icon_heavy_freezing_rain}});
    WeatherDataMap.insert({71, {"Slight Snow Fall", weather_icon_slight_snow_fall}});
    WeatherDataMap.insert({73, {"Moderate Snow Fall", weather_icon_moderate_snow_fall}});
    WeatherDataMap.insert({75, {"Heavy Snow Fall", weather_icon_heavy_snow_fall}});
    WeatherDataMap.insert({77, {"Snow Grains", weather_icon_snow_grains}});
    WeatherDataMap.insert({80, {"Slight Rain Showers", weather_icon_slight_rain_shower}});
    WeatherDataMap.insert({81, {"Moderate Rain Showers", weather_icon_moderate_rain_shower}});
    WeatherDataMap.insert({82, {"Heavy Rain Showers", weather_icon_heavy_rain_shower}});
    WeatherDataMap.insert({85, {"Slight Snow Showers", weather_icon_slight_snow_shower}});
    WeatherDataMap.insert({86, {"Heavy Snow Showers", weather_icon_heavy_snow_shower}});
    WeatherDataMap.insert({95, {"Thunderstorm", weather_icon_thunderstorm}});
    WeatherDataMap.insert({96, {"Thunderstorm with Slight Hail", weather_icon_thunderstorm_slight_hail}});
    WeatherDataMap.insert({99, {"Thunderstorm with Heavy Hail", weather_icon_thunderstorm_heavy_hail}});
}

void setup() {
    screen_control = new ScreenControl();
    network_control = new NetworkControl();
    time_control = new TimeControl(network_control);
    
    SetupWeatherDataMap();

    screen_control->DisplayMessage("Connecting to WiFi..", 0, 0);
    screen_control->Display();

    if(!network_control->TryConnecting())
    {
        bHasError = true;
    }
    else
    {
        time_control->CorrectTime();
        time_control->CalculateNextQuarter();

        if(!network_control->GetWeatherJSON())
        {
            bHasError = true;
        }
        else
        {
            bHasError = false;
        }

        network_control->Disconnect();
    }
}

void loop() {
    time_control->Tick();

    screen_control->DisplayClearScreen();
    screen_control->ResetFont();

    //Turn screen off between 00:00 and 06:00, we (I) dont care about the weather at that time.
    if((time_control->GetCurrentTimeStruct().tm_hour >= 0) &&
        (time_control->GetCurrentTimeStruct().tm_hour <= 6) && !bHasError)
    {
        screen_control->SetContrast(0);
    } 
    else
    {
        screen_control->SetContrast(1);
    }

    if((time_control->GetCurrentTime() >= time_control->GetQuarterTime()) || bHasError)
    {
        bShouldScroll = false;

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
                screen_control->DisplayMessage("Failed obtaining JSON.", 0, 16);
                screen_control->DisplayMessage("An Error has occured.", 0, 32);
                screen_control->DisplayMessage("Retrying in 10 sec.", 0, 48);

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

    if(bHasError)
    {
        screen_control->Display();
        std::this_thread::sleep_for(std::chrono::seconds(10));
        return;
    }

    if((time_control->GetCurrentTime() > time_control->GetNextTenSeconds()))
    {
        time_control->CalculateNextTenSeconds();

        WeatherCode = network_control->GetWeatherCode(HourlyIndex);

        int16_t CharWidth = screen_control->GetUTFWidth(WeatherDataMap[WeatherCode].first.c_str());
        bShouldScroll = (CharWidth > screen_control->GetMaxTextWidth());

        screen_control->DisplayWeekDay(time_control->GetCurrentTimeStruct(), 0, 0, false);
        screen_control->DisplayDate(time_control->GetCurrentTimeStruct(), 0, 0, true);

        screen_control->DisplayTimeHrMin(network_control->GetTime(HourlyIndex), 64, 16, true);
        screen_control->DisplayTemprature(network_control->GetTemprature(HourlyIndex), 64, 32, true);
        screen_control->DisplayWeatherIcon(WeatherDataMap[WeatherCode].second, 0, 16);

        //Display up to 6 hours of weather data
        HourlyIndex++;
        if(HourlyIndex > 5)
        {
            HourlyIndex = 0;
        }

        if(!bShouldScroll)
        {
            bHasReachedScrollEnd = screen_control->DisplayWeatherCode(WeatherDataMap[WeatherCode].first.c_str(), 64, 48, true);
        }

        screen_control->Display();
    }

    if(bShouldScroll)
    {
        bHasReachedScrollEnd = screen_control->DisplayWeatherCode(WeatherDataMap[WeatherCode].first.c_str(), 64, 48, true);
        screen_control->DisplayArea(8, 6, 8, 2);

        if(bHasReachedScrollEnd)
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(45));
        }
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}