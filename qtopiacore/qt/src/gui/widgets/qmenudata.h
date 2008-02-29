/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMENUDATA_H
#define QMENUDATA_H

#include <QtCore/qglobal.h>

#ifdef QT3_SUPPORT
#include <QtGui/qaction.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class Q_GUI_EXPORT QMenuItem : public QAction
{
    Q_OBJECT

public:
    QMenuItem();

    QT3_SUPPORT int id() const;
    QT3_SUPPORT int signalValue() const;
private:
    friend class QMenu;
    friend class QMenuBar;
    void setId(int);
    void setSignalValue(int);
};

QT_END_HEADER

#endif

#endif // QMENUDATA_H
