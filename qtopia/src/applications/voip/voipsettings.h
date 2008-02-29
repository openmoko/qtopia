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

#ifndef VOIPSETTINGS_H
#define VOIPSETTINGS_H

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
class QGroupBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QTabWidget;
class QTextView;
class QIntValidator;
class VScrollView;

class VOIPSettings : public QDialog
{
    Q_OBJECT

public:
    VOIPSettings( QWidget* parent = 0, WFlags fl = 0 );
    ~VOIPSettings();

    QLabel* label;
    QLabel* devRdOnlylabel;
    QTabWidget* settingsTab;
    VScrollView* sipTab;
    QButtonGroup* hideViaBTNGRP;
    QRadioButton* dontHideRB;
    QRadioButton* reqNextHopRB;
    QRadioButton* reqFullRouteCB;
    QLineEdit* maxForwardsLE;
    QLineEdit* expireTimeRegLE;
    QLineEdit* expireTimeSubsLE;
    VScrollView* audioTab;
    QButtonGroup* ossDevModeBTNGRP;
    QRadioButton* readWriteRB;
    QRadioButton* rdonlyWronlyRB;
    QLineEdit* deviceRdWrLE;
    QLineEdit* deviceRdLE;
    QComboBox* preferredCodecCombo;
    QButtonGroup* payloadSizeBTNGRP;
    QRadioButton* payloadSize80RB;
    QRadioButton* payloadSize160RB;
    QRadioButton* payloadSize240RB;
    VScrollView* socketTab;
    QComboBox* socketProtocolCombo;
    QCheckBox* symmetricSignallingChkBox;
    QCheckBox* symmetricMediaChkBox;
    QCheckBox* useSTUNServerChkBox;
    QGroupBox* stunServerOptGRPBOX;
    QLineEdit* mediaMinPortLE;
    QLineEdit* stunServerLE;
    QLineEdit* mediaMaxPortLE;
    QLineEdit* reqPeriodSTUNServerLE;
    VScrollView* callTab;
    QGroupBox* callTabGRPBOX;
    QTextView* bussyMessageTV;

protected:
    QVBoxLayout* VOIPSettingsLayout;
    QVBoxLayout* sipTabLayout;
    QGridLayout* hideViaBTNGRPLayout;
    QVBoxLayout* audioTabLayout;
    QVBoxLayout* ossDevModeBTNGRPLayout;
    QVBoxLayout* ossDevGLayout;
    QHBoxLayout* preferredCodecHLayout;
    QGridLayout* payloadSizeBTNGRPLayout;
    QVBoxLayout* socketTabLayout;
    QHBoxLayout* socketProtocolHLayout;
    QVBoxLayout* stunServerOptGRPBOXLayout;
    QGridLayout* settingTabOptLayout;
    QVBoxLayout* callTabLayout;
    QVBoxLayout* callTabGRPBOXLayout;

    QVBoxLayout* mainSipTabVerticalLayout;
    QVBoxLayout* mainAudioTabVerticalLayout;
    QVBoxLayout* mainSocketTabVerticalLayout;
    QVBoxLayout* mainCallTabVerticalLayout;
    QHBoxLayout* symSigHLayout;
    QHBoxLayout* symMedHLayout;
    QHBoxLayout* stunChkHLayout;

    QIntValidator* intValidator;
    VoIPConfig* voipSettingsInfo;

    SipPreferences_t   SIPPreferences;
    AudioPreferences_t AudioPreferences;

private slots:
    void rdonlyWronlySelected();
    void stunServerSelected();
    void setProperPayload(const QString& );
    void getPreferences(SipPreferences_t&, AudioPreferences_t&);
    void setSipPreferences( SipPreferences_t& );
    void setAudioPreferences( AudioPreferences_t& );
    void accept();
};

#endif // VOIPSETTINGS_H
