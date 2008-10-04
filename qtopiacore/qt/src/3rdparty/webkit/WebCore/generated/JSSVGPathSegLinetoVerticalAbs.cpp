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


#if ENABLE(SVG)

#include "Document.h"
#include "Frame.h"
#include "SVGDocumentExtensions.h"
#include "SVGElement.h"
#include "SVGAnimatedTemplate.h"
#include "JSSVGPathSegLinetoVerticalAbs.h"

#include <wtf/GetPtr.h>

#include "SVGPathSegLinetoVertical.h"

using namespace KJS;

namespace WebCore {

/* Hash table */

static const HashEntry JSSVGPathSegLinetoVerticalAbsTableEntries[] =
{
    { "y", JSSVGPathSegLinetoVerticalAbs::YAttrNum, DontDelete, 0, 0 }
};

static const HashTable JSSVGPathSegLinetoVerticalAbsTable = 
{
    2, 1, JSSVGPathSegLinetoVerticalAbsTableEntries, 1
};

/* Hash table for prototype */

static const HashEntry JSSVGPathSegLinetoVerticalAbsPrototypeTableEntries[] =
{
    { 0, 0, 0, 0, 0 }
};

static const HashTable JSSVGPathSegLinetoVerticalAbsPrototypeTable = 
{
    2, 1, JSSVGPathSegLinetoVerticalAbsPrototypeTableEntries, 1
};

const ClassInfo JSSVGPathSegLinetoVerticalAbsPrototype::info = { "SVGPathSegLinetoVerticalAbsPrototype", 0, &JSSVGPathSegLinetoVerticalAbsPrototypeTable, 0 };

JSObject* JSSVGPathSegLinetoVerticalAbsPrototype::self(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSSVGPathSegLinetoVerticalAbsPrototype>(exec, "[[JSSVGPathSegLinetoVerticalAbs.prototype]]");
}

const ClassInfo JSSVGPathSegLinetoVerticalAbs::info = { "SVGPathSegLinetoVerticalAbs", &JSSVGPathSeg::info, &JSSVGPathSegLinetoVerticalAbsTable, 0 };

JSSVGPathSegLinetoVerticalAbs::JSSVGPathSegLinetoVerticalAbs(ExecState* exec, SVGPathSegLinetoVerticalAbs* impl)
    : JSSVGPathSeg(exec, impl)
{
    setPrototype(JSSVGPathSegLinetoVerticalAbsPrototype::self(exec));
}

bool JSSVGPathSegLinetoVerticalAbs::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGPathSegLinetoVerticalAbs, JSSVGPathSeg>(exec, &JSSVGPathSegLinetoVerticalAbsTable, this, propertyName, slot);
}

JSValue* JSSVGPathSegLinetoVerticalAbs::getValueProperty(ExecState* exec, int token) const
{
    switch (token) {
    case YAttrNum: {
        SVGPathSegLinetoVerticalAbs* imp = static_cast<SVGPathSegLinetoVerticalAbs*>(impl());

        return jsNumber(imp->y());
    }
    }
    return 0;
}

void JSSVGPathSegLinetoVerticalAbs::put(ExecState* exec, const Identifier& propertyName, JSValue* value, int attr)
{
    lookupPut<JSSVGPathSegLinetoVerticalAbs, JSSVGPathSeg>(exec, propertyName, value, attr, &JSSVGPathSegLinetoVerticalAbsTable, this);
}

void JSSVGPathSegLinetoVerticalAbs::putValueProperty(ExecState* exec, int token, JSValue* value, int /*attr*/)
{
    switch (token) {
    case YAttrNum: {
        SVGPathSegLinetoVerticalAbs* imp = static_cast<SVGPathSegLinetoVerticalAbs*>(impl());

        imp->setY(value->toFloat(exec));
        break;
    }
    }
    SVGPathSegLinetoVerticalAbs* imp = static_cast<SVGPathSegLinetoVerticalAbs*>(impl());

    ASSERT(exec && exec->dynamicInterpreter());
    Frame* activeFrame = static_cast<ScriptInterpreter*>(exec->dynamicInterpreter())->frame();
    if (!activeFrame)
        return;

    SVGDocumentExtensions* extensions = (activeFrame->document() ? activeFrame->document()->accessSVGExtensions() : 0);
    if (extensions && extensions->hasGenericContext<SVGPathSeg>(imp)) {
        const SVGElement* context = extensions->genericContext<SVGPathSeg>(imp);
        ASSERT(context);

        context->notifyAttributeChange();
    }

}


}

#endif // ENABLE(SVG)
