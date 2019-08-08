/**
 *  Demo of using ESPEasyCfg as a trivial captive portal
 *  This captive portal will help you to set the SSID and
 *  password of your WiFi network, without hard-coding it.
 *  Additionnaly, you can set some custom parameters to be used
 *  /!\ The data folder (located with this library), must be copied to
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
    //Add custom HTTP handlers to serve our custom web pages
    //If we want to add a handler on web site root (i.e. http://xxx.xxx.xxx.xxx/)
    //we need to use the setRootHandler method (because it's internally used)
    captivePortal.setRootHandler([](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Hello root!");
    });
    //Other routes can be added directly by using the AsyncWebServer object:
    server.on("/hello", HTTP_GET, [=](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Hello from hello!");
    });
    
    //Start our captive portal (if not configured)
    //At first usage, you will find a new WiFi network named "MyThing"
    captivePortal.begin();
    //Serve web pages
    server.begin();
}

void loop()
{
    //Nothing to do here, the configuration is done in a parallel task (ESP32)
    delay(1000);
}
