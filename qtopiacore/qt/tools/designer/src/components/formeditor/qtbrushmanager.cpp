/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtbrushmanager.h"
#include <QPixmap>
#include <QPainter>

using namespace qdesigner_internal;

namespace qdesigner_internal {

class QtBrushManagerPrivate
{
    QtBrushManager *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushManager)
public:
    QMap<QString, QBrush> theBrushMap;
    QString theCurrentBrush;
};

}  // namespace qdesigner_internal

QtBrushManager::QtBrushManager(QObject *parent)
    : QDesignerBrushManagerInterface(parent)
{
    d_ptr = new QtBrushManagerPrivate;
    d_ptr->q_ptr = this;

}

QtBrushManager::~QtBrushManager()
{
    delete d_ptr;
}

QBrush QtBrushManager::brush(const QString &name) const
{
    if (d_ptr->theBrushMap.contains(name))
        return d_ptr->theBrushMap[name];
    return QBrush();
}

QMap<QString, QBrush> QtBrushManager::brushes() const
{
    return d_ptr->theBrushMap;
}

QString QtBrushManager::currentBrush() const
{
    return d_ptr->theCurrentBrush;
}

QString QtBrushManager::addBrush(const QString &name, const QBrush &brush)
{
    if (name.isNull())
        return QString();

    QString newName = name;
    QString nameBase = newName;
    int i = 0;
    while (d_ptr->theBrushMap.contains(newName)) {
        newName = nameBase + QString::number(++i);
    }
    d_ptr->theBrushMap[newName] = brush;
    emit brushAdded(newName, brush);

    return newName;
}

void QtBrushManager::removeBrush(const QString &name)
{
    if (!d_ptr->theBrushMap.contains(name))
        return;
    if (currentBrush() == name)
        setCurrentBrush(QString());
    emit brushRemoved(name);
    d_ptr->theBrushMap.remove(name);
}

void QtBrushManager::setCurrentBrush(const QString &name)
{
    QBrush newBrush;
    if (!name.isNull()) {
        if (d_ptr->theBrushMap.contains(name))
            newBrush = d_ptr->theBrushMap[name];
        else
            return;
    }
    d_ptr->theCurrentBrush = name;
    emit currentBrushChanged(name, newBrush);
}

QPixmap QtBrushManager::brushPixmap(const QBrush &brush) const
{
    int w = 64;
    int h = 64;

    /*
    int pixSize = 5;
    QPixmap pm(2 * pixSize, 2 * pixSize);
    QPainter pmp(&pm);
    pmp.fillRect(0, 0, pixSize, pixSize, Qt::lightGray);
    pmp.fillRect(pixSize, pixSize, pixSize, pixSize, Qt::lightGray);
    pmp.fillRect(0, pixSize, pixSize, pixSize, Qt::darkGray);
    pmp.fillRect(pixSize, 0, pixSize, pixSize, Qt::darkGray);
    */

    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(QRect(0, 0, w, h), brush);
    /*
    if (brush.style() == Qt::LinearGradientPattern ||
            brush.style() == Qt::RadialGradientPattern ||
            brush.style() == Qt::ConicalGradientPattern) {
        p.scale(w, h);
        p.fillRect(QRect(0, 0, 1, 1), brush);
    } else {
        p.fillRect(QRect(0, 0, w, h), brush);
    }
    */

    return QPixmap::fromImage(img);
}

