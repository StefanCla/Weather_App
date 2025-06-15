#include <Arduino.h>
#include <map>

#include "screen_control.h"
#include "network_control.h"
#include "time_control.h"

ScreenControl* screen_control = nullptr;
TimeControl* time_control = nullptr;
NetworkControl* network_control = nullptr;

std::map<int, std::string> WeatherMap;

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
    WeatherMap.insert({71, "Slightl Snow Fall"});
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

void setup() {
    network_control = new NetworkControl();
    screen_control = new ScreenControl();
    time_control = new TimeControl(network_control);

    SetupWeatherMap();

    if(!network_control->GetWeatherJSON())
    {
        screen_control->DisplayError();
        return;
    }

    screen_control->DisplayWeekDay(time_control->GetCurrentTimeStruct());
    screen_control->DisplayDate(time_control->GetCurrentTimeStruct());

    screen_control->DisplayTemprature(network_control->GetTemprature3(), 0, 16);
    int WeatherCode = network_control->GetWeatherCode3();
    screen_control->DisplayWeatherCode(WeatherMap[WeatherCode], 0, 36);

    screen_control->DisplayTimeHrMin(time_control->GetCurrentTimeStruct(), 0, 16, true);
    screen_control->DisplayTimeHrMin(time_control->GetQuaterTimeStruct(), 0, 26, true);

    screen_control->DisplayIteration(0, 36, true);
}

void loop() {

    time_control->Tick();

    if(time_control->GetCurrentTime() >= time_control->GetQuarterTime())
    {
        time_control->CorrectTime();
        time_control->CalculateNextQuarter();

        if(!network_control->GetWeatherJSON())
        {
            screen_control->DisplayError();
            return;
        }

        screen_control->DisplayClearScreen();
        screen_control->DisplayWeekDay(time_control->GetCurrentTimeStruct());
        screen_control->DisplayDate(time_control->GetCurrentTimeStruct());

        screen_control->DisplayTemprature(network_control->GetTemprature3(), 0, 16);
        int WeatherCode = network_control->GetWeatherCode3();
        screen_control->DisplayWeatherCode(WeatherMap[WeatherCode], 0, 36);

        screen_control->DisplayTimeHrMin(time_control->GetCurrentTimeStruct(), 0, 16, true);
        screen_control->DisplayTimeHrMin(time_control->GetQuaterTimeStruct(), 0, 26, true);

        screen_control->DisplayIteration(0, 36, true);
    }
}
