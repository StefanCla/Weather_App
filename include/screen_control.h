#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

class ScreenControl
{
public:
    ScreenControl();
    ~ScreenControl();

    void Display();
    void DisplayClearScreen();

    void DisplayMessage(const char* Msg, int16_t x, int16_t y);

    void DisplayWeekDay(const tm& timeinfo);
    void DisplayDate(const tm& timeinfo);
    void DisplayTimeHrMin(const tm& timeinfo, int16_t x, int16_t y, bool bFromRightSide);

    void DisplayDrawCelcius(int16_t x, int16_t y);
    void DisplayTemprature(float Temprature, int16_t x, int16_t y);
    void DisplayWeatherCode(const std::string& WeatherCode, int16_t x, int16_t y);
    void DisplayWeatherIcon(const unsigned char* WeatherIcon);
    
    //For debug
    void DisplayIteration(const int Iterate, int16_t x, int16_t y, bool bFromRightSide);

    void ResetFont();

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* m_Display;

private:
    const uint8_t* m_WeatherNumFont = u8g2_font_victoriabold8_8n;
    const uint8_t* m_WeatherAlphFont = nullptr;

    const uint8_t* m_DefaultFont = u8g2_font_6x10_mf;

};