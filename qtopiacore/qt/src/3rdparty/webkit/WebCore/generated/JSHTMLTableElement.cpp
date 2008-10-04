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

#include "JSHTMLTableElement.h"

#include <wtf/GetPtr.h>

#include "ExceptionCode.h"
#include "HTMLCollection.h"
#include "HTMLElement.h"
#include "HTMLTableCaptionElement.h"
#include "HTMLTableElement.h"
#include "HTMLTableSectionElement.h"
#include "JSHTMLCollection.h"
#include "JSHTMLElement.h"
#include "JSHTMLTableCaptionElement.h"
#include "JSHTMLTableSectionElement.h"
#include "PlatformString.h"

using namespace KJS;

namespace WebCore {

/* Hash table */

static const HashEntry JSHTMLTableElementTableEntries[] =
{
    { "bgColor", JSHTMLTableElement::BgColorAttrNum, DontDelete, 0, 0 },
    { "tFoot", JSHTMLTableElement::TFootAttrNum, DontDelete, 0, &JSHTMLTableElementTableEntries[15] },
    { "summary", JSHTMLTableElement::SummaryAttrNum, DontDelete, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "cellPadding", JSHTMLTableElement::CellPaddingAttrNum, DontDelete, 0, &JSHTMLTableElementTableEntries[17] },
    { "align", JSHTMLTableElement::AlignAttrNum, DontDelete, 0, 0 },
    { "caption", JSHTMLTableElement::CaptionAttrNum, DontDelete, 0, 0 },
    { "tBodies", JSHTMLTableElement::TBodiesAttrNum, DontDelete|ReadOnly, 0, &JSHTMLTableElementTableEntries[16] },
    { "tHead", JSHTMLTableElement::THeadAttrNum, DontDelete, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "frame", JSHTMLTableElement::FrameAttrNum, DontDelete, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "constructor", JSHTMLTableElement::ConstructorAttrNum, DontDelete|DontEnum|ReadOnly, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "rules", JSHTMLTableElement::RulesAttrNum, DontDelete, 0, 0 },
    { "rows", JSHTMLTableElement::RowsAttrNum, DontDelete|ReadOnly, 0, &JSHTMLTableElementTableEntries[18] },
    { "border", JSHTMLTableElement::BorderAttrNum, DontDelete, 0, 0 },
    { "cellSpacing", JSHTMLTableElement::CellSpacingAttrNum, DontDelete, 0, 0 },
    { "width", JSHTMLTableElement::WidthAttrNum, DontDelete, 0, 0 }
};

static const HashTable JSHTMLTableElementTable = 
{
    2, 19, JSHTMLTableElementTableEntries, 15
};

/* Hash table for constructor */

static const HashEntry JSHTMLTableElementConstructorTableEntries[] =
{
    { 0, 0, 0, 0, 0 }
};

static const HashTable JSHTMLTableElementConstructorTable = 
{
    2, 1, JSHTMLTableElementConstructorTableEntries, 1
};

class JSHTMLTableElementConstructor : public DOMObject {
public:
    JSHTMLTableElementConstructor(ExecState* exec)
    {
        setPrototype(exec->lexicalInterpreter()->builtinObjectPrototype());
        putDirect(exec->propertyNames().prototype, JSHTMLTableElementPrototype::self(exec), None);
    }
    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    JSValue* getValueProperty(ExecState*, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;

    virtual bool implementsHasInstance() const { return true; }
};

const ClassInfo JSHTMLTableElementConstructor::info = { "HTMLTableElementConstructor", 0, &JSHTMLTableElementConstructorTable, 0 };

bool JSHTMLTableElementConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSHTMLTableElementConstructor, DOMObject>(exec, &JSHTMLTableElementConstructorTable, this, propertyName, slot);
}

JSValue* JSHTMLTableElementConstructor::getValueProperty(ExecState*, int token) const
{
    // The token is the numeric value of its associated constant
    return jsNumber(token);
}

/* Hash table for prototype */

static const HashEntry JSHTMLTableElementPrototypeTableEntries[] =
{
    { 0, 0, 0, 0, 0 },
    { "deleteTFoot", JSHTMLTableElement::DeleteTFootFuncNum, DontDelete|Function, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "createTHead", JSHTMLTableElement::CreateTHeadFuncNum, DontDelete|Function, 0, &JSHTMLTableElementPrototypeTableEntries[8] },
    { 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
    { "createCaption", JSHTMLTableElement::CreateCaptionFuncNum, DontDelete|Function, 0, &JSHTMLTableElementPrototypeTableEntries[9] },
    { "createTFoot", JSHTMLTableElement::CreateTFootFuncNum, DontDelete|Function, 0, 0 },
    { "deleteTHead", JSHTMLTableElement::DeleteTHeadFuncNum, DontDelete|Function, 0, &JSHTMLTableElementPrototypeTableEntries[10] },
    { "deleteCaption", JSHTMLTableElement::DeleteCaptionFuncNum, DontDelete|Function, 0, 0 },
    { "insertRow", JSHTMLTableElement::InsertRowFuncNum, DontDelete|Function, 1, &JSHTMLTableElementPrototypeTableEntries[11] },
    { "deleteRow", JSHTMLTableElement::DeleteRowFuncNum, DontDelete|Function, 1, 0 }
};

static const HashTable JSHTMLTableElementPrototypeTable = 
{
    2, 12, JSHTMLTableElementPrototypeTableEntries, 8
};

const ClassInfo JSHTMLTableElementPrototype::info = { "HTMLTableElementPrototype", 0, &JSHTMLTableElementPrototypeTable, 0 };

JSObject* JSHTMLTableElementPrototype::self(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSHTMLTableElementPrototype>(exec, "[[JSHTMLTableElement.prototype]]");
}

bool JSHTMLTableElementPrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticFunctionSlot<JSHTMLTableElementPrototypeFunction, JSObject>(exec, &JSHTMLTableElementPrototypeTable, this, propertyName, slot);
}

const ClassInfo JSHTMLTableElement::info = { "HTMLTableElement", &JSHTMLElement::info, &JSHTMLTableElementTable, 0 };

JSHTMLTableElement::JSHTMLTableElement(ExecState* exec, HTMLTableElement* impl)
    : JSHTMLElement(exec, impl)
{
    setPrototype(JSHTMLTableElementPrototype::self(exec));
}

bool JSHTMLTableElement::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSHTMLTableElement, JSHTMLElement>(exec, &JSHTMLTableElementTable, this, propertyName, slot);
}

