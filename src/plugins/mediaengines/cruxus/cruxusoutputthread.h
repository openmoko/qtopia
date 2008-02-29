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

#ifndef __CRUXUS_OUTPUTTHREAD_H
#define __CRUXUS_OUTPUTTHREAD_H

#include <QMediaDevice>

class QMediaDecoder;

namespace cruxus
{

class OutputThreadPrivate;

class OutputThread : public QMediaDevice
{
    Q_OBJECT

public:
    OutputThread();
    ~OutputThread();

    void connectInputPipe(QMediaPipe* inputPipe);
    void connectOutputPipe(QMediaPipe* outputPipe);

    void disconnectInputPipe(QMediaPipe* inputPipe);
    void disconnectOutputPipe(QMediaPipe* outputPipe);

    void setValue(QString const& name, QVariant const& value);
    QVariant value(QString const& name);

private slots:
    void readFromPipe();

private:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

    OutputThreadPrivate*    d;
};

}   // ns cruxus

#endif  // __CRUXUS_OUTPUTTHREAD_H

