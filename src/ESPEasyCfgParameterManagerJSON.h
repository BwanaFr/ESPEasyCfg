#ifndef _ESPEasyCfgParameterManagerJSON_H_
#define _ESPEasyCfgParameterManagerJSON_H_

#include <ESPEasyCfgParameter.h>
#include <ArduinoJson.h>

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
