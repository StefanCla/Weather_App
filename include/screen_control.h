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

    int16_t Height = 0;
    int16_t Width = 0;
};

class ScreenControl
{
public:
    ScreenControl();
    ~ScreenControl();

    void Display();
    void DisplayClearScreen();

    void DisplayMessage(const char* Msg, int16_t x, int16_t y);
    bool DisplayScrollMessage(const char* Msg, int16_t x, int16_t y);

    void DisplayWeekDay(const tm& timeinfo);
    void DisplayDate(const tm& timeinfo);
    void DisplayTimeHrMin(const tm& timeinfo, int16_t x, int16_t y, bool bFromRightSide);

    void DisplayDrawCelcius(int16_t x, int16_t y);
    void DisplayTemprature(float Temprature, int16_t x, int16_t y);
    bool DisplayWeatherCode(const std::string& WeatherCode, int16_t x, int16_t y);
    void DisplayWeatherIcon(const unsigned char* WeatherIcon);
    
    void ResetFont();

    inline int16_t GetMaxTextWidth() const { return m_MaxTextWidth; }
    int16_t GetUTFWidth(const std::string& String);

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* m_Display;

private:
    const uint8_t* m_WeatherNumFont = u8g2_font_victoriabold8_8n;
    const uint8_t* m_WeatherAlphFont = nullptr;

    const uint8_t* m_DefaultFont = u8g2_font_6x10_mf;

    const int16_t m_MaxTextWidth = 64;
    std::map<const char*, Scroller*> m_ScrollMap;

};