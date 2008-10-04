/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include "JSPlugin.h"

#include <wtf/GetPtr.h>

#include "AtomicString.h"
#include "PlatformString.h"
#include "Plugin.h"

using namespace KJS;

namespace WebCore {

/* Hash table */

static const HashEntry JSPluginTableEntries[] =
{
    { 0, 0, 0, 0, 0 },
    { "name", JSPlugin::NameAttrNum, DontDelete|ReadOnly, 0, &JSPluginTableEntries[4] },
    { "filename", JSPlugin::FilenameAttrNum, DontDelete|ReadOnly, 0, 0 },
    { "description", JSPlugin::DescriptionAttrNum, DontDelete|ReadOnly, 0, 0 },
    { "length", JSPlugin::LengthAttrNum, DontDelete|ReadOnly, 0, 0 }
};

static const HashTable JSPluginTable = 
{
    2, 5, JSPluginTableEntries, 4
};

/* Hash table for prototype */

static const HashEntry JSPluginPrototypeTableEntries[] =
{
    { 0, 0, 0, 0, 0 }
};

static const HashTable JSPluginPrototypeTable = 
{
    2, 1, JSPluginPrototypeTableEntries, 1
};

const ClassInfo JSPluginPrototype::info = { "PluginPrototype", 0, &JSPluginPrototypeTable, 0 };

JSObject* JSPluginPrototype::self(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSPluginPrototype>(exec, "[[JSPlugin.prototype]]");
}

const ClassInfo JSPlugin::info = { "Plugin", 0, &JSPluginTable, 0 };

JSPlugin::JSPlugin(ExecState* exec, Plugin* impl)
    : m_impl(impl)
{
    setPrototype(JSPluginPrototype::self(exec));
}

JSPlugin::~JSPlugin()
{
    ScriptInterpreter::forgetDOMObject(m_impl.get());
}

bool JSPlugin::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    const HashEntry* entry = Lookup::findEntry(&JSPluginTable, propertyName);
    if (entry) {
        slot.setStaticEntry(this, entry, staticValueGetter<JSPlugin>);
        return true;
    }
    bool ok;
    unsigned index = propertyName.toUInt32(&ok, false);
    if (ok && index < static_cast<Plugin*>(impl())->length()) {
        slot.setCustomIndex(this, index, indexGetter);
        return true;
    }
    if (canGetItemsForName(exec, static_cast<Plugin*>(impl()), propertyName)) {
        slot.setCustom(this, nameGetter);
        return true;
    }
    return KJS::DOMObject::getOwnPropertySlot(exec, propertyName, slot);
}

JSValue* JSPlugin::getValueProperty(ExecState* exec, int token) const
{
    switch (token) {
    case NameAttrNum: {
        Plugin* imp = static_cast<Plugin*>(impl());

        return jsString(imp->name());
    }
    case FilenameAttrNum: {
        Plugin* imp = static_cast<Plugin*>(impl());

        return jsString(imp->filename());
    }
    case DescriptionAttrNum: {
        Plugin* imp = static_cast<Plugin*>(impl());

        return jsString(imp->description());
    }
    case LengthAttrNum: {
        Plugin* imp = static_cast<Plugin*>(impl());

        return jsNumber(imp->length());
    }
    }
    return 0;
}


JSValue* JSPlugin::indexGetter(ExecState* exec, JSObject* originalObject, const Identifier& propertyName, const PropertySlot& slot)
{
    JSPlugin* thisObj = static_cast<JSPlugin*>(slot.slotBase());
    return toJS(exec, static_cast<Plugin*>(thisObj->impl())->item(slot.index()));
}
KJS::JSValue* toJS(KJS::ExecState* exec, Plugin* obj)
{
    return KJS::cacheDOMObject<Plugin, JSPlugin>(exec, obj);
}
Plugin* toPlugin(KJS::JSValue* val)
{
    return val->isObject(&JSPlugin::info) ? static_cast<JSPlugin*>(val)->impl() : 0;
}

}
