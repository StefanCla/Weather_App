#include "screen_control.h"
#include <time.h>
#include <sstream>
#include <iomanip>

ScreenControl::ScreenControl()
{
    m_Display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C (U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

    m_Display->begin();

    DisplayClearScreen();

    ResetFont();
    m_Display->setFontPosTop();
    m_Display->home();

    m_Display->setContrast(1);
}

ScreenControl::~ScreenControl()
{
    delete m_Display;
}

void ScreenControl::Display()
{
    m_Display->sendBuffer();
}

void ScreenControl::DisplayClearScreen()
{
    m_Display->clearBuffer();
}

void ScreenControl::DisplayMessage(const char* Msg, int16_t x, int16_t y)
{
    m_Display->setCursor(x, y);
    m_Display->println(Msg);
}

void ScreenControl::DisplayWeekDay(const tm& timeinfo)
{
    m_Display->setCursor(0, 5);

    char buffer[10];
    strftime(buffer, 10, "%A", &timeinfo);
    m_Display->println(buffer);
}

void ScreenControl::DisplayDate(const tm& timeinfo)
{
    char buffer[10];
    strftime(buffer, 10, "%e-%b-%y", &timeinfo);

    short int x1, y1;
    short unsigned int w, h;
    int16_t width = m_Display->getUTF8Width(buffer);

    int offsetX = SCREEN_WIDTH - width;
    
    m_Display->setCursor(offsetX, 5);
    m_Display->println(buffer);
}

void ScreenControl::DisplayTimeHrMin(const tm& timeinfo, int16_t x, int16_t y, bool bFromRightSide)
{
    m_Display->setFont(m_WeatherNumFont);

    char buffer[10];
    strftime(buffer, 10, "%R", &timeinfo);

    if(bFromRightSide)
    {
        int16_t width = m_Display->getUTF8Width(buffer);
        x = SCREEN_WIDTH - (width + x);
    }

    m_Display->setCursor(x, y);
    m_Display->println(buffer);
}

void ScreenControl::DisplayDrawCelcius(int16_t x, int16_t y)
{
    //Drawing ASCII degree looks weird
    //Thus we do it manually

    x += 1; //Add slight offset

    //draw small circle
    m_Display->setDrawColor(1);
    m_Display->drawPixel(x + 1, y);  //Top
    m_Display->drawPixel(x + 2, y);

    m_Display->drawPixel(x, y + 1);  //Left
    m_Display->drawPixel(x, y + 2);

    m_Display->drawPixel(x + 3, y + 1);  //Right
    m_Display->drawPixel(x + 3, y + 2);

    m_Display->drawPixel(x + 1, y + 3);  //Bottom
    m_Display->drawPixel(x + 2, y + 3);

    m_Display->setCursor(x + 5, y);
    m_Display->println("C");
}

void ScreenControl::DisplayTemprature(float Temprature, int16_t x, int16_t y)
{
    m_Display->setFont(m_WeatherNumFont);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << Temprature;
    std::string NewTemp = stream.str();

    m_Display->setCursor(x, y);
    m_Display->println(NewTemp.c_str());

    int16_t width = m_Display->getUTF8Width(NewTemp.c_str());
    DisplayDrawCelcius(x + width, y);
}

void ScreenControl::DisplayWeatherCode(const std::string& WeatherCode, int16_t x, int16_t y)
{
    m_Display->setCursor(x, y);
    m_Display->println(WeatherCode.c_str());
}

void ScreenControl::DisplayWeatherIcon(const unsigned char* WeatherIcon)
{
    m_Display->drawBitmap(0, 16, 6, 48, WeatherIcon);
}

void ScreenControl::DisplayIteration(const int Iterate, int16_t x, int16_t y, bool bFromRightSide)
{
    if(bFromRightSide)
    {
        std::string TempString = std::to_string(Iterate);
        int16_t width = m_Display->getUTF8Width(TempString.c_str());

        x = SCREEN_WIDTH - (width + x);
    }

    m_Display->setCursor(x, y);
    m_Display->println(Iterate);
}

void ScreenControl::ResetFont()
{
    m_Display->setFont(m_DefaultFont);
}