JSValue* JSHTMLTableElement::getValueProperty(ExecState* exec, int token) const
{
    switch (token) {
    case CaptionAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return toJS(exec, WTF::getPtr(imp->caption()));
    }
    case THeadAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return toJS(exec, WTF::getPtr(imp->tHead()));
    }
    case TFootAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return toJS(exec, WTF::getPtr(imp->tFoot()));
    }
    case RowsAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return toJS(exec, WTF::getPtr(imp->rows()));
    }
    case TBodiesAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return toJS(exec, WTF::getPtr(imp->tBodies()));
    }
    case AlignAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->align());
    }
    case BgColorAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->bgColor());
    }
    case BorderAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->border());
    }
    case CellPaddingAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->cellPadding());
    }
    case CellSpacingAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->cellSpacing());
    }
    case FrameAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->frame());
    }
    case RulesAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->rules());
    }
    case SummaryAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->summary());
    }
    case WidthAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        return jsString(imp->width());
    }
    case ConstructorAttrNum:
        return getConstructor(exec);
    }
    return 0;
}

void JSHTMLTableElement::put(ExecState* exec, const Identifier& propertyName, JSValue* value, int attr)
{
    lookupPut<JSHTMLTableElement, JSHTMLElement>(exec, propertyName, value, attr, &JSHTMLTableElementTable, this);
}

void JSHTMLTableElement::putValueProperty(ExecState* exec, int token, JSValue* value, int /*attr*/)
{
    switch (token) {
    case CaptionAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setCaption(toHTMLTableCaptionElement(value));
        break;
    }
    case THeadAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setTHead(toHTMLTableSectionElement(value));
        break;
    }
    case TFootAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setTFoot(toHTMLTableSectionElement(value));
        break;
    }
    case AlignAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setAlign(valueToStringWithNullCheck(exec, value));
        break;
    }
    case BgColorAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setBgColor(valueToStringWithNullCheck(exec, value));
        break;
    }
    case BorderAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setBorder(valueToStringWithNullCheck(exec, value));
        break;
    }
    case CellPaddingAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setCellPadding(valueToStringWithNullCheck(exec, value));
        break;
    }
    case CellSpacingAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setCellSpacing(valueToStringWithNullCheck(exec, value));
        break;
    }
    case FrameAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setFrame(valueToStringWithNullCheck(exec, value));
        break;
    }
    case RulesAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setRules(valueToStringWithNullCheck(exec, value));
        break;
    }
    case SummaryAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setSummary(valueToStringWithNullCheck(exec, value));
        break;
    }
    case WidthAttrNum: {
        HTMLTableElement* imp = static_cast<HTMLTableElement*>(impl());

        imp->setWidth(valueToStringWithNullCheck(exec, value));
        break;
    }
    }
}

JSValue* JSHTMLTableElement::getConstructor(ExecState* exec)
{
    return KJS::cacheGlobalObject<JSHTMLTableElementConstructor>(exec, "[[HTMLTableElement.constructor]]");
}
JSValue* JSHTMLTableElementPrototypeFunction::callAsFunction(ExecState* exec, JSObject* thisObj, const List& args)
{
    if (!thisObj->inherits(&JSHTMLTableElement::info))
      return throwError(exec, TypeError);

    HTMLTableElement* imp = static_cast<HTMLTableElement*>(static_cast<JSHTMLTableElement*>(thisObj)->impl());

    switch (id) {
    case JSHTMLTableElement::CreateTHeadFuncNum: {


        KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->createTHead()));
        return result;
    }
    case JSHTMLTableElement::DeleteTHeadFuncNum: {

        imp->deleteTHead();
        return jsUndefined();
    }
    case JSHTMLTableElement::CreateTFootFuncNum: {


        KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->createTFoot()));
        return result;
    }
    case JSHTMLTableElement::DeleteTFootFuncNum: {

        imp->deleteTFoot();
        return jsUndefined();
    }
    case JSHTMLTableElement::CreateCaptionFuncNum: {


        KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->createCaption()));
        return result;
    }
    case JSHTMLTableElement::DeleteCaptionFuncNum: {

        imp->deleteCaption();
        return jsUndefined();
    }
    case JSHTMLTableElement::InsertRowFuncNum: {
        ExceptionCode ec = 0;
        bool indexOk;
        int index = args[0]->toInt32(exec, indexOk);
        if (!indexOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }


        KJS::JSValue* result = toJS(exec, WTF::getPtr(imp->insertRow(index, ec)));
        setDOMException(exec, ec);
        return result;
    }
    case JSHTMLTableElement::DeleteRowFuncNum: {
        ExceptionCode ec = 0;
        bool indexOk;
        int index = args[0]->toInt32(exec, indexOk);
        if (!indexOk) {
            setDOMException(exec, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }

        imp->deleteRow(index, ec);
        setDOMException(exec, ec);
        return jsUndefined();
    }
    }
    return 0;
}

}
