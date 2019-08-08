/**
 *  Demo of using ESPEasyCfg as a trivial captive portal
 *  This captive portal will help you to set the SSID and
 *  password of your WiFi network, without hard-coding it.
 *  The data folder (located with this library), must be copied to
 *  SPIFFS. It contains all static web data to be served.
 */

#include <ESPEasyCfg.h>

/**
 * Reference to the AsyncWebServer object.
 * You can freely use this webserver to serve your pages.
 * If you need to use the root (/) web handler or a custom not found
 * handler, please use functions included in ESPEasyCfg
 */
AsyncWebServer server(80);

/**
 * Our captive portal class
 */
ESPEasyCfg captivePortal(&server);

/**
 * Simple example of how to use the captive portal
 **/
void setup()
{
    Serial.begin(115200);
    //we can provide a IO pin to ESPEasyCfg
    //this pin will blink according to what the captive portal is doing
    //captivePortal.setLedPin(BUILTIN_LED);

    //Additionnally, we can also wire a switch (active low)
    //if this pin is low during startup, the password will be reset
    //captivePortal.setSwitchPin(0);
    
    //Start our captive portal (if not configured)
    //At first usage, you will find a new WiFi network named "MyThing"
    captivePortal.begin();
    //Serve web pages
    server.begin();
}

void loop()
{
    //Nothing to do here, the configuration is done in a parallel task
    delay(1000);
}
