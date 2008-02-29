/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __GREENPHONE_AUDIO_PLUGIN_H__
#define __GREENPHONE_AUDIO_PLUGIN_H__

#include <QAudioStatePlugin>

class GreenphoneAudioPluginPrivate;

class GreenphoneAudioPlugin : public QAudioStatePlugin
{
    Q_OBJECT
    friend class GreenphoneAudioPluginPrivate;

public:
    GreenphoneAudioPlugin(QObject *parent = 0);
    ~GreenphoneAudioPlugin();

    QList<QAudioState *> statesProvided() const;

private:
    GreenphoneAudioPluginPrivate *m_data;
};

#endif
