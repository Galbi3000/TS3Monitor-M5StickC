/*
Note: This is not an actual source file. The whole file has been commented out to prevent errors.
The Arduino IDE would not let me include a text file as a tab so I went with this method!

TeamSpeakMonitorM5StickC Revisions

Current version 3.33

3.33 - Added a message on button press if nobody is logged in to the TS3 server. Without this it
       looked like the device was powered off because nothing happened!

3.32 - Made the title show all the time the screen is on.
     - Added a battery status indicator in the upper right corner of the display.
     - Reduced the screen on time to 30 seconds to save more power.
     - Recreated the screen power control system to use a single function to turn it on/off.
     - Changes to the message scroller: Now scrollMessage() will turn the screen on. The scroller
       routine in drawDisplay() will switch the screen off if there are no clients logged in when the
       scroller has finished.
     - The above two points have fixed the bug introduced in the previous version.
     - Known Issue: For some reason the battery data jumps to around 3500 when powered by USB but
       instantly drops to < 3000 when unplugged! I will have to look at another method of calculating
       the charge so it can show the battery filling up while plugged in instead of instantly full!

3.31 - Added feature to show a list of clients online after device bootup.
     - Changed button screen wakeup to also display the list of clients to save having to press
       the button twice.
     - Changes scroller colour to red for error messages.
     - Changed the logged in/out scroller messages to only display once. Now a list of clients
       can be called at will the scroller does not need a repeat in case the name is missed.
     - Fixed a bug where the screen would not come on as soon as the button was pressed after
       adding the client list on button press. The bug appeared after the button press client list
       feature was added. The wake on button press had been tested fully before adding that.
     - BUG: There is an introduced bug where the screen does not power off after waking up to show
       the last client logged out. I am having a hard time pinpointing this one without the ability
       to debug properly, using breakpoints and stepping through the code while looking at variables!

3.30 - Display switches off after 60 seconds of inactivity. The timer starts from when the screen
       is turned on or from when a scroller messages ends to make sure the screen can not turn off
       while a long message is being scrolled in case the timer has been reduced to 30 seconds for
       example or the slower character jump scroller is being used instead of the pixel smooth one!
     - Now wakes the display on button press.
     - Added a scroller message of a list of clients logged in when the button is press while the
       screen is on.

Bugs:  Before this update, after starting up with clients already on the TS3 server it would think
       they had just logged in and so scroll a list of names in the normal way when people log in.
       For some reason this does not happen anymore! I could take advantage of this and instead of
       showing everyone just logged in, display a list of names in the same way as when the button
       is pressed.
     
3.23 - Display now switching off when there are no clients. Setting the brightness did nothing so I
       had to quickly learn how to turn the display off properly using the AXP chip in the device.
       Now I know how to do this it will be a simple matter of implimenting the display off timer
       with wake on button press. It already wakes on client updates or TS3 connection errors with
       this update!

3.22 - Fixed smooth scrolling pausing during calls to the TeamSpeak server. I chose to simply call
       updateDisplay() during loops in the TeamSpeak functions. This meant re-ordering the include
       statements to make sure Display.h is included before TeamSpeakFunctions.h.
       I went against using the second core as I am not experienced enough yet to do dual core on
       the ESP32! The method I used works fine anyway.
     - Added a pause() function into Display.h that does the same as delay() but also updates the
       display while waiting for the specified milliseconds.
     - Set the display brightness (backlight) to 0 while the screen is blank when there are no
       clients on the server. Resets back to 96 when there is something to be displayed. Hopefully
       this reduces the battery use until I can impliment a means to turn the display off after a
       timeout and back on with a button press or client number changes.

3.21 - Added colour to the display. Title in cyan, boot status messages in white with yellow
       highlights, finished message in green. Main display title cyan, client count white,
       scroller yellow and error count in red. 

3.20 - Added a smooth scroll techneque from the LovyanGFX examples to the message scroller
       routines. Changing USE_SMOOTH_SCROLL to 0 reverts back to the old routine.
     - Cleaned up code to remove non-functional timer routines ready for going dual core.
     - A missed issue where the scroller for clients logging in repeats for clarity but logging
       out did not, fixed now so that both have repeats.

3.10 - Added device orientation detection to flip the screen depending on which way you are holding
       the M5StickC. The startup routine does read the orientation once so the startup screen is
       displayed correctly but it will not flip during startup if you turn the device over. This is
       only done live during the main display in the loop function.

3.00 - Duplicated original TeamSpeakMonitor project to create TeamSpeakMonitorM5StickC
     - Switched to using LovyanGFX library instead of AdafruitGFX. It's faster, has more features,
       and is compatible with the M5StickC & other devices.
     - Fixed a flicker issue with the screen redraw. Now it only redraws the whole screen when a
       client enters or leaves the TeamSpeak server.
     - Modified the scroller to erase itself on each update so it's not relying on the clear
       screen function of the LCD. This makes the scroller flicker but not the rest of the screen.
     - Renamed OLEDDisplay.h to just Display.h as the M5StickC is using a colour TFT display!


TeamSpeakMonitor revisions:

2.32 - Fixed a bug in the code for checking names logged out. It was using the wrong list
       of names to create the scroll message.
     - Fixed a bug in the code for checking names logged in. It was comparing names from the
       same list! Amazing how it still showed new names though!
     - Added a bit of clean up to the lists of names so that the list always contains empty
       strings after the last name.

2.31 - Added a line in the scroller routine to also reset the scrollerPos when setting 
       currentScroller to the next message.
     - Added commas between names in scroll message when more than one person logs in/out.

2.30 - Added scroller message indicating who logs in or out of the TeamSpeak 3 server.

2.20 - Moved global variables and functions into Globals.h
     - Moved the HTML Server functions to HTMLFunctions.h
     - Moved the TS3 Telnet functions to TEAMSPEAKFunctions.h
     - Moved the OLED Display functions to Display.h
     The above changes tidy the main file to just the setup() and loop() functions and
     makes it easier to find functions that may need changing.

2.15 - Moved the display refresh routine into a timer call-back to prevent visible
       freezes in the scrolling messages while the server is being queried.
       This turns out to be buggy so added optional compiler switch DISPLAY_ON_TIMER.

2.10 - Tweaked the display content using different fonts.
     - Added message scroller code ready for displaying who logged in/out.
     - Put display refresh (redraw) in own 'timer' for scroller speed control. 
       NOTE: See BUGS section at the end of this file for timer bug details.

2.00 - Added an OLED screen to the hardware side of the project. Currently using the
       Adafruit libraries which have ESP8266 support.
     - LED pin reassigned to prevent conflict with the OLED data pins.
     - Added code to display start-up progress to the OLED display.
     - Added code to display the number of connected clients on the TS server on the
       OLED screen. When no clients are connected the screen goes blank.

1.01 - Modified the Initial code for the NodeMCU device that the sketch is now used on.
     - Changed the TeamSpeak server web page to the root page.

1.00 - Initial version consisting of code copied from another sketch which provided the
       web server and LED control as well as the TeamSpeak code.
     - Removed the unrelated code for a test page controlling some LEDs.
       
TODO:

- Impliment use of the second ESP32 core for updating the display. I may get round to this
  but it's not high importance now the scroller does not pause.
- See if the LED brightness can be reduced to save power.

KNOWN BUG:

Names with extended characters like é, è, ö, etc. do not display correctly.
This seems to be an issue with the way the WiFi libraries process telnet data, perhaps telnet does
not permit these characters!

I am unable to fix this as it's not this sketch doing it.

*/
