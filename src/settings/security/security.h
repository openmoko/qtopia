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
#ifndef SECURITY_H
#define SECURITY_H

#ifdef QTOPIA_CELL
#include "ui_securityphone.h"
#else
#include "ui_securitybase.h"
#endif

#include <QDialog>
#include <QDateTime>

class WaitScreen;
class PhoneSecurity;

class Security
    : public QDialog, public Ui::SecurityBase
{
    Q_OBJECT

public:
    Security( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~Security();

    void setVisible(bool vis);

protected:
    void accept();
    void done(int);
    void applySecurity();

private slots:
    void changePassCode();
    void clearPassCode();
    void setSyncNet(const QString&);

    void markProtected(bool);
    void updateGUI();

    void phoneChanged(bool success);
    void phoneLocked(bool success);
    void phoneLockDone(bool success);

private:
    bool telnetAvailable() const;
    bool sshAvailable() const;

    static bool parseNet(const QString& sn,int& auth_peer,int& auth_peer_bits);
    void selectNet(int auth_peer,int auth_peer_bits);

    QString enterPassCode(const QString&, bool encrypt = true, bool last = true);
    QString passcode;
    bool valid;
    QTime timeout;

#ifdef QTOPIA_CELL
    WaitScreen *mStatus;
    PhoneSecurity *phonesec;
                    // confirm pin number.
#endif
};


#endif // SECURITY_H

