#ifndef ESPEASYCFG_PARAMETER_
#define ESPEASYCFG_PARAMETER_

#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <functional>

#define MAX_STRING_SIZE 64


class ESPEasyCfgParameterGroup;

/**
 * A generic parameter interface
 */ 
class ESPEasyCfgAbstractParameter
{
private:
    const char* _id;
    const char* _name;
    const char* _description;
    const char* _extraAttributes;
    ESPEasyCfgAbstractParameter* _nextParam;
    friend class ESPEasyCfgParameterGroup;

public:
    /**
     * Action to be performed on parameter valid
     */
    enum Action {NONE=0, RELOAD=1, CLOSE=2};
    ESPEasyCfgAbstractParameter(const char* id, const char* name, 
            const char* description = nullptr,
            const char* extraAttributes = nullptr) : 
            _id(id), _name(name), _description(description), _extraAttributes(extraAttributes), _nextParam(nullptr){}

    /**
     * Return a string representation of the parameter
     */
    virtual String toString() = 0;

    /**
     * Get the number of bytes occupied in storage
     */
    virtual size_t getStorageSize() = 0;

    /**
     * Store the parameter into the specified buffer
     * @buffer pointer to buffer 
     * @bufferLen Buffer size
     * @return True if success (buffer size ok)
     */
    virtual bool storeTo(void* buffer, size_t bufferLen) = 0;

    /**
     * Load the parameter from the specified buffer
     * @buffer Buffer to load data to
     * @bufferLen Buffer size
     * @return True if success (buffer size ok)
     */
    virtual bool loadFrom(const void* buffer, size_t bufferLen) = 0;

    /**
     * Gets the next parameter, used for chained list
     * @return Pointer to next parameter of nullptr if this is the last
     */
    inline ESPEasyCfgAbstractParameter* getNextParameter() { return _nextParam; }

    /**
     * Get parameter identifier
     */
    inline const char* getIdentifier() const { return _id; }

    /**
     * Get parameter name
     */
    inline const char* getName() const { return _name; }

    /**
     * Get description 
     */
    inline const char* getDescription() const { return _description; }

    /**
     * Gets extra attributes
     */
    inline const char* getExtraAttributes() const { return _extraAttributes; }

    /**
     * Sets extra attributes
     */
    inline void setExtraAttributes(const char* extraAttributes){ _extraAttributes = extraAttributes; }

    /**
     * Get input type
    */
    virtual const char* getInputType() = 0;

    /**
     * Serialize to JSON
     */
    virtual void toJSON(ArduinoJson::JsonObject& dest, bool lightOutput=false) = 0;

    /**
     * Sets a value from a string
     * @value Value to set
     * @msg Message to be displayed to user
     * @validate True if validator must be called
     * @action Action to be performed on target page 
     * @return true if ok
     */
    virtual bool setValue(const char* value, String& msg, int8_t& action, bool validate=false) = 0;
};

/**
 * Generic parameter implementation
 */
template<typename T>
class ESPEasyCfgParameter : public ESPEasyCfgAbstractParameter
{
public:
    typedef std::function<bool(ESPEasyCfgParameter *param, T newValue, String& msg, int8_t& action)> ValidatorFunction;
private:
    T _value;
    const char* _type;
    ValidatorFunction _validator;
public:
    ESPEasyCfgParameter(const char* id, const char* name, T defaultValue, const char* description = nullptr,
                        const char* extraAttributes = nullptr);
    virtual ~ESPEasyCfgParameter();
    String toString();
    size_t getStorageSize();
    bool storeTo(void* buffer, size_t bufferLen);
    bool loadFrom(const void* buffer, size_t bufferLen);
    T getValue();
    void setValue(T value);
    void setInputType(const char* type);
    const char* getInputType();
    void toJSON(ArduinoJson::JsonObject& dest, bool lightOutput);
    bool setValue(const char* value, String& errMsg, int8_t& action, bool validate);
    inline void setValidator(ValidatorFunction validator) { _validator = validator; }
};

template<typename T>
ESPEasyCfgParameter<T>::ESPEasyCfgParameter(const char* id, const char* name, T defaultValue,
    const char* description, const char* extraAttributes) : 
    ESPEasyCfgAbstractParameter(id, name, description, extraAttributes), _value(defaultValue), _type(nullptr)
{}

template<typename T>
ESPEasyCfgParameter<T>::~ESPEasyCfgParameter()
{}

template<typename T>
String ESPEasyCfgParameter<T>::toString()
{
    return String(_value);
}

template<typename T>
T ESPEasyCfgParameter<T>::getValue()
{
    return _value;
}

template<typename T>
void ESPEasyCfgParameter<T>::setValue(T value)
{
    _value = value;
}

template<typename T>
size_t ESPEasyCfgParameter<T>::getStorageSize()
{
    return sizeof(T);
}

template<typename T>
bool ESPEasyCfgParameter<T>::storeTo(void* buffer, size_t bufferLen)
{
    if(bufferLen>=getStorageSize()){
        *((T*)buffer) = getValue();
        return true;
    }
    return false;
}

