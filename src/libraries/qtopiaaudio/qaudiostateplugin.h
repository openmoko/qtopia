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

#ifndef __QAUDIOSTATEPLUGIN_H__
#define __QAUDIOSTATEPLUGIN_H__

#include <qglobal.h>
#include <qobject.h>
#include <qtopiaglobal.h>
#include <qaudionamespace.h>
#include <QList>

class QAudioState;

class QTOPIAMEDIA_EXPORT QAudioStatePlugin : public QObject
{
    Q_OBJECT

public:
    explicit QAudioStatePlugin(QObject *parent = 0);
    virtual ~QAudioStatePlugin();

    virtual QList<QAudioState *> statesProvided() const = 0;
};

#endif
