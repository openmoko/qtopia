/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
