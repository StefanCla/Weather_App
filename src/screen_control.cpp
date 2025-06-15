#include "screen_control.h"
#include <time.h>
#include <sstream>
#include <iomanip>

ScreenControl::ScreenControl()
{
    m_Display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    Init();
}

ScreenControl::~ScreenControl()
{
    delete m_Display;
}

void ScreenControl::Init()
{
    Serial.begin(9600);

    // initialize the OLED object
    if (!m_Display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
        ;  // Don't proceed, loop forever
    }

    //Lower brightness
    m_Display->ssd1306_command(0x81); 
    m_Display->ssd1306_command(1);

    m_Display->clearDisplay();
}

void ScreenControl::DisplayWeekDay(const tm& timeinfo)
{
    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->setCursor(0, 5);

    char buffer[10];
    strftime(buffer, 10, "%A", &timeinfo);
    m_Display->println(buffer);
    m_Display->display();
}

void ScreenControl::DisplayDate(const tm& timeinfo)
{
    char buffer[10];
    strftime(buffer, 10, "%e-%b-%y", &timeinfo);

    short int x1, y1;
    short unsigned int w, h;
    m_Display->getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

    int offsetX = SCREEN_WIDTH - w;
    
    m_Display->setCursor(offsetX, 5);
    m_Display->println(buffer);
    m_Display->display();
}

void ScreenControl::DisplayTest()
{
    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->setCursor(0,16);
    m_Display->println("HEY");
    m_Display->display();
}

void ScreenControl::DisplayClearScreen()
{
    m_Display->clearDisplay();
}

void ScreenControl::DisplayDrawCelcius(int16_t x, int16_t y)
{
    //Drawing ASCII degree looks weird
    //Thus we do it manually

    x += 1; //Add slight offset

    //draw small circle
    m_Display->drawPixel(x + 1, y, 1);  //Top
    m_Display->drawPixel(x + 2, y, 1);

    m_Display->drawPixel(x, y + 1, 1);  //Left
    m_Display->drawPixel(x, y + 2, 1);

    m_Display->drawPixel(x + 3, y + 1, 1);  //Right
    m_Display->drawPixel(x + 3, y + 2, 1);

    m_Display->drawPixel(x + 1, y + 3, 1);  //Bottom
    m_Display->drawPixel(x + 2, y + 3, 1);

    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);

    m_Display->setCursor(x + 5, y);
    m_Display->println("C");
    m_Display->display();
}

void ScreenControl::DisplayTemprature(float Temprature, int16_t x, int16_t y)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << Temprature;
    std::string NewTemp = stream.str();

    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->setCursor(x, y);
    m_Display->println(NewTemp.c_str());
    m_Display->display();

    short int x1, y1;
    short unsigned int w, h;
    m_Display->getTextBounds(NewTemp.c_str(), 0, 0, &x1, &y1, &w, &h);

    DisplayDrawCelcius(w, y);

    //m_Display->drawBitmap(0, 32, epd_bitmap_wi_day_sunny_overcast, 32, 32, 1);
    m_Display->display();
}

void ScreenControl::DisplayTimeHrMin(const tm& timeinfo, int16_t x, int16_t y, bool bFromRightSide)
{
    char buffer[10];
    strftime(buffer, 10, "%R", &timeinfo);

    if(bFromRightSide)
    {
        int16_t x1, y1;
        uint16_t w, h;

        m_Display->getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);

        x = SCREEN_WIDTH - (w + x);
    }

    m_Display->setCursor(x, y);
    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->println(buffer);
    m_Display->display();
}

void ScreenControl::DisplayIteration(int16_t x, int16_t y, bool bFromRightSide)
{
    if(bFromRightSide)
    {
        int16_t x1, y1;
        uint16_t w, h;
        std::string TempString = std::to_string(m_IterateCounter);
        m_Display->getTextBounds(TempString.c_str(), 0, 0, &x1, &y1, &w, &h);

        x = SCREEN_WIDTH - (w + x);
    }

    m_Display->setCursor(x, y);
    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->println(m_IterateCounter);
    m_Display->display();

    m_IterateCounter++;
}

void ScreenControl::DisplayWeatherCode(const std::string& WeatherCode, int16_t x, int16_t y)
{
    m_Display->setCursor(x, y);
    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->println(WeatherCode.c_str());
    m_Display->display();
}

void ScreenControl::DisplayError()
{
    std::string ErrorMsg = "An Error has occured...";

    m_Display->clearDisplay();
    m_Display->setCursor(0, 16);
    m_Display->setTextSize(1);
    m_Display->setTextColor(WHITE);
    m_Display->println(ErrorMsg.c_str());
    m_Display->display();
}