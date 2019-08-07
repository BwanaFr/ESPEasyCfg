#include "ESPEasyCfg.h"
#include <AsyncJson.h>
#include <ArduinoJson.hpp>

#ifdef ESP32
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#define WIFI_AUTH_OPEN ENC_TYPE_NONE
#include <FS.h>
#else
#error Platform not supported
#endif

#include "ESPEasyCfgParameterManagerJSON.h"
#include "ESPEasyCfgConfiguration.h"

#define CFG_VERSION "1.0.0"
#define UNUSED_PIN 0xFF

#ifdef ESP32
void ESPEasyCfgMonitorTask(void* instance)
{
    ESPEasyCfg* obj = reinterpret_cast<ESPEasyCfg*>(instance);
    obj->monitorState();
    vTaskDelete( NULL );
}
#endif

ESPEasyCfg::ESPEasyCfg(AsyncWebServer *webServer) :
    _webServer(webServer),
    _iotName("_iotName", "Thing name", "MyThing", "Name of this thing"),
    _iotPass("_iotPass", "IoT password", "", "Configuration password"),
    _wifiSSID("_wifiSSID", "WiFi SSID", "", "Name of the WiFi network"),
    _wifiPass("_wifiPass", "WiFi password", "", "Password of WiFi network"),
    _paramGrp("Global settings", &_iotName), _state(State::WillConnect),
     _cfgHandler(nullptr), _dnsServer(nullptr), _paramManager(nullptr),
     _lastCon(0), _ledPin(UNUSED_PIN), _switchPin(UNUSED_PIN)
{
    //Chain built-in parameters
    _iotName.setNextParameter(&_iotPass);
    _iotPass.setNextParameter(&_wifiSSID);
    _wifiSSID.setNextParameter(&_wifiPass);
    _iotPass.setInputType("password");
    _wifiPass.setInputType("password");
    _wifiSSID.setInputType("ssid");
    _iotName.setExtraAttributes("{\"required\":\"\"}");
}

ESPEasyCfg::ESPEasyCfg(AsyncWebServer *webServer, const char* thingName) : 
    ESPEasyCfg(webServer)
{
    _iotName.setValue(thingName);
}

ESPEasyCfg::~ESPEasyCfg()
{
    delete _cfgHandler;
    delete _dnsServer;
    delete _paramManager;
}

void ESPEasyCfg::toJSON(ArduinoJson::JsonArray& arr, ESPEasyCfgParameterGroup* first)
{
    ESPEasyCfgAbstractParameter* param = first->getFirst();
    //Create an entry in the array
    JsonObject paramCol = arr.createNestedObject();
    //Put name of the parameter group
    paramCol["name"] = first->getName();
    //Create array of parameters
    JsonArray paramArr = paramCol.createNestedArray("parameters");
    //Create JSON entry for each parameter in the group
    while(param != nullptr)
    {
        JsonObject obj1 = paramArr.createNestedObject();
        param->toJSON(obj1);
        const char* type = param->getInputType();
        if(type != nullptr){ obj1["type"] = type;}
        param = param->getNextParameter();
    }
    //Recursive call if a parameter group follow this one
    ESPEasyCfgParameterGroup* next = first->getNext();
    if(next != nullptr){
        toJSON(arr, next);
    }
}

void ESPEasyCfg::fromJSON(ArduinoJson::JsonObject& json, ESPEasyCfgParameterGroup* first, String& msg, int8_t& action)
{
    ESPEasyCfgAbstractParameter* param = first->getFirst();
    //Got through all parameters
    while(param != nullptr)
    {
        if(json.containsKey(param->getIdentifier())){
            const char* val = json[param->getIdentifier()];
            param->setValue(val, msg, action, true);
        }
        param = param->getNextParameter();
    }
    //Recursive call if a parameter group follow this one
    ESPEasyCfgParameterGroup* next = first->getNext();
    if(next != nullptr){
        fromJSON(json, next, msg, action);
    }
}

