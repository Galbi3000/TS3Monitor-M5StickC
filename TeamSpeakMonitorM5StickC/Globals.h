// GLOBALS.H
//
// All global variables.
//

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <M5StickC.h>

//
// WiFi variables
//

const char* ssid = "your-ssid";
const char* password = "your-pass";

//
// HTML Server variables
//

WebServer server(90);                  // The web server using port 90
String content;

//
// TeamSpeak3 Telnet variables and defines
//

// Support up to 128 channels and 32 clients. 32 clients is the max for a free TeamSpeak server.
// These can be increased as long as microcontroler has enough RAM!
#define SERVER_MAX_CHANNELS 128
#define SERVER_MAX_CLIENTS 32

// Refresh times for channel and client lists. (1000 = 1 second)
#define CHANNEL_REFRESH 30000
#define CLIENT_REFRESH 4000

typedef struct
{
  int ID;
  int parentID;
  int channelAboveID;
  String channelName;
} ts3Channel;

typedef struct
{
  int ID;
  int channelID;
  String clientName;
} ts3Client;

WiFiClient telnet;                            // The telnet client connection
const char* telnetHost = "192.168.0.110";     // The host address for the TeamSpeak 3 server
const int telnetPort = 10011;                 // The TeamSpeak 3 query server port
const char* queryLogin = "serveradmin";       // The login name for the query server
const char* queryPass = "server-pass";        // The login password

String okResponse = "error id=0 msg=ok";      // The query server's success response
int loginOK, errorCount = 0, reconnectCount = 0;

String ts3ServerName;

ts3Channel channels[SERVER_MAX_CHANNELS]; // List of channels on the server
ts3Client clients[SERVER_MAX_CLIENTS];    // List of clients connected to the server
String oldNames[SERVER_MAX_CLIENTS];      // Old list of names for tracking people leaving/joining the server

int numChannels;          // The number of channels on the server
int numClients;           // The number of clients logged in to the server
int oldNumClients;        // For tracking people leaving/joining the server
int clientCount;          // The number of clients that are not Glabi!

const int LED = 10;       // The LED used to indicate users logged on to TS.

int LEDlit = 0;
int LEDState;

unsigned long timeoutChan, timeoutClient;

//
// Display and message scroller variables and defines
//

// Refresh time for the OLED scroller display. (1000 = 1 second)
#if USE_SMOOTH_SCROLL
#define DISPLAY_REFRESH 10
#else
#define DISPLAY_REFRESH 200
#endif

#define POWER_SAVE_INTERVAL 30000     // Screen on for 30 seconds before shutting off (starts at end of message scrollers)
#define BATTERY_CHECK_INTERVAL 15000  // Check the battery state every 15 seconds

String scrollerDisplay, scrollerMessage, currentScroller;
int showScroller = 0; // Set this to the number of times a message should scroll past the screen.
int scrollerPos = 0;  // Position in the message for the scroller display.
unsigned long timeoutDisplay, timeoutBattery, displayOnTime;
int redrawAll = 1;
int screenRotation = 1;
boolean displayEnabled = false;
// The following 3 variables are for detecting the orientation movement of the M5StickC for setting
// the screen rotation variable above.
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
int batteryCharge;

//
// Global Functions
//

void sprint(String str)
{
  #if SERIAL_ON
  Serial.print(str);
  #endif  
}

void sprintln(String str)
{
  #if SERIAL_ON
  Serial.println(str);
  #endif  
}
