#include "ESPEasyCfgParameterManagerJSON.h"
#include <ArduinoJson.h>
#include "ESPEasyCfgConfiguration.h"

#ifdef ESP32
#include <SPIFFS.h>
#else
#include <FS.h>
#endif

ESPEasyCfgParameterManagerJSON::ESPEasyCfgParameterManagerJSON() : ESPEasyCfgParameterManager()
{    
}

ESPEasyCfgParameterManagerJSON::~ESPEasyCfgParameterManagerJSON()
{
}

void ESPEasyCfgParameterManagerJSON::init(ESPEasyCfgParameterGroup* firstGroup)
{
    //Initialise SPIFFS
    SPIFFS.begin();
}

bool ESPEasyCfgParameterManagerJSON::saveParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version)
{
    StaticJsonDocument<JSON_BUFFER_SIZE> root;
    root["version"] = version;
    JsonArray arr = root.createNestedArray("parameters");
    ESPEasyCfgParameterGroup* grp = firstGroup;
    while(grp){
        ESPEasyCfgAbstractParameter* param = grp->getFirst();
        while(param){
            JsonObject p = arr.createNestedObject();
            param->toJSON(p, true);
            param = param->getNextParameter();
        }
        grp = grp->getNext();
    }
    File paramFile = SPIFFS.open(PARAMETER_JSON_FILE, "w");
    serializeJson(root, paramFile);
    paramFile.close();
    return true;
}

bool ESPEasyCfgParameterManagerJSON::loadParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version)
{
    bool ret = false;
    File configFile = SPIFFS.open(PARAMETER_JSON_FILE, "r");
    if(configFile){
        DynamicJsonDocument json(JSON_BUFFER_SIZE);
        if(deserializeJson(json, configFile) == DeserializationError::Ok) {
                const char* fVersion = json["version"];
                if(strcmp(fVersion, version) == 0){
                    JsonArray arr = json["parameters"];
                    // All is fine
                    ESPEasyCfgParameterGroup* grp = firstGroup;
                    while(grp){
                        ESPEasyCfgAbstractParameter* param = grp->getFirst();
                        while(param){
                            JsonVariant ob = locateByID(arr, param->getIdentifier());
                            if(!ob.isNull()){
                                String s;
                                int8_t action;
                                param->setValue(ob["value"], s, action);
                            }
#ifdef ESPEasyCfg_SERIAL_DEBUG
                            Serial.print("Loading ");
                            Serial.print(param->getIdentifier());                            
                            Serial.print(" value ");
                            Serial.println(param->toString());
#endif                            
                            param = param->getNextParameter();
                        }
                        grp = grp->getNext();
                    }                
                    ret = true;
                }else{
#ifdef ESPEasyCfg_SERIAL_DEBUG                    
                    Serial.print("Bad config file version. Got ");
                    Serial.print(fVersion);
                    Serial.print(" expected ");
                    Serial.println(version);
#endif                    
                    ret = false; 
                }
        } else {
            ret = false;
        }
        if (configFile) {
            configFile.close();
        }
    }
    return ret;
}

JsonVariant ESPEasyCfgParameterManagerJSON::locateByID(JsonArray& arr, const char* id)
{

    for(JsonVariant elem: arr) {
        const char* vId = elem["id"];
        if(vId && (strcmp(vId, id) == 0)){
            return elem;
        }
    }
    return JsonVariant();
}