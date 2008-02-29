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

#ifndef Q3DROPSITE_H
#define Q3DROPSITE_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class QWidget;

class Q_COMPAT_EXPORT Q3DropSite {
public:
    Q3DropSite(QWidget* parent);
    virtual ~Q3DropSite();
};

QT_END_HEADER

#endif  // QDROPSITE_H
