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

#ifndef RDSPROGRAMINFO_H
#define RDSPROGRAMINFO_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include "radioband.h"

class RdsProgramInfoPrivate;
class RdsGroup;

class RdsProgramInfo : public QObject
{
    Q_OBJECT
public:
    RdsProgramInfo( QObject *parent = 0 );
    ~RdsProgramInfo();

    void clear();
    void addGroup( const RdsGroup& group );

    int piCode() const;
    int ptyCode() const;
    bool trafficProgram() const;
    bool trafficAnnouncement() const;
    bool isMusic() const;
    bool isSpeech() const;
    QString programName() const;
    QString programType() const;
    QString radioText() const;
    QList<RadioBand::Frequency> alternateFrequencies() const;

signals:
    void timeNotification( const QDateTime& time, int zone );

private:
    RdsProgramInfoPrivate *d;
};

#endif
