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

#ifndef CONFRECORDER_H
#define CONFRECORDER_H

#include "confrecorderbase.h"


struct QualitySetting;
class MediaRecorderPluginList;


class ConfigureRecorder : public ConfigureRecorderBase
{
    Q_OBJECT

public:
    ConfigureRecorder( QualitySetting *qualities, MediaRecorderPluginList *plugins, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~ConfigureRecorder();

public:
    int currentQuality() const { return quality; }

public slots:
    void setQuality( int index );
    void saveConfig();

protected slots:
    void setChannels( int index );
    void setSampleRate( int index );
    void setFormat( int index );
    void resetQuality();

private:
    void updateConfig( int channels, int frequency, const QString& mimeType, const QString& formatTag );
    void loadConfig();

private:

    QualitySetting *qualities;
    MediaRecorderPluginList *plugins;
    int quality;

};

#endif
