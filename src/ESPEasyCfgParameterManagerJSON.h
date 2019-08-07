#ifndef _ESPEasyCfgParameterManagerJSON_H_
#define _ESPEasyCfgParameterManagerJSON_H_

#include <ESPEasyCfgParameter.h>
#include <ArduinoJson.h>

#define JSON_BUFFER_SIZE 1024
#define PARAMETER_JSON_FILE "/parameters.json"

class ESPEasyCfgParameterManagerJSON : public ESPEasyCfgParameterManager
{
public:    
    ESPEasyCfgParameterManagerJSON();
    virtual ~ESPEasyCfgParameterManagerJSON();
    void init(ESPEasyCfgParameterGroup* firstGroup);    
    bool saveParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version);
    bool loadParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version);
private:
    JsonVariant locateByID(JsonArray& arr, const char* id);
};

#endif
