/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __C3200_AUDIO_PLUGIN_H__
#define __C3200_AUDIO_PLUGIN_H__

#include <QAudioStatePlugin>

class C3200AudioPluginPrivate;

class C3200AudioPlugin : public QAudioStatePlugin
{
    Q_OBJECT
    friend class C3200AudioPluginPrivate;

public:
    C3200AudioPlugin(QObject *parent = 0);
    ~C3200AudioPlugin();

    QList<QAudioState *> statesProvided() const;

private:
    C3200AudioPluginPrivate *m_data;
};

#endif