void ESPEasyCfg::begin()
{
    //Register parameter callback to validate/act when needed
    _wifiSSID.setValidator([=](ESPEasyCfgParameter<String> *param, String newValue, String &msg, int8_t& action) -> bool{
        if((newValue != param->getValue()) || (_state == State::AP))
        {
            if(newValue.length()>0){
                if(_state != State::Connected){
                    msg +=  "You will be disconnected from AP.";
                    action |= ESPEasyCfgAbstractParameter::CLOSE;
                }else{
                    msg +=  "Trying to connect to ";
                    msg += newValue;
                }
            }
            _state = State::WillConnect;
        }
        return false;
    });

    //Parameter handler. Default to ESPEasyCfgParameterManagerJSON if not specified
    if(_paramManager == nullptr){
        _paramManager = new ESPEasyCfgParameterManagerJSON();
    }
    _paramManager->init(&_paramGrp);
    //Load parameters from file
    _paramManager->loadParameters(&_paramGrp, CFG_VERSION);
    //Install HTTP handlers
    //Server static files
    SPIFFS.begin();
    //Libraries (JQuery, Bootstrap) static files
    _webServer->serveStatic("/www/", SPIFFS, "/www/")
                .setCacheControl("public, max-age=31536000").setLastModified("Mon, 04 Mar 2019 07:00:00 GMT");
    //Configuration webpage, we must keep the handler reference to enable/disable authentication
    AsyncStaticWebHandler &fileHandler = _webServer->serveStatic("/ESPEasyCfg/config.html", SPIFFS, "/ESPEasyCfg/config.html")
                .setCacheControl("public, max-age=31536000").setLastModified("Mon, 04 Mar 2019 07:00:00 GMT");
    _fileHandler = &fileHandler;
    //Root handling
    _webServer->on("/", HTTP_GET, [=](AsyncWebServerRequest *request){
        if(_state == State::AP){
            //In AP mode, we must serve the first page
            DebugPrintln("Captive portal redirected");
            request->send(200, "text/html", F("<!DOCTYPE html><html><body><script>location.replace(\"/ESPEasyCfg/config.html\");</script></body></html>"));
        }else{
            if(!_rootHandler){
                //No root handler installed, fall back to our configuration page
                if(!request->authenticate("admin", _iotPass.getValue().c_str()))
                    return request->requestAuthentication(_iotName.getValue().c_str());
                request->redirect(F("/ESPEasyCfg/config.html"));
            }else{
                _rootHandler(request);
            }
        }
    });

    //Gets the device configuration as JSON document
    _webServer->on("/config", HTTP_GET, [=](AsyncWebServerRequest *request){
        if((_state != State::AP) && (_iotPass.getValue().length()>0) && !request->authenticate("admin", _iotPass.getValue().c_str()))
            return request->requestAuthentication(_iotName.getValue().c_str());
        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","ESP Async Web Server");
        response->addHeader("Access-Control-Allow-Origin", "*");
        JsonObject& root = (JsonObject&)response->getRoot();
        JsonArray arr = root.createNestedArray("groups");
        toJSON(arr, &_paramGrp);
        response->setLength();
        request->send(response);
    });

    //Handler to receive new configuration
    _cfgHandler = new AsyncCallbackJsonWebHandler("/configPost", [=](AsyncWebServerRequest *request, JsonVariant &json){
        if((_state != State::AP) && (_iotPass.getValue().length()>0) && !request->authenticate("admin", _iotPass.getValue().c_str()))
            return request->requestAuthentication(_iotName.getValue().c_str());
        JsonObject jsonObj = json.as<JsonObject>();
        String str;
        int8_t action;
        fromJSON(jsonObj, &_paramGrp, str, action);

        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","ESP Async Web Server");
        response->addHeader("Access-Control-Allow-Origin", "*");
        JsonObject& root = (JsonObject&)response->getRoot();
        JsonArray arr = root.createNestedArray("groups");
        toJSON(arr, &_paramGrp);
        if(str.length()>0){
            root["message"] = str;
        }
        if(action != 0){
            root["action"] = action;
        }
        response->setLength();
        request->send(response);
        _paramManager->saveParameters(&_paramGrp, CFG_VERSION);
    });
    _webServer->addHandler(_cfgHandler);


    //Handler to scan networks
    _webServer->on("/scan", HTTP_GET, [=](AsyncWebServerRequest *request){
        if((_state != State::AP) && (_iotPass.getValue().length()>0) && !request->authenticate("admin", _iotPass.getValue().c_str()))
            return request->requestAuthentication(_iotName.getValue().c_str());
        int n = WiFi.scanNetworks();
        AsyncJsonResponse * response = new AsyncJsonResponse();
        response->addHeader("Server","ESP Async Web Server");
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Cache-Control", "max-age=10");
        JsonObject& root = (JsonObject&)response->getRoot();
        JsonArray arr = root.createNestedArray("networks");
        for (int i = 0; i < n; ++i) {
            JsonObject network = arr.createNestedObject();
            network["SSID"] = WiFi.SSID(i);
            network["RSSI"] = WiFi.RSSI(i);
            network["open"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            network["channel"] = WiFi.channel(i);
        }
		WiFi.scanDelete();
        response->setLength();
        request->send(response);
    });
    _webServer->onNotFound([=](AsyncWebServerRequest * request){
        if(_state == State::AP){
            DebugPrint("Requested :" );
            DebugPrintln(request->host());
            if(!request->host().startsWith(_iotName.getValue()) ||
                !request->host().startsWith(WiFi.softAPIP().toString())){
                String loc = "http://";
                loc += WiFi.softAPIP().toString();
                request->redirect(loc);
                DebugPrint("Redirected to :" );
                DebugPrintln(loc);
            }else{
                request->send(404, "text/plain", "Not found");
            }
        }else{
            DebugPrint("Send 404 error on ");
            DebugPrintln(request->url());
            if(_notFoundHandler){
                _notFoundHandler(request);
            }else{
                request->send(404, "text/plain", "Not found");
            }
        }
    });

    //Connect to WiFi
    if(_wifiSSID.getValue().length()>0){
        //Configuration already done, we must switch to AP mode and start
        WiFi.begin();
    }else{
        //Not configured, switch to AP mode
        switchToAP();
    }
#ifdef ESP32
    //Monitor the connection state using a dedicated FreeRTOS task
    xTaskCreate(ESPEasyCfgMonitorTask,   /* Task function. */
                    "ConMonitor",        /* String with name of task. */
                    4096,                /* Stack size in bytes. */
                    this,                /* Parameter passed as input of the task */
                    1,                   /* Priority of the task. */
                    NULL);               /* Task handle. */
#endif

}

ESPEasyCfg::State ESPEasyCfg::getState()
{
    return _state;
}

void ESPEasyCfg::runDNS()
{
    //Instantiate/start DNS server if not running
    if(_dnsServer == nullptr){
        _dnsServer = new DNSServer();
        _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
        _dnsServer->start(53, "*", WiFi.softAPIP());
    }
    _dnsServer->processNextRequest();
}

void ESPEasyCfg::stopDNS()
{
    if(_dnsServer != nullptr){
        //Stops the dns server
        _dnsServer->stop();
        delete _dnsServer;
        _dnsServer = nullptr;
    }
}

void ESPEasyCfg::setParameterManager(ESPEasyCfgParameterManager* manager)
{
    _paramManager = manager;
}

/**
 * Switch to AP mode
 */
void ESPEasyCfg::switchToAP()
{
    DebugPrintln("Switching to AP mode");
    WiFi.mode(WIFI_AP);
    if(_iotPass.getValue().length()>0){
        //Enable authentication on AP if a password is set
        WiFi.softAP(_iotName.getValue().c_str(), _iotPass.getValue().c_str());
    }else{
        //Open AP (factory or lazy)
        WiFi.softAP(_iotName.getValue().c_str());
    }
    _fileHandler->setAuthentication("", "");
    delay(100);
    DebugPrint("AP IP ");
    DebugPrintln(WiFi.softAPIP());
    _state = State::AP;
}

void ESPEasyCfg::switchToSTA()
{
    stopDNS();
    DebugPrint("Trying to connect to ");
    DebugPrintln(_wifiSSID.getValue());
    WiFi.mode(WIFI_STA);
    if(_wifiPass.getValue().length()>0){
        WiFi.begin(_wifiSSID.getValue().c_str(), _wifiPass.getValue().c_str());
    }else{
        WiFi.begin(_wifiSSID.getValue().c_str());
    }
    _state = State::Connecting;
}

#ifdef ESP32
void ESPEasyCfg::monitorState()
#else
void ESPEasyCfg::loop()
#endif	
{
    static unsigned long connectStart = 0;
    static unsigned long lastLedChange = 0;
    unsigned long ledTimeOn = 500;
    unsigned long ledTimeOff = 500;
#ifdef ESPEasyCfg_SERIAL_DEBUG
    static unsigned long lastPrint = 0;
#endif
    static bool ledState = false;
#ifdef ESP32
    while(true){
        esp_task_wdt_reset();
#endif		
        unsigned long now = millis();
        switch(_state){
            case State::Connecting:
            {
                ledTimeOn = 50;
                ledTimeOff = 50;
                if(WiFi.status() == WL_CONNECTED){
                    _state = State::Connected;
                    //Set authentication for files
                    if(_iotPass.getValue().length()>0){
                        _fileHandler->setAuthentication("admin", _iotPass.getValue().c_str());
                    }else{
                        _fileHandler->setAuthentication("", "");
                    }
                    DebugPrint("\nConnected, IP is ");
                    DebugPrintln(WiFi.localIP());
                }else if((now-connectStart)>60000){
                    DebugPrintln();
                    DebugPrint("Connection timeout ");
                    switchToAP();
#ifdef ESPEasyCfg_SERIAL_DEBUG
                }else if((now-lastPrint)>1000){
                    lastPrint = now;
                    DebugPrint('.');
#endif
                }else{
                    if(_switchPin != UNUSED_PIN){
                        if(digitalRead(_switchPin) == LOW){
							//Switch pressed.
							DebugPrintln("Reseting password");
                            _iotPass.setValue("");
                            switchToAP();
                        }
                    }
                }
                break;
            }
            case State::WillConnect:
            {
                ledTimeOn = 500;
                ledTimeOff = 500;
                if(_wifiSSID.getValue().length()>0){
                    connectStart = now;
#ifdef ESP32					
                    //Wait to have time to send response
                    delay(100);
#endif					
                    switchToSTA();
                }else{
                    switchToAP();
                }
                break;
            }
            case State::AP:
            {
                ledTimeOn = 100;
                ledTimeOff = 100;
                runDNS();
                break;
            }
            case State::Connected:
                ledTimeOn = 50;
                ledTimeOff = 5000;
                break;
            default:
                break;
        }
        //Led blinker
        if(_ledPin != UNUSED_PIN){
            if(ledState){
                if((now-lastLedChange)>ledTimeOn){
                    digitalWrite(_ledPin, LOW);
                    ledState = false;
                    lastLedChange = now;
                }
            }else{
                if((now-lastLedChange)>ledTimeOff){
                    digitalWrite(_ledPin, HIGH);
                    ledState = true;
                    lastLedChange = now;
                }
            }
        }
#ifdef ESP32		
    }
#endif
}

void ESPEasyCfg::setRootHandler(ArRequestHandlerFunction func)
{
    _rootHandler = func;
}

void ESPEasyCfg::setNotFoundHandler(ArRequestHandlerFunction func)
{
    _notFoundHandler = func;
}

void ESPEasyCfg::setLedPin(int8_t pin)
{
    _ledPin = pin;
}

void ESPEasyCfg::setSwitchPin(int8_t pin)
{
    _switchPin = pin;
}