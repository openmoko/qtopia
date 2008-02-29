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

#ifndef __QTOPIA_MEDIALIBRARY_VIDEOCONTROL_H
#define __QTOPIA_MEDIALIBRARY_VIDEOCONTROL_H

#include <quuid.h>
#include <qobject.h>

#include <qtopiaglobal.h>
#include "qmediahandle.h"

class QWidget;


class QMediaVideoControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaVideoControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaVideoControl(QMediaHandle const& mediaHandle, QObject* parent = 0);
    ~QMediaVideoControl();

    QWidget* createVideoWidget(QWidget* parent = 0) const;

    static QString name();

private:
    QMediaVideoControlPrivate*    d;
};

#endif  // __QTOPIA_MEDIALIBRARY_VIDEOCONTROL_H
