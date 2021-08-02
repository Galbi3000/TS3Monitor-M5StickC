/*
 *  This sketch is designed for am M5StickC device and provides a simple web server 
 *  on port 90 which gives the user a web page on which they can see the channel list for 
 *  the TeamSpeak 3 server as well as any people logged in to the channels.
 *
 *  The GFX library LovyanGFX is required also and can be included from the Library Manager
 *  or found at: https://github.com/lovyan03/LovyanGFX
 *  
 *  I chose to use this library instead of the M5 built in LCD library because it has more
 *  features like sprites (used for smooth text scroller eventually) and is faster. It's
 *  only downside is that most of the documentation is in Japanese, especially with the
 *  comments in the examples.
 *
 *  The sketch will light up the internal LED connected to GPIO 10 when there are users
 *  logged in to the TeamSpeak 3 server. It can optionally ignore certain usernames for
 *  the LED. For example I made it ignore all names I log in with on my TS3 server so that 
 *  it is not telling me I am logged in as I already know I am! See commented out section
 *  for more details on this feature.
 *  
 *  Finally the display built in to the M5StickC is used to display a count of users
 *  connected to the TeamSpeak 3 server as well as scroll the name of a user who has logged
 *  in or out of the TS3 server.
 *
 *  See Revisions tab for details of version changes.
 */

#define SERIAL_ON 0
//#define DISPLAY_ON_TIMER 0
#define USE_SMOOTH_SCROLL 1

#include "Globals.h"
#include "TeamSpeakFunctions.h"
#include "HTMLFunctions.h"
#include "Display.h"

void setup()
{

  // Start the serial connection
  #if SERIAL_ON
  Serial.begin(115200);
  #endif

  // Initialise the LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 1);
  LEDState = 1;

  // initialize the M5StickC object
  M5.begin();
  M5.IMU.Init();
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  sprintln("");
  sprint("IMU AccelData: X=");
  sprint(String(accX));
  sprint("Y=");
  sprint(String(accY));
  sprint("Z=");
  sprintln(String(accZ));

  if (accX > 0.4 && screenRotation != 1)
    screenRotation = 1;
  else if (accX < -0.4 && screenRotation != 3)
    screenRotation = 3;
  
  sprintln("");
  sprint("Screen rotation: ");
  sprintln(String(screenRotation));

  lcd.init();
  lcd.setRotation(screenRotation);
  lcd.setColorDepth(8);
  lcd.setBrightness(96);
  
  #if USE_SMOOTH_SCROLL
  initScroller();
  #endif

  lcd.clearDisplay();           // Clears the display buffer
  lcd.setTextSize(2);           // Set the text size to 2
  lcd.setTextColor(TFT_CYAN);   // Set the text colour to cyan.
  lcd.setFont(&TITLE_FONT);
  lcd.setTextSize(1);
  lcd.setCursor(2,2);
  lcd.println("TS3 Monitor");
  lcd.setFont(&fonts::Font0);
  lcd.setCursor(0,22);
  lcd.display();                // Refresh the OLED display to show the changes made
  lcd.setTextColor(TFT_WHITE);


  // Connect to the WiFi router
  WiFi.begin(ssid, password);

  // Output connecting message to serial
  sprintln("");
  sprint("Connecting to WiFi");

  // Output connecting message to the OLED display
  lcd.setTextSize(1);
  lcd.println(" Connecting to WiFi..");
  lcd.display();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    sprint(".");
  }

  // Provide connection details to serial output
  sprintln("");
  sprint("Connected to ");
  sprintln(ssid);
  sprint("IP address: ");
  //sprintln(String(WiFi.localIP()));
  sprintln(WiFi.localIP().toString());

  // Display connection status on OLED screen
  lcd.print(" Connected to ");
  lcd.setTextColor(TFT_YELLOW);
  lcd.println(ssid);
  lcd.setTextColor(TFT_WHITE);
  lcd.display();
  lcd.print(" Address: ");
  lcd.setTextColor(TFT_YELLOW);
  lcd.println(WiFi.localIP().toString());
  lcd.setTextColor(TFT_WHITE);

  // HTTP Server
  
  // Set up the web page handlers
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  
  // Here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"};
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);

  // Ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize);

  server.begin();
  sprintln("HTTP server started");
  sprintln("");

  // Display the HTTP server status on the OLED screen
  lcd.println(" HTTP server started.");
  lcd.display();

  // Indicate connecting to the TS3 server on the OLED screen
  lcd.println(" Connecting to TS3.");
  lcd.display();

  // Telnet Client for TeamSpeak 3 ServerQuery
  loginServerQuery();

  timeoutChan = millis() + channelRefresh;   // Set the timeout for Channel list refresh so that it's run once at start
  timeoutClient = millis() + clientRefresh;  // Do the same for client list refreshing
  errorCount = 0;
  reconnectCount = 0;

  // Display a finished message on the OLED screen and wait 3 seconds to allow reading of the screen
  lcd.setTextColor(TFT_GREEN);
  lcd.println(" Finished setup.");
  lcd.display();
  delay(3000);

