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

#include "qaudiostateplugin.h"

#include <QSet>
#include <QPair>

/*!
  \class QAudioStatePlugin
  */

/*!
    Constructs a QAudioStatePlugin object with parent set to \a parent.
*/
QAudioStatePlugin::QAudioStatePlugin(QObject *parent)
    : QObject(parent)
{
    
}

/*!
    Destroys a QAudioStatePlugin object.
*/

QAudioStatePlugin::~QAudioStatePlugin()
{
    
}

/*!
    Returns a list of all QAudioStates provided by this plugin.
*/
QList<QAudioState *> QAudioStatePlugin::statesProvided() const
{
    return QList<QAudioState *>();
}
