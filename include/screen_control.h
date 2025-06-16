#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1  // Reset pin
#define SCREEN_ADDRESS 0x3C

class ScreenControl
{
public:
    ScreenControl();
    ~ScreenControl();
    void Init();

    void DisplayTest();
    void DisplayWeekDay(const tm& timeinfo);
    void DisplayDate(const tm& timeinfo);
    void DisplayClearScreen();
    void Display();

    void DisplayMessage(const char* Msg, int16_t x, int16_t y);

    void DisplayConnecting();

    void DisplayDrawCelcius(int16_t x, int16_t y);
    void DisplayTemprature(float Temprature, int16_t x, int16_t y);
    void DisplayTimeHrMin(const tm& timeinfo, int16_t x, int16_t y, bool bFromRightSide);
    void DisplayIteration(int16_t x, int16_t y, bool bFromRightSide);
    void DisplayWeatherCode(const std::string& WeatherCode, int16_t x, int16_t y);

    void DisplayError();

    Adafruit_SSD1306* m_Display = nullptr;

private:
    int m_IterateCounter = 0;

};