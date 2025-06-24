#include "screen_control.h"
#include <time.h>
#include <sstream>
#include <iomanip>

ScreenControl::ScreenControl()
{
    m_Display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C (U8G2_R0, U8X8_PIN_NONE, SCL, SDA);

    //I don't think my screen can accept this high bus clock, but it still works fine and I believe there is less screen tearing.
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

    //Clips screen
    m_Display->setClipWindow(64, 48, 128, 64);  //bottom right corner

    //Reset offset on first display, and set bHasReachedEnd true to avoid immediate start of scrolling.
    std::string CurrentScrollText = Msg;
    if(m_PreviousScrollText != CurrentScrollText)
    {
        m_PreviousScrollText = CurrentScrollText;
        m_bDirection = false;
        m_ScrollOffsetX = 0;
        bHasReachedEnd = true;
    }
    else
    {
        int16_t SizeDiff = GetUTFWidth(Msg) - m_MaxTextWidth;

        if(m_bDirection)
        {
            m_ScrollOffsetX += 1;
        }
        else
        {
            m_ScrollOffsetX  -= 1;
        }

        if(m_ScrollOffsetX > 0)
        {
            m_bDirection = false;
            bHasReachedEnd = true;
        }
        else if (m_ScrollOffsetX < -SizeDiff)
        {
            m_bDirection = true;
            bHasReachedEnd = true;
        }
    }

    m_Display->drawUTF8(X + m_ScrollOffsetX, Y, Msg);

    //Reset clipped screen
    m_Display->setMaxClipWindow();

    return bHasReachedEnd;
}

void ScreenControl::DisplayWeekDay(const tm& TimeInfo, int16_t X, int16_t Y, bool bCalculateX)
{
    m_Display->setFont(m_WeatherAlphFont);

    char Buffer[10];
    strftime(Buffer, 10, "%A", &TimeInfo);

    int16_t PositionX = X;

    if(bCalculateX)
    {
        int16_t TextWidth = GetUTFWidth(Buffer);
        PositionX = CalculateX(TextWidth, true, false, 0);
    }

    m_Display->setCursor(PositionX, Y);
    m_Display->println(Buffer);
}

void ScreenControl::DisplayDate(const tm& TimeInfo, int16_t X, int16_t Y, bool bCalculateX)
{
    m_Display->setFont(m_WeatherAlphFont);

    char Buffer[10];
    strftime(Buffer, 10, "%e-%b-%y", &TimeInfo);

    int16_t PositionX = X;

    if(bCalculateX)
    {
        int16_t TextWidth = GetUTFWidth(Buffer);
        PositionX = CalculateX(TextWidth, true, false, 0);
    }
    
    m_Display->setCursor(PositionX, Y);
    m_Display->println(Buffer);
}

void ScreenControl::DisplayTimeHrMin(const tm& TimeInfo, int16_t X, int16_t Y, bool bCalculateX)
{
    m_Display->setFont(m_WeatherNumFont);

    char Buffer[10];
    strftime(Buffer, 10, "%R", &TimeInfo);

    int16_t PositionX = X;

    if(bCalculateX)
    {
        int16_t TextWidth = GetUTFWidth(Buffer);
        PositionX = CalculateX(TextWidth, true, true, 64);
    }

    m_Display->setCursor(PositionX, Y);
    m_Display->println(Buffer);
}

void ScreenControl::DisplayDrawCelcius(int16_t X, int16_t Y)
{
    //Drawing ASCII degree looks weird
    //Thus we do it manually

    X += 2; //Add slight offset

    //draw small circle
    m_Display->setDrawColor(1);
    m_Display->drawPixel(X + 1, Y);  //Top
    m_Display->drawPixel(X + 2, Y);
    m_Display->drawPixel(X + 3, Y);

    m_Display->drawPixel(X, Y + 1);  //Left
    m_Display->drawPixel(X, Y + 2);
    m_Display->drawPixel(X, Y + 3);

    m_Display->drawPixel(X + 4, Y + 1);  //Right
    m_Display->drawPixel(X + 4, Y + 2);
    m_Display->drawPixel(X + 4, Y + 3);

    m_Display->drawPixel(X + 1, Y + 4);  //Bottom
    m_Display->drawPixel(X + 2, Y + 4);
    m_Display->drawPixel(X + 3, Y + 4);

    m_Display->setFont(m_WeatherAlphFont);
    m_Display->drawUTF8(X + 7, Y - 1, "C");
}

void ScreenControl::DisplayTemprature(float Temprature, int16_t X, int16_t Y, bool bCalculateX)
{
    m_Display->setFont(m_WeatherNumFont);

    std::stringstream Stream;
    Stream << std::fixed << std::setprecision(1) << Temprature;
    std::string TempratureStr = Stream.str();

    int16_t PositionX = X;
    int16_t NumWidth = GetUTFWidth(TempratureStr);

    if(bCalculateX)
    {
        int16_t DegreeWidth, CelciusWidth = 0;

        m_Display->setFont(m_WeatherAlphFont);
        CelciusWidth = GetUTFWidth("C");

        DegreeWidth = 10;

        PositionX = CalculateX((NumWidth + DegreeWidth + CelciusWidth), true, true, 64);
    }

    m_Display->setFont(m_WeatherNumFont);
    m_Display->drawUTF8(PositionX, Y, TempratureStr.c_str());

    m_Display->setFont(m_WeatherAlphFont);
    DisplayDrawCelcius((PositionX + NumWidth), Y);
}

bool ScreenControl::DisplayWeatherCode(const std::string& WeatherCode, int16_t X, int16_t Y, bool bCalculateX)
{
    m_Display->setFont(m_WeatherAlphFont);
    int16_t CharWidth = GetUTFWidth(WeatherCode);

    //Only scroll if the weather code doesn't fit all on screen
    if(CharWidth > m_MaxTextWidth)
    {
        return DisplayScrollMessage(WeatherCode.c_str(), X, Y);
    }
    else
    {
        m_PreviousScrollText = WeatherCode;
        int16_t PositionX = X;

        if(bCalculateX)
        {
            int16_t TextWidth = GetUTFWidth(WeatherCode.c_str());
            PositionX = CalculateX(TextWidth, true, true, 64);
        }

        m_Display->setCursor(PositionX, Y);
        m_Display->println(WeatherCode.c_str());
    }

    return false;
}

void ScreenControl::DisplayWeatherIcon(const unsigned char* WeatherIcon, int16_t X, int16_t Y)
{
    //All icons are 64x48 in size
    m_Display->drawBitmap(X, Y, 8, 48, WeatherIcon);
}

void ScreenControl::ResetFont()
{
    m_Display->setFont(m_DefaultFont);
}

void ScreenControl::SetContrast(uint8_t Contrast)
{
    m_Display->setContrast(Contrast);
}

int16_t ScreenControl::CalculateX(int16_t TextWidth, bool bRightSide, bool bCenter, int16_t ClampCenter)
{
    int16_t OffsetX = 0;
    if(bCenter)
    {
        int16_t RemainingWidth = (SCREEN_WIDTH - ClampCenter) - TextWidth;
        OffsetX = RemainingWidth / 2;
    }

    if(bRightSide)
    {
        OffsetX = SCREEN_WIDTH - (TextWidth + OffsetX);
    }

    return OffsetX;
}

int16_t ScreenControl::GetUTFWidth(const std::string& String)
{
    return m_Display->getUTF8Width(String.c_str());
}