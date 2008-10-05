/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QCOMMINTERFACE_H
#define QCOMMINTERFACE_H

#include <qabstractipcinterface.h>

class QTOPIACOMM_EXPORT QCommInterface : public QAbstractIpcInterface
{
    Q_OBJECT
public:
    explicit QCommInterface( const QString& interfaceName,
                             const QString& service = QString(),
                             QObject *parent = 0,
                             QAbstractIpcInterface::Mode mode = Client );
    ~QCommInterface();

    QString service() const;
};

#endif
