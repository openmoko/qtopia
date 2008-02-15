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

#ifndef CONFRECORDER_H
#define CONFRECORDER_H

#include <qdialog.h>

#include "ui_confrecorderbase.h"

struct QualitySetting;
class MediaRecorderPluginList;


class ConfigureRecorder : public QDialog
{
    Q_OBJECT

public:
    ConfigureRecorder( QualitySetting *qualities, MediaRecorderPluginList *plugins, QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ConfigureRecorder();

public:
    int currentQuality() const { return quality; }

    void processPopup();

public slots:
    void setQuality( int index );
    void saveConfig();

protected slots:
    void setChannels( int index );
    void setSampleRate( int index );
    void setFormat( int index );

private:
    void updateConfig( int channels, int frequency, const QString& mimeType, const QString& formatTag );
    void loadConfig();

private:
    Ui::ConfigureRecorderBase *conf;
    QualitySetting *qualities;
    MediaRecorderPluginList *plugins;
    int quality;
};

#endif