template<typename T>
bool ESPEasyCfgParameter<T>::loadFrom(const void* buffer, size_t bufferLen)
{
    if(bufferLen>=getStorageSize()){
        _value = *((T*)buffer);
        return true;
    }
    return false;
}

/**
 * Get input type
*/
template<typename T>
void ESPEasyCfgParameter<T>::setInputType(const char* type)
{
    _type = type;
}

/**
 * Get input type
*/
template<typename T>
const char* ESPEasyCfgParameter<T>::getInputType()
{
    return _type;
}

template<typename T>
void ESPEasyCfgParameter<T>::toJSON(ArduinoJson::JsonObject& dest, bool lightOutput)
{
    dest["id"] = getIdentifier();
    if(!lightOutput){        
        const char* type  = getInputType();
        if(type != nullptr){
            dest["type"] = type;            
            if(strcmp(type, "password") == 0){
                if(toString().length()>0){
                    dest["value"] = "----------";
                }
            }else{
                dest["value"] = getValue();
            }
        }else{
            dest["value"] = getValue();
        }
        dest["name"] = getName();
        const char* desc = getDescription();
        if(desc){
            dest["desc"] = desc;
        }

        const char* extraAttributes = getExtraAttributes();
        if(extraAttributes){
            dest["attributes"] = extraAttributes;
        }
    }else{
        //Lightoutput, used to save parameter, don't hide password
        dest["value"] = getValue();
    }
}

template<typename T>
bool ESPEasyCfgParameter<T>::setValue(const char* value, String& msg, int8_t& action, bool validate)
{
    T newValue = dynamic_cast<T>(value);
    if(_validator && validate){
        if(_validator(this, newValue, msg, action)){
            return false;
        }
    }
    _value = newValue;
    return true;
}

/*************************
 * 
 * Specialized functions
 */

template<> ESPEasyCfgParameter<char*>::ESPEasyCfgParameter(const char* id, const char* name, 
                                                    char* defaultValue, const char* description, const char* extraAttributes); 

template<> ESPEasyCfgParameter<char*>::~ESPEasyCfgParameter();

template<> void ESPEasyCfgParameter<char*>::setValue(char* value);

template<> size_t ESPEasyCfgParameter<char*>::getStorageSize();
template<> size_t ESPEasyCfgParameter<String>::getStorageSize();

template<> bool ESPEasyCfgParameter<char*>::storeTo(void* buffer, size_t bufferLen);
template<> bool ESPEasyCfgParameter<String>::storeTo(void* buffer, size_t bufferLen);

template<> bool ESPEasyCfgParameter<char*>::loadFrom(const void* buffer, size_t bufferLen);
template<> bool ESPEasyCfgParameter<String>::loadFrom(const void* buffer, size_t bufferLen);

/**
 * Get input type
*/
template<> const char* ESPEasyCfgParameter<float>::getInputType();
template<> const char* ESPEasyCfgParameter<double>::getInputType();
template<> const char* ESPEasyCfgParameter<int32_t>::getInputType();
template<> const char* ESPEasyCfgParameter<int16_t>::getInputType();
template<> const char* ESPEasyCfgParameter<uint32_t>::getInputType();
template<> const char* ESPEasyCfgParameter<uint16_t>::getInputType();

/**
 * Value validation/set
 */
template<> bool ESPEasyCfgParameter<char*>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<String>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<float>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<double>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<int32_t>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<int16_t>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<uint32_t>::setValue(const char* value, String& msg, int8_t& action, bool validate);
template<> bool ESPEasyCfgParameter<uint16_t>::setValue(const char* value, String& msg, int8_t& action, bool validate);

/**
 * Group of parameters
 */
class ESPEasyCfgParameterGroup
{
private:
    const char* _name;
    ESPEasyCfgAbstractParameter *_first;
    ESPEasyCfgParameterGroup *_next;
public:
    ESPEasyCfgParameterGroup(const char* name);
    virtual ~ESPEasyCfgParameterGroup();
    const char* getName() const;
    ESPEasyCfgAbstractParameter* getFirst();
    void add(ESPEasyCfgAbstractParameter* param);
    void add(ESPEasyCfgParameterGroup* paramGrp);
    ESPEasyCfgParameterGroup* getNext();
};

/**
 * Abstract class to load/save parameters
 */
class ESPEasyCfgParameterManager
{
public:
    virtual inline ~ESPEasyCfgParameterManager() {};
    /**
     * Initialize the manager
     * @firstGroup First parameter group
     */
    virtual void init(ESPEasyCfgParameterGroup* firstGroup) = 0;
    /**
     * Save parameters into file/EEPROM or whatever
     * @firstGroup First parameter group
     * @version Version string of parameters
     * @return true on success
     */
    virtual bool saveParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version) = 0;

    /**
     * Load parameters from file/EEPROM or whatever
     * @firstGroup First parameter group
     * @version Version string of parameters
     * @return true on success
     */
    virtual bool loadParameters(ESPEasyCfgParameterGroup* firstGroup, const char* version) = 0;
};


#endif