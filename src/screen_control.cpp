#include "screen_control.h"
#include <time.h>
#include <sstream>
#include <iomanip>

ScreenControl::ScreenControl()
{
    m_Display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C (U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

    m_Display->setBusClock(8000000);
    m_Display->begin();

    DisplayClearScreen();

    ResetFont();

    m_Display->setFontMode(1);
    m_Display->setFontPosTop();
    m_Display->home();

    m_Display->setContrast(1);
}

ScreenControl::~ScreenControl()
{
    std::map<const char*, Scroller*>::iterator It;
    for(It = m_ScrollMap.begin(); It != m_ScrollMap.end(); ++It)
    {
        delete It->second;
    }
    
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

bool ScreenControl::DisplayScrollMessage(const char* Msg, int16_t x, int16_t y)
{
    //m_Display->setFont(u8g2_font_t0_12b_tf);
    //m_Display->setFont(u8g2_font_pxplustandynewtv_8u);

    bool bHasReachedEnd = false;

    int16_t SizeDiff = GetUTFWidth(Msg) - m_MaxTextWidth;

    std::map<const char*, Scroller*>::iterator It;
    Scroller* CurrentScroller = nullptr;

    It = m_ScrollMap.find(Msg);
    if(It == m_ScrollMap.end())
    {
        Scroller* NewScroller = new Scroller();
        m_ScrollMap.insert({Msg, NewScroller});
        It = m_ScrollMap.find(Msg);

        It->second->Height = m_Display->getAscent() - m_Display->getDescent();
        It->second->Width = m_Display->getUTF8Width(Msg);
    }

    CurrentScroller = It->second;

    //Clips screen
    m_Display->setClipWindow(64, 48, 128, 64);

    if(CurrentScroller->bDirection)
    {
        CurrentScroller->ScrollOffsetX += 1;
    }
    else
    {
        CurrentScroller->ScrollOffsetX  -= 1;
    }

    if((CurrentScroller->ScrollOffsetX) > 0)
    {
        CurrentScroller->bDirection = false;
        bHasReachedEnd = true;
    }
    else if ((CurrentScroller->ScrollOffsetX) < -SizeDiff)
    {
        CurrentScroller->bDirection = true;
        bHasReachedEnd = true;
    }

    m_Display->drawUTF8(x + CurrentScroller->ScrollOffsetX, y, Msg);

    //Reset clipped screen
    m_Display->setMaxClipWindow();

    return bHasReachedEnd;
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

    int16_t width = GetUTFWidth(buffer);

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
        int16_t width = GetUTFWidth(buffer);
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

bool ScreenControl::DisplayWeatherCode(const std::string& WeatherCode, int16_t x, int16_t y)
{
    int16_t CharWidth = GetUTFWidth(WeatherCode);

    //Only scroll if the weather code doesn't fit all on screen
    if(CharWidth > m_MaxTextWidth)
    {
        return DisplayScrollMessage(WeatherCode.c_str(), x, y);
    }
    else
    {
        m_Display->setCursor(x, y);
        m_Display->println(WeatherCode.c_str());
    }

    return false;
}

void ScreenControl::DisplayWeatherIcon(const unsigned char* WeatherIcon)
{
    m_Display->drawBitmap(0, 16, 6, 48, WeatherIcon);
}

void ScreenControl::ResetFont()
{
    m_Display->setFont(m_DefaultFont);
}

int16_t ScreenControl::GetUTFWidth(const std::string& String)
{
    return m_Display->getUTF8Width(String.c_str());
}