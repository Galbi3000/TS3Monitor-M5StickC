#include <SPI.h>
#include <Wire.h>

#define LGFX_M5STICK_C
#include <LovyanGFX.hpp>

//#include "FreeSerif9pt7b.h"
//#define TITLE_FONT FreeSerif9pt7b
#define TITLE_FONT fonts::FreeSerif9pt7b

#define OLED_RESET LED_BUILTIN  //4
LGFX lcd;
LGFX_Sprite scrollerCanvas;

/*
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
*/

#if USE_SMOOTH_SCROLL
int32_t cursor_x;

void initScroller()
{
  scrollerCanvas.setColorDepth(8);
  scrollerCanvas.setFont(&fonts::Font0);
  scrollerCanvas.setTextSize(2);
  scrollerCanvas.setTextColor(TFT_YELLOW);
  scrollerCanvas.setTextWrap(false);
  scrollerCanvas.createSprite(lcd.width()+46,54);
}

void scrollMessage(String message)
{
  if (scrollerMessage == "")
  {
    scrollerMessage = " "; // One leading space to make sure the message scrolls in smoothly instead of the first pixel jumping in!
    showScroller++;
  }
  scrollerMessage += message;
  scrollerMessage += "              ";
}

#else  // The old scroller routine needs buffers of spaces begining and end of the message to scroll on and off the screen
void scrollMessage(String message)
{
  if (scrollerMessage == "")
  {
    scrollerMessage = "              ";
    showScroller++;
  }
  scrollerMessage += message;
  scrollerMessage += "              ";
}
#endif

void drawDisplay()
{
  displayUpdating = 1;
  if (redrawAll)
  {
    redrawAll = 0;
    lcd.clearDisplay();
    lcd.setFont(&TITLE_FONT);
    lcd.setTextSize(1);
    if (numClients)
    {
      lcd.setCursor(2,2);
      lcd.setTextColor(TFT_CYAN);   // Set the text colour to cyan.
      lcd.println("TS3 Monitor");
      lcd.setCursor(0,24);
      lcd.setTextColor(TFT_WHITE);
      lcd.print(numClients);
      lcd.print(" client");
      if(numClients > 1)
        lcd.print("s");
      else
        lcd.print("");
    }
    if (errorCount)
    {
      lcd.setCursor(2,2);
      lcd.println("TS3 Monitor");
      lcd.setCursor(100,24);
      lcd.setTextColor(TFT_RED);
      lcd.print("E");
      lcd.print(errorCount);
    }
    lcd.setFont(&fonts::Font0);
    lcd.setTextSize(2);
  }

  #if USE_SMOOTH_SCROLL
  if (showScroller)
  {
    if (currentScroller == "")
    {
      currentScroller = scrollerMessage;
      scrollerMessage = "";
      scrollerPos = 0;
    }

    cursor_x = scrollerCanvas.getCursorX() - 1;
    if (cursor_x <= 0)
    {
      scrollerPos = 0;
      cursor_x = lcd.width();
    }

    scrollerCanvas.setCursor(cursor_x, 0);
    scrollerCanvas.scroll(-1, 0);

    while (scrollerPos < currentScroller.length() && cursor_x <= lcd.width())
    {
      scrollerCanvas.print(currentScroller.charAt(scrollerPos++));
      cursor_x = scrollerCanvas.getCursorX();

      if (scrollerPos == currentScroller.length())
      {
        showScroller--;
        scrollerPos = 0;
        if (scrollerMessage != "")
        {
          currentScroller = scrollerMessage;
          scrollerMessage = "";
        }
        if (!showScroller)
        {
          currentScroller = "";
          scrollerCanvas.clear(TFT_BLACK);
        }
      }
    }
    scrollerCanvas.pushSprite(&lcd, 0, 54);
  }
  #else // Use old character jump scroller method
  if (showScroller)
  {
    if (currentScroller == "")
    {
      currentScroller = scrollerMessage;
      scrollerMessage = "";
      scrollerPos = 0;
    }
    scrollerDisplay = currentScroller.substring(scrollerPos, scrollerPos + 13);
    scrollerPos++;
    if (scrollerPos == currentScroller.length()-13)
    {
      showScroller--;
      scrollerPos = 0;
      if (scrollerMessage != "")
      {
        currentScroller = scrollerMessage;
        scrollerMessage = "";
      }
      if (!showScroller)
      {
        currentScroller = "";
      }
    }
    lcd.fillRect(0, 54, lcd.width(), lcd.height()-54, TFT_BLACK);
    lcd.setTextSize(2);
    lcd.setCursor(0,56);
    lcd.print(scrollerDisplay);
  }
  #endif
  //lcd.display();
  displayUpdating = 0;
}

void updateDisplay()
{
  if (millis() - timeoutDisplay > displayRefresh)
  {
    drawDisplay();
    timeoutDisplay = millis();
  }
}
