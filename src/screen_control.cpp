#include "screen_control.h"
#include <time.h>
#include <sstream>
#include <iomanip>

ScreenControl::ScreenControl()
{
    m_Display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C (U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

    m_Display->setBusClock(8000000); //Not sure if this actually makes a difference, but placebo is powerful.
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

void ScreenControl::DisplayArea(uint8_t TileX, uint8_t TileY, uint8_t TileXCount, uint8_t  TileYCount)
{
    m_Display->updateDisplayArea(TileX, TileY, TileXCount, TileYCount);
}

void ScreenControl::DisplayClearScreen()
{
    m_Display->clearBuffer();
}

void ScreenControl::DisplayMessage(const char* Msg, int16_t X, int16_t Y)
{
    m_Display->setCursor(X, Y);
    m_Display->println(Msg);
}

bool ScreenControl::DisplayScrollMessage(const char* Msg, int16_t X, int16_t Y)
{
    bool bHasReachedEnd = false;

    int16_t SizeDiff = GetUTFWidth(Msg) - m_MaxTextWidth;

    std::map<const char*, Scroller*>::iterator It;
    Scroller* CurrentScroller = nullptr;

    It = m_ScrollMap.find(Msg);
    if(It == m_ScrollMap.end())
    {
        //Add new entry if we can't find any
        Scroller* NewScroller = new Scroller();
        m_ScrollMap.insert({Msg, NewScroller});
        It = m_ScrollMap.find(Msg);

        It->second->Height = m_Display->getAscent() - m_Display->getDescent();
        It->second->Width = m_Display->getUTF8Width(Msg);
    }

    CurrentScroller = It->second;

    //Clips screen
    m_Display->setClipWindow(64, 48, 128, 64);  //bottom right corner

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

    //Reset offset on first display, and set bHasReachedEnd true to avoid immediate start of scrolling.
    std::string CurrentScrollText = Msg;
    if((m_PreviousScrollText.empty()) || (m_PreviousScrollText != CurrentScrollText))
    {
        m_PreviousScrollText = CurrentScrollText;
        It->second->bDirection = false;
        It->second->ScrollOffsetX = 0;
        bHasReachedEnd = true;
    }

    m_Display->drawUTF8(X + CurrentScroller->ScrollOffsetX, Y, Msg);

    //Reset clipped screen
    m_Display->setMaxClipWindow();

    return bHasReachedEnd;
}

void ScreenControl::DisplayWeekDay(const tm& TimeInfo)
{
    m_Display->setCursor(0, 5);

    char Buffer[10];
    strftime(Buffer, 10, "%A", &TimeInfo);
    m_Display->println(Buffer);
}

void ScreenControl::DisplayDate(const tm& TimeInfo)
{
    char Buffer[10];
    strftime(Buffer, 10, "%e-%b-%y", &TimeInfo);

    int16_t Width = GetUTFWidth(Buffer);

    int OffsetX = SCREEN_WIDTH - Width;
    
    m_Display->setCursor(OffsetX, 5);
    m_Display->println(Buffer);
}

void ScreenControl::DisplayTimeHrMin(const tm& TimeInfo, int16_t X, int16_t Y, bool bFromRightSide)
{
    m_Display->setFont(m_WeatherNumFont);

    char Buffer[10];
    strftime(Buffer, 10, "%R", &TimeInfo);

    if(bFromRightSide)
    {
        int16_t Width = GetUTFWidth(Buffer);
        X = SCREEN_WIDTH - (Width + X);
    }

    m_Display->setCursor(X, Y);
    m_Display->println(Buffer);
}

void ScreenControl::DisplayDrawCelcius(int16_t X, int16_t Y)
{
    //Drawing ASCII degree looks weird
    //Thus we do it manually

    X += 1; //Add slight offset

    //draw small circle
    m_Display->setDrawColor(1);
    m_Display->drawPixel(X + 1, Y);  //Top
    m_Display->drawPixel(X + 2, Y);

    m_Display->drawPixel(X, Y + 1);  //Left
    m_Display->drawPixel(X, Y + 2);

    m_Display->drawPixel(X + 3, Y + 1);  //Right
    m_Display->drawPixel(X + 3, Y + 2);

    m_Display->drawPixel(X + 1, Y + 3);  //Bottom
    m_Display->drawPixel(X + 2, Y + 3);

    m_Display->setCursor(X + 5, Y);
    m_Display->println("C");
}

void ScreenControl::DisplayTemprature(float Temprature, int16_t X, int16_t Y)
{
    m_Display->setFont(m_WeatherNumFont);

    std::stringstream Stream;
    Stream << std::fixed << std::setprecision(1) << Temprature;
    std::string TempratureStr = Stream.str();

    m_Display->setCursor(X, Y);
    m_Display->println(TempratureStr.c_str());

    int16_t Width = m_Display->getUTF8Width(TempratureStr.c_str());
    DisplayDrawCelcius(X + Width, Y);
}

bool ScreenControl::DisplayWeatherCode(const std::string& WeatherCode, int16_t X, int16_t Y)
{
    int16_t CharWidth = GetUTFWidth(WeatherCode);

    //Only scroll if the weather code doesn't fit all on screen
    if(CharWidth > m_MaxTextWidth)
    {
        return DisplayScrollMessage(WeatherCode.c_str(), X, Y);
    }
    else
    {
        m_PreviousScrollText = WeatherCode;
        m_Display->setCursor(X, Y);
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

void ScreenControl::SetContrast(uint8_t Contrast)
{
    m_Display->setContrast(Contrast);
}

int16_t ScreenControl::GetUTFWidth(const std::string& String)
{
    return m_Display->getUTF8Width(String.c_str());
}