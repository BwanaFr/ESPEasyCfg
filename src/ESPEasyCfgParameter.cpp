#include "ESPEasyCfgParameter.h"
#include <FS.h>

template<>
ESPEasyCfgParameter<char*>::ESPEasyCfgParameter(const char* id, const char* name,
     char* defaultValue, const char* description, const char* extraAttributes) : 
    ESPEasyCfgAbstractParameter(id, name, description, extraAttributes)
{
    _value = new char[MAX_STRING_SIZE];
    strncpy(_value, defaultValue, MAX_STRING_SIZE-1);
    _value[MAX_STRING_SIZE-1] = '\0';
}

template<>
ESPEasyCfgParameter<char*>::ESPEasyCfgParameter(ESPEasyCfgParameterGroup& group, const char* id, const char* name,
     char* defaultValue, const char* description, const char* extraAttributes) : 
    ESPEasyCfgAbstractParameter(group, id, name, description, extraAttributes)
{
    _value = new char[MAX_STRING_SIZE];
    strncpy(_value, defaultValue, MAX_STRING_SIZE-1);
    _value[MAX_STRING_SIZE-1] = '\0';
}

template<>
ESPEasyCfgParameter<char*>::~ESPEasyCfgParameter()
{
    delete _value;
}

template<>
void ESPEasyCfgParameter<char*>::setValue(char* value)
{
    strncpy(_value, value, MAX_STRING_SIZE-1);
    _value[MAX_STRING_SIZE-1] = '\0';
}

/**
 * Specialization function for string
 */
template<>
size_t ESPEasyCfgParameter<char*>::getStorageSize()
{
    return MAX_STRING_SIZE;
}

/**
 * Specialization function for string
 */
template<>
size_t ESPEasyCfgParameter<String>::getStorageSize()
{
    return MAX_STRING_SIZE;
}

template<>
bool ESPEasyCfgParameter<char*>::storeTo(void* buffer, size_t bufferLen)
{
    if(bufferLen>=getStorageSize()){
        size_t strLen = strlen(_value);
        for(size_t i=0;i<strLen;++i)
        {
            ((char*)buffer)[i] = _value[i];
        }
        ((char*)buffer)[strLen] = '\0';
        return true;
    }
    return false;
}

template<>
bool ESPEasyCfgParameter<String>::storeTo(void* buffer, size_t bufferLen)
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

template<>
bool ESPEasyCfgParameter<char*>::loadFrom(const void* buffer, size_t bufferLen)
{
    if(bufferLen>=getStorageSize()){
        for(size_t i=0;i<MAX_STRING_SIZE;++i)
        {
            _value[i] = ((const char*)buffer)[i];
        }
        return true;
    }
    return false;
}

template<>
bool ESPEasyCfgParameter<String>::loadFrom(const void* buffer, size_t bufferLen)
{
    _value = ((const char*)buffer);
    return true;
}

/**
 * Get input type
*/
template<>
const char* ESPEasyCfgParameter<float>::getInputType()
{
    return _type ? _type : "number";
}

/**
 * Get input type
*/
template<>
const char* ESPEasyCfgParameter<double>::getInputType()
{
    return _type ? _type : "number";
}

/**
 * Get input type
*/
template<>
const char* ESPEasyCfgParameter<int32_t>::getInputType()
{
    return _type ? _type : "number";
}

/**
 * Get input type
*/
template<>
const char* ESPEasyCfgParameter<int16_t>::getInputType()
{
    return _type ? _type : "number";
}


/**
 * Get input type
*/
template<>
const char* ESPEasyCfgParameter<uint32_t>::getInputType()
{
    return _type ? _type : "number";
}

/**
 * Get input type
*/
template<>
const char* ESPEasyCfgParameter<uint16_t>::getInputType()
{
    return _type ? _type : "number";
}

template<> bool ESPEasyCfgParameter<char*>::setValue(const char* value, String& errMsg, int8_t& action, bool validate)
{
    char newValue[MAX_STRING_SIZE];
    size_t strLen = strlen(value);
    for(size_t i=0;i<strLen;++i)
    {
        ((char*)newValue)[i] = value[i];
    }
    ((char*)newValue)[strLen] = '\0';

    if(_validator && validate){
        if(_validator(this, newValue, errMsg, action)){
            return false;
        }
    }
    for(size_t i=0;i<MAX_STRING_SIZE;++i)
    {
        _value[i] = newValue[i];
    }
    return true;
}

template<> bool ESPEasyCfgParameter<String>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    String newValue(value);
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

template<> bool ESPEasyCfgParameter<float>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    float newValue = atof(value);
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

template<> bool ESPEasyCfgParameter<double>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    double newValue = atof(value);
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

template<> bool ESPEasyCfgParameter<int32_t>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    int32_t newValue = atoi(value);
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

template<> bool ESPEasyCfgParameter<int16_t>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    int16_t newValue = atoi(value);
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

template<> bool ESPEasyCfgParameter<uint32_t>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    uint32_t newValue = static_cast<uint32_t>(atoi(value));
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

template<> bool ESPEasyCfgParameter<uint16_t>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    uint16_t newValue = static_cast<uint16_t>(atoi(value));
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

ESPEasyCfgParameterGroup::ESPEasyCfgParameterGroup(const char* name) : 
    _name(name), _first(nullptr), _next(nullptr)
{

}

ESPEasyCfgParameterGroup::ESPEasyCfgParameterGroup(ESPEasyCfgParameterGroup* paramGrp, const char* name) :
_name(name), _first(nullptr), _next(nullptr)
{
    add(paramGrp);
}

ESPEasyCfgParameterGroup::~ESPEasyCfgParameterGroup()
{    
}

const char* ESPEasyCfgParameterGroup::getName() const
{
    return _name;
}

ESPEasyCfgAbstractParameter* ESPEasyCfgParameterGroup::getFirst()
{
    return _first;
}

void ESPEasyCfgParameterGroup::add(ESPEasyCfgAbstractParameter* param)
{
    ESPEasyCfgAbstractParameter** par = &_first;
    while(*par){
        par = &(*par)->_nextParam;
    }
    *par = param;
}

void ESPEasyCfgParameterGroup::add(ESPEasyCfgParameterGroup* paramGrp)
{
    ESPEasyCfgParameterGroup** grp = &_next;
    while(*grp){
        grp = &(*grp)->_next;
    }
    *grp = paramGrp;
}

ESPEasyCfgParameterGroup* ESPEasyCfgParameterGroup::getNext()
{
    return _next;
}