//  scrollerMessage = "          This is a test scroll message.          ";
//  showScroller = 3;

  redrawAll = 1;        // Make sure the display is redrawn fully on the first update
  #if DISPLAY_ON_TIMER
  if(!initDisplayTimer())   // If the display is on a timer interrupt then this will initialize the timer
  {
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_RED);
    lcd.println("Unable to set display timer!");
    lcd.println("Please recompile with");
    lcd.println("different timer settings.");
    while(true);  // Go into an endless loop, no point continuing at this point!
  }
  #endif

}

void loop()
{
  // HTTP first
  server.handleClient();  // Done, HTTP server handling is easy in loop()!

  // Refresh the channel and client lists from the TeamSpeak server (the functions handle the timing)
  refreshChannels();
  refreshClients();

  if (errorCount == 0)
    reconnectCount = 0;

  // If the number of clients has changed then see what names have been added/removed

  if (numClients < oldNumClients)       // Someone logged out
  {
    String message = "";
    for (int i = 0; i < oldNumClients; i++)
    {
      int found = 0;
      for (int j = 0; j < numClients; j++)
      {
        if (oldNames[i] == clients[j].clientName)
          found = 1;
      }
      if (!found)
      {
        if (message == "")
          message = oldNames[i];
        else
          message += ", " + oldNames[i];
      }
    }
    message += " logged out.";
    message += "........    " + message;
    scrollMessage(message);
  }
  else if (numClients > oldNumClients)  // Someone logged in
  {
    String message = "";
    for (int i = 0; i < numClients; i++)
    {
      int found = 0;
      for (int j = 0; j < oldNumClients; j++)
      {
        if (clients[i].clientName == oldNames[j])
          found = 1;
      }
      if (!found)
      {
        if (message == "")
          message = clients[i].clientName;
        else
          message += ", " + clients[i].clientName;
      }
    }
    message += " logged in.";
    message += "........    " + message;
    scrollMessage(message);
  }

  if (oldNumClients != numClients)
  {
    int i;
    for (i = 0; i < numClients; i++)
      oldNames[i] = clients[i].clientName;
    // Make sure the ends of the lists are empty, may cause issues if there are names still in place that shouldn't be there
    for (; i < maxClients; i++)
    {
      oldNames[i] = "";
      clients[i].clientName = "";
    }
    oldNumClients = numClients;
    redrawAll = 1;  // Redraw the display fully to update the client count.
  }

  M5.IMU.getAccelData(&accX,&accY,&accZ);

  if (accX > 0.4 && screenRotation != 1)
  {
    screenRotation = 1;
    lcd.setRotation(screenRotation);
    redrawAll = 1;  // Redraw the display fully to update the rotation.
    sprintln("");
    sprintln("Screen rotation: 1");
 }
  else if (accX < -0.4 && screenRotation != 3)
  {
    screenRotation = 3;
    lcd.setRotation(screenRotation);
    redrawAll = 1;  // Redraw the display fully to update rotation.
    sprintln("");
    sprintln("Screen rotation: 3");
  }

  // Update the display (the function handles timing)
  updateDisplay();

  if (errorCount >= 5)
  {
    sprintln("Attempting to regain communications with TeamSpeak Server.");
    loginOK = false;
    telnet.stop();
    loginServerQuery();
    errorCount = 0;
    reconnectCount += 1;
    scrollMessage("ERROR: Reconnecting to TS3 Server");

    if (WiFi.status() != WL_CONNECTED)
    {
      sprintln("Lost WiFi connection!");
      
      // Connect to the WiFi router
      WiFi.begin(ssid, password);
      
      // Wait for connection
      while (WiFi.status() != WL_CONNECTED)
      {
        // Output connecting message to serial
        sprintln("");
        sprint("Reconnecting to WiFi");
  
        delay(500);
        sprint(".");
      }  
      sprintln("");
    }
  }

  if (reconnectCount == 10)
  {
    sprintln("Tried reconnecting 10 times, rebooting!");
    ESP.restart();
  }

  int state = LEDState;
  if (clientCount == 0 && LEDState == 0)
    LEDState = 1;
  else if (clientCount > 0 && LEDState == 1)
    LEDState = 0;    
  if (state != LEDState)
    digitalWrite(LED, LEDState);
}
