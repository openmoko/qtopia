/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef SECURITY_H
#define SECURITY_H

#ifdef QTOPIA_PHONE
#include "securityphone.h"
#include <qtopia/phone/phone.h>
#else
#include "securitybase.h"
#endif

#include <qdatetime.h>

class QPEDialogListener;
class WaitScreen;
class PhoneSecurity;

class Security : public SecurityBase
{ 
    Q_OBJECT

public:
    Security( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Security();

    void show();

protected:

#ifdef QTOPIA_PHONE
    bool close(bool);
#else
    void accept();
    void done(int);
#endif

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

    void changePhoneCode();
    void secureLockChanged(int);

private:
    bool telnetAvailable() const;
    bool sshAvailable() const;

    static bool parseNet(const QString& sn,int& auth_peer,int& auth_peer_bits);
    void selectNet(int auth_peer,int auth_peer_bits);
    
    QString enterPassCode(const QString&, bool encrypt = TRUE);
    QString passcode;
    QString phoneCode;
    bool valid;
    QTime timeout;

#ifdef QTOPIA_PHONE
    WaitScreen *mStatus;
    PhoneSecurity *phonesec;
		    // confirm pin number.
#else
    QPEDialogListener *dl;
#endif
};


#endif // SECURITY_H

