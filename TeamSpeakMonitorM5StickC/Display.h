#include <SPI.h>
#include <Wire.h>

#define LGFX_M5STICK_C
#include <LovyanGFX.hpp>

#define TITLE_FONT fonts::FreeSerif9pt7b

#define OLED_RESET LED_BUILTIN  //4
LGFX lcd;
LGFX_Sprite scrollerCanvas, batteryCanvas;

boolean screenPower;

void setScreenPower(boolean power)
{
  if (power == screenPower)
    return;

  screenPower = power;

  M5.Axp.SetLDO2(power);
  M5.Axp.SetLDO3(power);

  if (power)
    displayOnTime = millis();
}

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
  setScreenPower(true);
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
  setScreenPower(true);
}
#endif

void initBatDisplay()
{
  batteryCanvas.setColorDepth(8);
  batteryCanvas.createSprite(25,15);
}

void drawBattery()
{
  batteryCanvas.fillRect(0, 0, 24, 14, BLACK);
  batteryCanvas.drawRect(0, 0, 22, 9, WHITE);
  batteryCanvas.drawPixel(21, 0, LIGHTGREY);
  batteryCanvas.drawPixel(21, 8, LIGHTGREY);
  batteryCanvas.drawLine(22, 3, 22, 5, WHITE);
  // Don't draw anything else if the battery is dead!
  if (batteryCharge > 0)
  {
    int colour;
    if (batteryCharge > 75)
      colour = GREEN;
    else if (batteryCharge > 50)
      colour = YELLOW;
    else if (batteryCharge > 25)
      colour = ORANGE;
    else if (batteryCharge > 0)
      colour = RED;
  
    batteryCanvas.fillRect(1, 1, batteryCharge/5, 7, colour);
  }

  // Paint the image to the main display
  batteryCanvas.pushSprite(&lcd, lcd.width() - 26, 4);
}

void drawDisplay()
{
  if (redrawAll)
  {
    redrawAll = 0;
    lcd.clearDisplay();
    lcd.setFont(&TITLE_FONT);
    lcd.setTextSize(1);
    lcd.setCursor(2,2);
    lcd.setTextColor(CYAN);   // Set the text colour to cyan.
    lcd.println("TS3 Monitor");
    lcd.setTextColor(WHITE);
    
    if (numClients)
    {
      lcd.setCursor(0,24);
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
      lcd.setTextColor(RED);
      lcd.print("E");
      lcd.print(errorCount);
    }
    lcd.setFont(&fonts::Font0);
    lcd.setTextSize(2);
  }

  #if USE_SMOOTH_SCROLL
  if (showScroller)
  {
    if (errorCount)
      scrollerCanvas.setTextColor(RED);
    else
      scrollerCanvas.setTextColor(YELLOW);
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
          scrollerCanvas.clear(BLACK);
          if (numClients == 0)
            setScreenPower(false);
          else
            displayOnTime = millis();
        }
      }
    }
    scrollerCanvas.pushSprite(&lcd, 0, 54);
  }
  #else // Use old character jump scroller method
  if (showScroller)
  {
    if (errorCount)
      lcd.setTextColor(RED);
    else
      lcd.setTextColor(YELLOW);
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
        if (numClients == 0)
          setScreenPower(false);
        else
          displayOnTime = millis();
      }
    }
    lcd.fillRect(0, 54, lcd.width(), lcd.height()-54, TFT_BLACK);
    lcd.setTextSize(2);
    lcd.setCursor(0,56);
    lcd.print(scrollerDisplay);
  }
  #endif
}

void updateDisplay()
{
  if(displayEnabled)
  {
    if (millis() - timeoutDisplay > DISPLAY_REFRESH)
    {
      drawDisplay();
      timeoutDisplay = millis();
    }
    drawBattery();
  }
  if (millis() - displayOnTime > POWER_SAVE_INTERVAL && screenPower && !showScroller)
    setScreenPower(false);
}

void pause(unsigned long pauseTime)
{   // Same as delay() but also updates the display while waiting
  unsigned long pauseEnd = millis() + pauseTime;
  while (millis() < pauseEnd)
    updateDisplay();
}
