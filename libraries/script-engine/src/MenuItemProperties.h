//
//  MenuItemProperties.h
//  hifi
//
//  Created by Brad Hefta-Gaub on 1/28/14.
//  Copyright (c) 2014 HighFidelity, Inc. All rights reserved.
//

#ifndef __hifi_MenuItemProperties_h__
#define __hifi_MenuItemProperties_h__

#include <QtScript/QScriptEngine>

#include "EventTypes.h"

const int UNSPECIFIED_POSITION = -1;

class MenuItemProperties {
public:
    MenuItemProperties(); 
    MenuItemProperties(const QString& menuName, const QString& menuItemName, 
                        const QString& shortcutKey = QString(""), bool checkable = false, bool checked = false);
    MenuItemProperties(const QString& menuName, const QString& menuItemName, 
                        const KeyEvent& shortcutKeyEvent, bool checkable = false, bool checked = false);

    QString menuName;
    QString menuItemName;
    
    // Shortcut key items: in order of priority
    QString shortcutKey;
    KeyEvent shortcutKeyEvent;
    QKeySequence shortcutKeySequence; // this is what we actually use, it's set from one of the above

    // location related items: in order of priority
    int position;
    QString beforeItem;
    QString afterItem;

    // other properties
    bool isCheckable;
    bool isChecked;
    bool isSeparator;
};
Q_DECLARE_METATYPE(MenuItemProperties)
QScriptValue menuItemPropertiesToScriptValue(QScriptEngine* engine, const MenuItemProperties& props);
void menuItemPropertiesFromScriptValue(const QScriptValue& object, MenuItemProperties& props);
void registerMenuItemProperties(QScriptEngine* engine);



#endif // __hifi_MenuItemProperties_h__
