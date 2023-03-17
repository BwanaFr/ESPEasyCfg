#include "ESPEasyCfgEnumParameter.h"

#define ITEM_SEPARATOR ';'

ESPEasyCfgEnumParameter::ESPEasyCfgEnumParameter(const char* id, const char* name, const char* items,
                        const char* defaultValue, const char* description,
                        const char* extraAttributes):
                        ESPEasyCfgAbstractParameter(id, name, description, extraAttributes),
                        _items(items), _itemSearchOffset(0)
{
    if(defaultValue){
        _value = defaultValue;
    }else{
        char val[MAX_STRING_SIZE];
        getNextItem(val);
        _value = val;
        _itemSearchOffset = 0;
    }
}

ESPEasyCfgEnumParameter::ESPEasyCfgEnumParameter(ESPEasyCfgParameterGroup& group, const char* id, const char* name, const char* items,
                        const char* defaultValue, const char* description,
                        const char* extraAttributes):
                        ESPEasyCfgAbstractParameter(group, id, name, description, extraAttributes),
                        _items(items), _itemSearchOffset(0)
{
    if(defaultValue){
        _value = defaultValue;
    }else{
        char val[MAX_STRING_SIZE];
        getNextItem(val);
        _value = val;
        _itemSearchOffset = 0;
    }
}

ESPEasyCfgEnumParameter::~ESPEasyCfgEnumParameter()
{

}

String ESPEasyCfgEnumParameter::toString()
{
    return _value;
}

size_t ESPEasyCfgEnumParameter::getStorageSize()
{
    return MAX_STRING_SIZE;
}

bool ESPEasyCfgEnumParameter::storeTo(void* buffer, size_t bufferLen)
{
    if(bufferLen>=getStorageSize()){
        size_t strLen = _value.length();
        for(size_t i=0;i<strLen;++i)
        {
            ((char*)buffer)[i] = _value[i];
        }
        ((char*)buffer)[strLen] = '\0';
        return true;
    }
    return false;
}

bool ESPEasyCfgEnumParameter::loadFrom(const void* buffer, size_t bufferLen)
{
    _value = ((const char*)buffer);
    return true;
}

const char* ESPEasyCfgEnumParameter::getInputType()
{
    return "enum";
}

void ESPEasyCfgEnumParameter::toJSON(ArduinoJson::JsonObject& dest, bool lightOutput)
{
    dest["id"] = getIdentifier();
    if(!lightOutput){        
        const char* type  = getInputType();
        dest["type"] = type;                    
        dest["value"] = _value;
        dest["name"] = getName();
        const char* desc = getDescription();
        if(desc){
            dest["desc"] = desc;
        }

        const char* extraAttributes = getExtraAttributes();
        if(extraAttributes){
            dest["attributes"] = extraAttributes;
        }
        ArduinoJson::JsonArray values = dest.createNestedArray("values");
        char val[MAX_STRING_SIZE];
        _itemSearchOffset = 0;
        while(getNextItem(&val[0])){
            values.add(val);
        }
    }else{
        dest["value"] = _value;
    }
}
bool ESPEasyCfgEnumParameter::setValue(const char* value, String& errMsg, int8_t& action, bool validate)
{
    _value = value;
    return true;
}

bool ESPEasyCfgEnumParameter::getNextItem(char* value)
{
    size_t len = strlen(_items);
    if(_itemSearchOffset>=len){
            *value = '\0';
            return false;
    }
    while(_itemSearchOffset<len){
            if(_items[_itemSearchOffset] == ITEM_SEPARATOR){
                    ++_itemSearchOffset;
                    break;
            }
            *value = _items[_itemSearchOffset];
            ++value;
            ++_itemSearchOffset;
    }
    *value = '\0';
    return true;
}