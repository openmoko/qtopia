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

#ifndef VOIPMAIN_H
#define VOIPMAIN_H

#include <qvariant.h>
#include <qwidget.h>
#include <qscrollview.h>
#include "voipuseragentstructures.h"
#include "voipconfig.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QComboBox;
class QLabel;
class QAction;
class VOIPSettings;
class VOIPIdentity;
class QGridLayout;
class QHBoxLayout;
class QSpacerItem;
class QPixmap;
class VScrollView;

class VOIPMain : public QWidget
{
    Q_OBJECT

public:
    VOIPMain( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~VOIPMain();

    QLabel* label;
    QLabel* idIMG;
    QLabel* userName_LBL;
    QLabel* sipIdentity_LBL;
    QLabel* serverName_LBL;
    QLabel* serverIMG;
    QLabel* regStatus_LBL;
    QLabel* regStatusIMG;
    QLabel* voipStatusLBL;
    QComboBox* voipStatusCombo;
    QComboBox* testCombo;
    QSpacerItem* idVSpacer;
    VScrollView* mainVScroll;
    QVBoxLayout* mainVerticalLayout;
    QWidget* container;

    VoIPConfig* voipRegInfo;
    QString updatedStatus;

    RegistrationStatus_t eRegStatus;
    AvailabilityStatus_t availStatus;

    VOIPIdentity* identityDialog;
    VOIPSettings* settingsDialog;

    QAction* voip_register;
    QAction* voip_identity;
    QAction* voip_settings;
    QAction* voip_logout;

protected:
    VScrollView* mView;
    QVBoxLayout* VOIPMainLayout;
    QHBoxLayout* identityIMGHLayout;
    QHBoxLayout* idInfoHLayout;
    QVBoxLayout* idInfoVLayout;
    QHBoxLayout* serverIMGHLayout;
    QHBoxLayout* serverInfoHLayout;
    QHBoxLayout* regStatusIMGHLayout;
    QHBoxLayout* regStatusInfoHLayout;
    QVBoxLayout* voipStatusHLayout;

    void keyPressEvent( QKeyEvent *e );

private slots:
    void setRegistrationMessage(int regMessage);
    void setUserIdentityInfo(SipIdentity_t&);
    void sendAvailabilityStatus(const QString& availStatus);
    void setCurrentAvailabilityStatus(int);
    void voipAppClose();
    void voipRegister();
    void voipSettings();
    void voipIdentity();
    void voipAppLogout();

};

#endif // VOIPMAIN_H
