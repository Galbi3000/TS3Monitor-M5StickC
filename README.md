# TS3Monitor-M5StickC

This project is the Arduino IDE code for programming an M5StickC (ESP32 based) to monitor a TeamSpeak 3 server.

It is a port from my TeamSpeak-3-Monitor-ESP8266 project with changes made to take advantage of features of the M5StickC and the ESP32 in general.

It creates a simple web page accessed through port 90 on the device's IP address (accessible through a browser with an address like: 192.168.1.44:90 ) that shows the simple layout of the server's channels and the clients logged in to them.
The M5StickC has a built in LED which is used for indicating that the server has people logged in. It also has a built in colour TFT display which is used to show a count of clients as well as the names of people who log in or out via a scrolling ticker-tape style message on the screen.

The new features added include the following:
- Device rotation detection. Flips the screen when the M5StickC is turned over.
- Smooth scroll messages. Instead of scrolling one character at a time it now scrolls pixel by pixel.
- Colour display instead of monochrome.

Planned features in comming updates:
- Dual core functionality to make the display and WiFi routines run on different cores.
- Screen off timer to save power. To wake up on button press or client number update from the server.
- A battery status indicator in the display.

How to use:
- Open up the Arduino IDE and chose from the file menu "Open..."
- Navigate to where you downloaded the Git project and in the folder "TeamSpeakMonitorM5StickC" select the .ino file of the same name as the folder.
- Go to the Globals.h tab and replace "your-ssid" with the ID of your WiFi connection and replace "your-password" with the password.
- Further down in the file change telnetHost address to the address of your TS3 Server.
- Next edit the queryLogin and queryPass entries to match the credentials given to you by your TS3 server for remote queries.
- Make sure the correct device is selected in the Tools menu and its correct COM port is selected.
- Compile and upload the sketch.

Here is a photo of the M5StickC running the sketch:
![image](https://github.com/Galbi3000/TS3Monitor-M5StickC/blob/main/TS3Monitor-M5StickC.jpg)
