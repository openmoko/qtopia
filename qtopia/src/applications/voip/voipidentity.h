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

#ifndef VOIPIDENTITY_H
#define VOIPIDENTITY_H

#include <qvariant.h>
#include <qwidget.h>
#include <qdialog.h>
#include "voipuseragentstructures.h"
#include "voipconfig.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QButtonGroup;
class QComboBox;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTabWidget;
class QTextView;
class QIntValidator;
class QGroupBox;
class QGridLayout;
class QSpacerItem;
class VScrollView;

class VOIPIdentity : public QDialog
{
    Q_OBJECT

public:
    VOIPIdentity( QWidget* parent = 0, WFlags fl = 0 );
    ~VOIPIdentity();

    QTabWidget* testTab;
    QWidget* tab;
    QLabel* label;

    QLineEdit* fullNameLE;
    QLineEdit* userSIPUriLE;
    QLineEdit* hostSIPUriLE;
    QLineEdit* outboundProxyLE;
    QLineEdit* authUsernameLE;
    QLineEdit* qValueLE;
    QLineEdit* authPasswordLE;
    QCheckBox* autoRegisterCB;
    QGroupBox* sipIdGRPBOX;
    VScrollView* mainVIScroll;
    QVBoxLayout* mainIVerticalLayout;
    QWidget *container;

    VoIPConfig* voipIdentityInfo;

    SipIdentity_t SIPIdentity;

protected:
    QVBoxLayout* VOIPIdentityLayout;
    QGridLayout* sipIdGRPBOXLayout;
    QHBoxLayout* autoRegHLayout;

private slots:
    void voipIdentityCancelClicked();
    void getPreferences(SipIdentity_t&);
    void setPreferences(SipIdentity_t&);
    void checkRegister();
    void accept();
};

#endif // VOIPIDENTITY_H
