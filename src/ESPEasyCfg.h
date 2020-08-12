#ifndef _ESPEASYCFG_H_
#define _ESPEASYCFG_H_

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include "ESPEasyCfgParameter.h"
#include "ESPEasyCfgEnumParameter.h"
#include <DNSServer.h>


void ESPEasyCfgMonitorTask(void* instance);

/**
 * Application state
 * @Connecting Trying to connect to WiFi
 * @AP Access point mode (captive portal)
 * @Connected WiFi is connected (normal mode)
 * @WillConnect Connection will be established
 * @Reconfigured Application is reconfigured via web interface
 */
enum class ESPEasyCfgState {Connecting, AP, Connected, WillConnect, Reconfigured};

typedef std::function<void(ESPEasyCfgState)> StateHandlerFunction;

class ESPEasyCfg
{
    private:    
        AsyncWebServer *_webServer;                 //!< Reference to the webserver
        ESPEasyCfgParameter<String> _iotName;       //!< Name of this thing (parameter)
        ESPEasyCfgParameter<String> _iotPass;       //!< Password of this thing
        ESPEasyCfgParameter<String> _wifiSSID;      //!< SSID of the WiFi network to connect to
        ESPEasyCfgParameter<String> _wifiPass;      //!< Password of WiFi to connect to (blank : open)
        ESPEasyCfgParameterGroup _paramGrp;         //!< Group for holding build-in parameters
        ESPEasyCfgState _state;                     //!< State of this application
        AsyncCallbackJsonWebHandler* _cfgHandler;   //!< Web handler to handle set of parameter
        AsyncStaticWebHandler* _fileHandler;        //!< Web handler for static files stored in SPIFFS on /wwww/
        DNSServer* _dnsServer;                      //!< DNS server to handle captive portal redirections
        ESPEasyCfgParameterManager* _paramManager;  //!< Manager to read/write application parameters        
        long long _lastCon;                         //!< Last millis() of WiFi connection
        uint8_t _ledPin;                            //!< LED pin to signal activity
        bool _ledActiveLow;                         //!< Led active low cabling
        uint8_t _switchPin;                         //!< Switch pin to reset password
        int8_t _scanCount;                          //!< WiFi scan
        ArRequestHandlerFunction _rootHandler;      //!< Root handler (if installed)
        ArRequestHandlerFunction _notFoundHandler;  //!< 404 error handler
        StateHandlerFunction _stateHandler;         //!< Custom handler for monitoring state
        /**
         * Serialize parameters to JSON
         * @param arr JSON array to put parameters to
         * @param first First parameter group to get parameters from
         */
        void toJSON(ArduinoJson::JsonArray& arr, ESPEasyCfgParameterGroup* first);
        /**
         * Parse parameters from JSON and store it into parameters
         * @param json JSON object to be parsed
         * @param first First parameter group
         * @param msg Message to be displayed to user
         * @param action Action to be performed
         */
        void fromJSON(ArduinoJson::JsonObject& json, ESPEasyCfgParameterGroup* first, String& msg, int8_t& action);

        /**
         * Start and run DNS server
         */
        void runDNS();
        /**
         * Stops and destroy the DNS server
         */
        void stopDNS();

        /**
         * Switch to AP mode
         */
        void switchToAP();

        /**
         * Switch to station
         */
        void switchToSTA();
        
        /**
         * Change the state
         */
        void setState(ESPEasyCfgState newState);
        
        /**
         * Sets LED state
         */
        void setLed(bool state);

    public:
#ifdef ESP32
        /**
         * Monitor state
         */
        void monitorState();
#elif defined(ESP8266)
		/**
		 * Performs background tasks
		 */
		void loop();
#endif
        /**
         * Constructor
         * @param webServer Webserver instance
         */
        ESPEasyCfg(AsyncWebServer *webServer);
        /**
         * Constructor
         * @param webServer Webserver instance
         * @param thingName Name of the thing (AP name)
         */
        ESPEasyCfg(AsyncWebServer *webServer, const char* thingName);
        /**
         * Destructor
         */
        virtual ~ESPEasyCfg();
        /**
         * Initialize this
         */
        void begin();
        /**
         * Get application state
         */
        ESPEasyCfgState getState();
        /**
         * Associate a parameter manager to this
         * This method must be called before begin()!
         * @param manager Manager to be associated
         */
        void setParameterManager(ESPEasyCfgParameterManager* manager);

        /**
         * Set the root handler function for a normal usage
         * @param func Handler to install to handle root 
         */
        void setRootHandler(ArRequestHandlerFunction func);

        /**
         * Set the not foung handler function for a normal usage
         * @param func Handler to install to handle 404 errors 
         */
        void setNotFoundHandler(ArRequestHandlerFunction func);

        /**
         * Sets the LED pin
         * @param pin Pin number (active high)
         */
        void setLedPin(int8_t pin);

        /**
         * Sets if the LED is active low or high
         * @param activeLow True if the LED is cabled as active low
         */
        inline void setLedActiveLow(bool activeLow) {
            _ledActiveLow = activeLow;
        }

        /**
         * Sets the switch pin to reset configuration
         * @param pin Pin number (active low)
         */
        void setSwitchPin(int8_t pin);

        /**
         * Adds a parameter group to be managed by the captive portal
         * This method must be called before begin!
         * @param grp Parameter group to be added on configuration page
         */
        void addParameterGroup(ESPEasyCfgParameterGroup* grp);

        /**
         * Sets a state handler callback to be called when portal state
         * changes
         * @handler Handler function to be called
         */
        void setStateHandler(StateHandlerFunction handler);

        /**
         * Save actual parameters values to flash
         */
        void saveParameters();
};


#endif