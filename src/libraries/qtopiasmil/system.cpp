/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "system.h"
#include "transfer.h"
#include "element.h"
#include "module.h"
#include "timing.h"
#include "layout.h"
#include <QPainter>
#include <QWidget>

SmilSystem::SmilSystem() : QObject(), root(0)
{
    xferServer = new SmilTransferServer(this);
}

SmilSystem::~SmilSystem()
{
    delete root;
    QMap<QString,SmilModule *>::Iterator it;
    for (it = mods.begin(); it != mods.end(); ++it)
        delete (*it);
}

void SmilSystem::setRootElement(SmilElement *r)
{
    root = r;
    root->setRect(targetWidget->rect());
    QMap<QString, SmilModule *>::ConstIterator it;
    for (it = modules().begin(); it != modules().end(); ++it)
        (*it)->process();
    root->process();
}

SmilElement *SmilSystem::findElement(SmilElement *e, const QString &id) const
{
    if (!e)
        e = root;
    if (e->id() == id)
        return e;
    SmilElementList::ConstIterator it;
    for (it = e->children().begin(); it != e->children().end(); ++it) {
        SmilElement *fe = findElement(*it, id);
        if (fe)
            return fe;
    }

    return 0;
}

QColor SmilSystem::rootColor() const
{
    QColor col(Qt::white);

    SmilElementList::ConstIterator it;
    for (it = root->children().begin(); it != root->children().end(); ++it) {
        if ((*it)->name() == "root-layout") {
            col = ((SmilRootLayout*)(*it))->backgroundColor();
            break;
        }
    }

    return col;
}

void SmilSystem::play()
{
    SmilElementList::ConstIterator it;
    for (it = root->children().begin(); it != root->children().end(); ++it) {
        if ((*it)->name() == "body") {
            SmilTimingAttribute *tn = (SmilTimingAttribute*)(*it)->module("Timing");
            tn->setState(SmilElement::Startup);
        }
    }
}

void SmilSystem::reset()
{
    root->reset();
}

void SmilSystem::setDirty(const QRect &r)
{
    updRgn += r;
}

void SmilSystem::update(const QRect &r)
{
    updRgn += r;
    targetWidget->update(updRgn);
}

void SmilSystem::paint(QPainter *p)
{
    paint(root, p);
}

void SmilSystem::paint(SmilElement *e, QPainter *p)
{
    if (e->isVisible() && e->state() >= SmilElement::Active) {
        e->paint(p);
        SmilElementList::ConstIterator it;
        for (it = e->children().begin(); it != e->children().end(); ++it) {
            paint(*it, p);
        }
    }
}

void SmilSystem::bodyFinished()
{
    emit finished();
}
