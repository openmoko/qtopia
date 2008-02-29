/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef NETWORKSETTINGS_H
#define NETWORKSETTINGS_H
#include "networksettingsbase.h"

struct NetworkSettingInterface;

class NetworkSettings : public NetworkSettingsBase
{ 
    Q_OBJECT

public:
    NetworkSettings( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~NetworkSettings();

private slots:
    void serviceSelected();
    void addSetting();
    void removeSetting();
    void doProperties();
    void updateCurrent();

private:
    void removeSetting(const QString& cfgfile);
    void doProperties(const QString& cfgfile);
    void loadSettings();
    QString settingsDir() const;
    QStringList filelist;
    NetworkSettingInterface* loadPlugin(const QString& type);
};

#endif // NETWORKSETTINGS_H
