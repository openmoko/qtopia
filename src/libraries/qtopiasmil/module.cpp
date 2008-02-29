/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "module.h"
#include "element.h"

SmilModule::~SmilModule()
{
}

QStringList SmilModule::elements() const
{
    QStringList l;
    return l;
}

QStringList SmilModule::attributes() const
{
    QStringList l;
    return l;
}

void SmilModule::process()
{
}

//===========================================================================

SmilModuleAttribute::SmilModuleAttribute(SmilElement *e, const QXmlAttributes &/*atts*/)
    : element(e)
{
    e->addModule(this);
}

SmilModuleAttribute::~SmilModuleAttribute()
{
}

void SmilModuleAttribute::process()
{
}

void SmilModuleAttribute::reset()
{
}

void SmilModuleAttribute::event(SmilElement *, SmilEvent *)
{
}

QString SmilModuleAttribute::name() const
{
    return "";
}

void SmilModuleAttribute::paint(QPainter *)
{
}

