#include "ESPEasyCfgParameterManagerJSON.h"
#include <FS.h>
#include <ArduinoJson.h>
#include "ESPEasyCfgConfiguration.h"

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
    StaticJsonBuffer<JSON_BUFFER_SIZE> jb;
    JsonObject& root = jb.createObject();
    root["version"] = version;
    JsonArray& arr = root.createNestedArray("parameters");
    ESPEasyCfgParameterGroup* grp = firstGroup;
    while(grp){
        ESPEasyCfgAbstractParameter* param = grp->getFirst();
        while(param){
            JsonObject& p = arr.createNestedObject();
            param->toJSON(p, true);
            param = param->getNextParameter();
        }
        grp = grp->getNext();
    }
    File paramFile = SPIFFS.open(PARAMETER_JSON_FILE, "w");
    root.printTo(paramFile);
    paramFile.close();
    return true;
}

bool ESPEasyCfgParameterManagerJSON::loadParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version)
{
    bool ret = false;
    File configFile = SPIFFS.open(PARAMETER_JSON_FILE, "r");
    if(configFile){
        std::unique_ptr<char[]> buf(new char[configFile.size()]);
        if ( configFile.readBytes(buf.get(), configFile.size()) == configFile.size() ) {
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
            if (json.success()) {
                const char* fVersion = json["version"];
                if(strcmp(fVersion, version) == 0){
                    JsonArray& arr = json["parameters"];
                    // All is fine
                    ESPEasyCfgParameterGroup* grp = firstGroup;
                    while(grp){
                        ESPEasyCfgAbstractParameter* param = grp->getFirst();
                        while(param){
                            JsonObject* ob = locateByID(arr, param->getIdentifier());
                            if(ob){
                                String s;
                                int8_t action;
                                param->setValue((*ob)["value"], s, action);
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
        } else {
            ret = false;
        }

        if (configFile) {
        configFile.close();
        }
    }
    return ret;
}

JsonObject* ESPEasyCfgParameterManagerJSON::locateByID(JsonArray& arr, const char* id)
{

    for(JsonObject& elem: arr) {
        const char* vId = elem["id"];
        if(vId && (strcmp(vId, id) == 0)){
            return &elem;
        }
    }
    return nullptr;
}