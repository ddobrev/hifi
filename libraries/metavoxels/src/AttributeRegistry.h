//
//  AttributeRegistry.h
//  metavoxels
//
//  Created by Andrzej Kapolka on 12/6/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#ifndef __interface__AttributeRegistry__
#define __interface__AttributeRegistry__

#include <QHash>
#include <QSharedPointer>
#include <QString>

#include "Bitstream.h"

class Attribute;

typedef QSharedPointer<Attribute> AttributePointer;

/// Maintains information about metavoxel attribute types.
class AttributeRegistry {
public:
    
    /// Returns a pointer to the singleton registry instance.
    static AttributeRegistry* getInstance() { return &_instance; }
    
    AttributeRegistry();
    
    /// Registers an attribute with the system.
    /// \return either the pointer passed as an argument, if the attribute wasn't already registered, or the existing
    /// attribute
    AttributePointer registerAttribute(AttributePointer attribute);
    
    /// Retrieves an attribute by name.
    AttributePointer getAttribute(const QString& name) const { return _attributes.value(name); }
    
private:

    static AttributeRegistry _instance;

    QHash<QString, AttributePointer> _attributes;
};

/// Pairs an attribute value with its type.
class AttributeValue {
public:
    
    AttributeValue(const AttributePointer& attribute = AttributePointer(), void* const* value = NULL);
    AttributeValue(const AttributeValue& other);
    ~AttributeValue();
    
    AttributeValue& operator=(const AttributeValue& other);
    
    AttributePointer getAttribute() const { return _attribute; }
    void* getValue() const { return _value; }
    
    void* copy() const;

    bool isDefault() const;

    bool operator==(const AttributeValue& other) const;
    bool operator==(void* other) const;
    
private:
    
    AttributePointer _attribute;
    void* _value;
};

/// Represents a registered attribute.
class Attribute {
public:

    Attribute(const QString& name);
    virtual ~Attribute();

    const QString& getName() const { return _name; }

    virtual void* create(void* const* copy = NULL) const = 0;
    virtual void destroy(void* value) const = 0;

    virtual bool read(Bitstream& in, void*& value) const = 0;
    virtual bool write(Bitstream& out, void* value) const = 0;

    virtual bool equal(void* first, void* second) const = 0;

    virtual void* createAveraged(void* values[]) const = 0;

    virtual void* getDefaultValue() const = 0;

private:

    QString _name;
};

template<class T> inline void* encodeInline(const T& value) {
    return *(void**)const_cast<T*>(&value);
}

template<class T> inline T decodeInline(void* value) {
    return *(T*)&value;
}

/// A simple attribute class that stores its values inline.
template<class T, int bits> class InlineAttribute : public Attribute {
public:
    
    InlineAttribute(const QString& name, T defaultValue = T()) : Attribute(name), _defaultValue(encodeInline(defaultValue)) { }
    
    virtual void* create(void* const* copy = NULL) const { return (copy == NULL) ? _defaultValue : *copy; }
    virtual void destroy(void* value) const { /* no-op */ }
    
    virtual bool read(Bitstream& in, void*& value) const { value = getDefaultValue(); in.read(&value, bits); return false; }
    virtual bool write(Bitstream& out, void* value) const { out.write(&value, bits); return false; }

    virtual bool equal(void* first, void* second) const { return first == second; }

    virtual void* createAveraged(void* values[]) const;

    virtual void* getDefaultValue() const { return _defaultValue; }

private:
    
    void* _defaultValue;
};

template<class T, int bits> inline void* InlineAttribute<T, bits>::createAveraged(void* values[]) const {
    T total = T();
    for (int i = 0; i < 8; i++) {
        total += decodeInline<T>(values[i]);
    }
    total /= 8;
    return encodeInline(total);
}

#endif /* defined(__interface__AttributeRegistry__) */
