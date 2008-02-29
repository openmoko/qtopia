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

#ifndef HARDWAREMANIPULATOR_H
#define HARDWAREMANIPULATOR_H

#include <QObject>

#include "qsmsmessagelist.h"

class HardwareManipulator : public QObject
{
Q_OBJECT

public:
    HardwareManipulator(QObject *parent=0);
    QSMSMessageList & getSMSList();

public slots:
    virtual void handleFromData( const QString& );
    virtual void handleToData( const QString& );
    virtual void setPhoneNumber( const QString& );

signals:
    void unsolicitedCommand(const QString &cmd);
    void command(const QString &cmd);
    void variableChanged(const QString &n, const QString &v);
    void switchTo(const QString &cmd);
    void startIncomingCall(const QString &number);

protected:
    virtual QString constructCBMessage(const QString &messageCode, int geographicalScope, const QString &updateNumber, const QString &channel,
    const QString &scheme, int language, const QString &numPages, const QString &page, const QString &content);
    virtual void constructSMSMessage(const QString &sender, const QString &serviceCenter, const QString &text);
    virtual void constructSMSDatagram(int port, const QString &sender,  const QByteArray &data, const QByteArray &contentType);

    virtual void warning(const QString &title, const QString &message);

    virtual int convertString(const QString &number, int maxValue, int numChar, int base, bool *ok);

private:
    QSMSMessageList SMSList;
};

class HardwareManipulatorFactory
{
public:
    virtual ~HardwareManipulatorFactory() {};
    inline virtual HardwareManipulator *create(QObject *p) { Q_UNUSED(p); return 0; }

    QString ruleFile() const { return ruleFilename; }
    void setRuleFile(const QString& filename) { ruleFilename = filename; }

private:
    QString ruleFilename;
};


#endif
