/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
/****************************************************************************
**
** Alternative Launcher for X11 integrating with existing environments
**
** Copyright (C) 2008 by OpenMoko, Inc.
** Written by Holger Freyther
** All Rights Reserved
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
****************************************************************************/
#ifndef PHONE_LAUNCHER_X11
#define PHONE_LAUNCHER_X11

#include "qabstractserverinterface.h"
#include "qsupplementaryservices.h"
#include "cellbroadcastcontrol.h"


class QAbstractMessageBox;
class QAbstractHomeScreen;
class CallHistory;
class ContextLabel;

class PhoneLauncherX11 : public QAbstractServerInterface {
    Q_OBJECT
public:
    PhoneLauncherX11(QWidget* parent = 0, Qt::WFlags fl = Qt::FramelessWindowHint);
    ~PhoneLauncherX11();

private Q_SLOTS:
#ifdef QTOPIA_CELL
    void unstructuredNotification (QSupplementaryServices::UnstructuredAction action, const QString& data);
    void cellBroadcast(CellBroadcastControl::Type, const QString &, const QString &);
#endif

    void showCallHistory(bool missed = false, const QString &hint = QString());
    void ussdMessage(const QString& text);
    void initializeCallHistory();


private:
    void createContext();
    CallHistory *callHistory() const { return m_callHistory; }

    ContextLabel* m_context;
    QAbstractMessageBox* m_cbsMessageBox;
    QAbstractMessageBox* m_serviceMessageBox;
    QAbstractHomeScreen* m_homeScreen;
    CallHistory* m_callHistory;
};

#endif
