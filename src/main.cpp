#include <Arduino.h>
#include <map>
#include <thread>
#include <chrono>

#include "screen_control.h"
#include "network_control.h"
#include "time_control.h"

ScreenControl* screen_control = nullptr;
TimeControl* time_control = nullptr;
NetworkControl* network_control = nullptr;

std::map<int, std::string> WeatherMap;

int Index = 0;
int QuarterIterateCounter = 0; //For debug
bool bHasError = false;

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
    screen_control = new ScreenControl();
    network_control = new NetworkControl();
    time_control = new TimeControl(network_control);

    SetupWeatherMap();

    screen_control->DisplayMessage("Connecting to WiFi..", 0, 0);
    screen_control->Display();

    if(!network_control->TryConnecting())
    {
        screen_control->DisplayMessage("An Error has occured.", 0, 16);
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
                Index = 0;
                QuarterIterateCounter++;
            }

            network_control->Disconnect();
        }
    }

    if(!bHasError)
    {
        screen_control->DisplayWeekDay(time_control->GetCurrentTimeStruct());
        screen_control->DisplayDate(time_control->GetCurrentTimeStruct());

        screen_control->DisplayTemprature(network_control->GetTemprature(Index), 0, 16);
        int WeatherCode = network_control->GetWeatherCode(Index);
        screen_control->DisplayWeatherCode(WeatherMap[WeatherCode], 0, 36);
        screen_control->DisplayTimeHrMin(network_control->GetTime(Index), 0, 46, false);

        screen_control->DisplayTimeHrMin(time_control->GetCurrentTimeStruct(), 0, 16, true);
        screen_control->DisplayTimeHrMin(time_control->GetQuaterTimeStruct(), 0, 26, true);

        screen_control->DisplayIteration(QuarterIterateCounter, 0, 36, true);

        Index++;
        if(Index > 5)
        {
            Index = 0;
        }
    }

    screen_control->Display();

    std::this_thread::sleep_for(std::chrono::seconds(10));
}
