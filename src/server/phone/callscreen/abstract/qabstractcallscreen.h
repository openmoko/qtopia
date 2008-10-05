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

#ifndef QABSTRACTCALLSCREEN_H
#define QABSTRACTCALLSCREEN_H

#include <QWidget>

class QAbstractCallScreen : public QWidget
{
    Q_OBJECT
public:
    QAbstractCallScreen(QWidget *parent = 0, Qt::WFlags f = 0)
    : QWidget(parent, f) {}

signals:
    void acceptIncoming();
    void hangupCall();

public slots:
    virtual void stateChanged();
};

#endif
