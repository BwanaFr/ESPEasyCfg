#ifndef ESPEASYCFG_ENUMPARAMETER_
#define ESPEASYCFG_ENUMPARAMETER_
#include "ESPEasyCfgParameter.h"

class ESPEasyCfgEnumParameter : public ESPEasyCfgAbstractParameter
{
public:
    ESPEasyCfgEnumParameter(const char* id, const char* name, const char* items,
                        const char* defaultValue = nullptr, const char* description = nullptr,
                        const char* extraAttributes = nullptr);
    virtual ~ESPEasyCfgEnumParameter();
    String toString() override;
    size_t getStorageSize() override;
    bool storeTo(void* buffer, size_t bufferLen) override;
    bool loadFrom(const void* buffer, size_t bufferLen) override;
    const char* getInputType() override;
    void toJSON(ArduinoJson::JsonObject& dest, bool lightOutput) override;
    bool setValue(const char* value, String& errMsg, int8_t& action, bool validate) override;
    inline void setValue(const char* value) { _value = value; };
private:
    String _value;
    const char* _items;
    size_t _itemSearchOffset;
    bool getNextItem(char* value);

};



#endif //ESPEASYCFG_ENUMPARAMETER_