#include "ESPEasyCfgParameterManagerJSON.h"
#include <ArduinoJson.h>
#include "ESPEasyCfgConfiguration.h"

#ifdef ESP32
#ifdef USE_LITTLE_FS
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#endif
#else
#include <FS.h>
#endif

#define PARAMETER_JSON_FILE "/parameters.json"

ESPEasyCfgParameterManagerJSON::ESPEasyCfgParameterManagerJSON() : ESPEasyCfgParameterManager()
{
}

ESPEasyCfgParameterManagerJSON::~ESPEasyCfgParameterManagerJSON()
{
}

void ESPEasyCfgParameterManagerJSON::init(ESPEasyCfgParameterGroup* firstGroup)
{
#ifdef USE_LITTLE_FS
    LittleFS.begin(true);
#else
    //Initialise SPIFFS
    SPIFFS.begin();
#endif
}

bool ESPEasyCfgParameterManagerJSON::saveParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version)
{
    JsonDocument  root;
    root["version"] = version;
    JsonArray arr = root["parameters"].to<JsonArray>();
    ESPEasyCfgParameterGroup* grp = firstGroup;
    while(grp){
        ESPEasyCfgAbstractParameter* param = grp->getFirst();
        while(param){
            JsonObject p = arr.add<JsonObject>();
            param->toJSON(p, true);
            param = param->getNextParameter();
        }
        grp = grp->getNext();
    }
#ifdef USE_LITTLE_FS
    File paramFile = LittleFS.open(PARAMETER_JSON_FILE, "w");
#else
    File paramFile = SPIFFS.open(PARAMETER_JSON_FILE, "w");
#endif
    serializeJson(root, paramFile);
    paramFile.close();
    return true;
}

bool ESPEasyCfgParameterManagerJSON::loadParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version)
{
    bool ret = false;
#ifdef USE_LITTLE_FS
    File configFile = LittleFS.open(PARAMETER_JSON_FILE, "r");
#else
    File configFile = SPIFFS.open(PARAMETER_JSON_FILE, "r");
#endif
    if(configFile){
        JsonDocument json;
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
                                DebugPrint("Loading ");
                                DebugPrint(param->getIdentifier());
                                String s;
                                int8_t action;
                                JsonVariant val = ob["value"];
                                if(!val.isNull()){
                                    String strVal = val.as<String>();
                                    param->setValue(strVal.c_str(), s, action);
                                }
                                DebugPrint(" value ");
                                if(param->getInputType() &&
                                    (strcmp(param->getInputType(), "password") == 0)){
                                    String paramValue = param->toString();
                                    if(paramValue.length()==0){
                                        DebugPrintln("-Not set-");
                                    }else{
                                        DebugPrintln("-Secret-");
                                    }
                                }else{
                                    DebugPrintln(param->toString());
                                }
                            }
                            param = param->getNextParameter();
                        }
                        grp = grp->getNext();
                    }
                    ret = true;
                }else{
                    DebugPrint("Bad config file version. Got ");
                    DebugPrint(fVersion);
                    DebugPrint(" expected ");
                    DebugPrintln(version);
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

void ESPEasyCfgParameterManagerJSON::resetToFactory()
{
#ifdef USE_LITTLE_FS
    LittleFS.remove(PARAMETER_JSON_FILE);
#else
    SPIFFS.remove(PARAMETER_JSON_FILE);
#endif
}
