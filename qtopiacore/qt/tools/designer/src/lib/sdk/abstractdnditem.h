/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ABSTRACTDNDITEM_H
#define ABSTRACTDNDITEM_H

#include <QtDesigner/sdk_global.h>

QT_BEGIN_HEADER

class DomUI;
class QWidget;
class QPoint;

class QDESIGNER_SDK_EXPORT QDesignerDnDItemInterface
{
public:
    enum DropType { MoveDrop, CopyDrop };

    QDesignerDnDItemInterface() {}
    virtual ~QDesignerDnDItemInterface() {}

    virtual DomUI *domUi() const = 0;
    virtual QWidget *widget() const = 0;
    virtual QWidget *decoration() const = 0;
    virtual QPoint hotSpot() const = 0;
    virtual DropType type() const = 0;
    virtual QWidget *source() const = 0;
};

QT_END_HEADER

#endif // ABSTRACTDNDITEM_H
