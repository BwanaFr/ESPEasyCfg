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
 * Our settings to be added to the captive portal
 * page. These settings are saved by ESPEasyCfg.
 * Once configured, you can still reach the page
 * on http://XXX.XXX.XXX.XXX/ESPEasyCfg/config.html
 */
ESPEasyCfgParameterGroup mqttParamGrp("MQTT");
ESPEasyCfgParameter<String> mqttServer("mqttServer", "MQTT server", "server.local");
ESPEasyCfgParameter<String> mqttUser("mqttUser", "MQTT user", "user");
ESPEasyCfgParameter<String> mqttPass("mqttPass", "MQTT password", "");
ESPEasyCfgParameter<int> mqttPort("mqttPort", "MQTT port", 1883);

/**
 * Shows actual MQTT parameters
 */
void printMQTTParameters()
{
  Serial.print("MQTT server :");
  Serial.println(mqttServer.getValue());
  Serial.print("MQTT user :");
  Serial.println(mqttUser.getValue());
  Serial.print("MQTT pass :");
  Serial.println(mqttPass.getValue());
  Serial.print("MQTT port :");
  Serial.println(mqttPort.getValue());
}

/**
 * Callback called when captive portal state change
 */
void stateCallback(ESPEasyCfgState state)
{
  switch(state){
    case ESPEasyCfgState::Connecting:
      Serial.print("Connecting to WiFi");
      break;
    case ESPEasyCfgState::AP:
      Serial.println("In captive portal mode");
      break;
    case ESPEasyCfgState::Connected:
      Serial.println("WiFi network connected");
      break;
    case ESPEasyCfgState::WillConnect:
      Serial.println("WiFi network connection planned");
      break;
    case ESPEasyCfgState::Reconfigured:
      Serial.println("Parameters changed");
      //Here we can use this state to reconfigure our software
      printMQTTParameters();
      break;
  }
}

/**
 * Simple example of how to use the captive portal
 **/
void setup()
{
    Serial.begin(115200);
    //Configure our parameters for adding them
    //The MQTT password is a password HTML type
    mqttPass.setInputType("password");
    //Add created parameters to the MQTT parameter group
    mqttParamGrp.add(&mqttServer);
    mqttParamGrp.add(&mqttUser);
    mqttParamGrp.add(&mqttPass);
    mqttParamGrp.add(&mqttPort);
    //Finally, add our parameter group to the captive portal
    captivePortal.addParameterGroup(&mqttParamGrp);

    //Register the callback to be notified when the captive portal
    //state change
    captivePortal.setStateHandler(stateCallback);
    
    //Start our captive portal (if not configured)
    //At first usage, you will find a new WiFi network named "MyThing"
    captivePortal.begin();
    //Serve web pages
    server.begin();

    //After having called begin() on the captive portal
    //Our parameter are loaded from SPIFFS with latest configured values
    printMQTTParameters();
    //So here, we can configure our MQTT client, for example.
}

void loop()
{
    //Nothing to do here, the configuration is done in a parallel task (ESP32)
    delay(1000);
}
