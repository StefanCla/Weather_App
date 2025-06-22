#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <map>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

struct Scroller
{
    bool bDirection = false; //True = ->, False = <-
    int16_t ScrollOffsetX = 0;
};

class ScreenControl
{
public:
    ScreenControl();
    ~ScreenControl();

    void Display();
    void DisplayArea(uint8_t TileX, uint8_t TileY, uint8_t TileXCount, uint8_t  TileYCount);
    void DisplayClearScreen();

    void DisplayMessage(const char* Msg, int16_t X, int16_t Y);
    bool DisplayScrollMessage(const char* Msg, int16_t X, int16_t Y);

    void DisplayWeekDay(const tm& TimeInfo);
    void DisplayDate(const tm& TimeInfo);
    void DisplayTimeHrMin(const tm& TimeInfo, int16_t X, int16_t Y, bool bCalculateX);

    void DisplayDrawCelcius(int16_t X, int16_t Y);
    void DisplayTemprature(float Temprature, int16_t X, int16_t Y, bool bCalculateX);
    bool DisplayWeatherCode(const std::string& WeatherCode, int16_t X, int16_t Y, bool bCalculateX);
    void DisplayWeatherIcon(const unsigned char* WeatherIcon);
    
    void ResetFont();
    void SetContrast(uint8_t Contrast);

    //Used to calculate the X position via string & parameters, as opposed to manually setting the position
    //bRightSide = start from right side, bCenter = center text, ClampCenter = clamp the space on which it centers in pixels
    int16_t CalculateX(int16_t TextWidth, bool bRightSide, bool bCenter, int16_t ClampCenter);

    inline int16_t GetMaxTextWidth() const { return m_MaxTextWidth; }
    int16_t GetUTFWidth(const std::string& String);

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* m_Display = nullptr;

    const uint8_t* m_WeatherNumFont = u8g2_font_courB10_tn;
    const uint8_t* m_WeatherAlphFont = u8g2_font_BBSesque_tr;
    const uint8_t* m_DefaultFont = u8g2_font_6x10_mf;

    const int16_t m_MaxTextWidth = 64;
    std::map<const char*, Scroller*> m_ScrollMap;
    std::string m_PreviousScrollText = "";
};