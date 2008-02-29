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

#ifndef QBANDSELECTION_H
#define QBANDSELECTION_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <QList>

class QTOPIAPHONE_EXPORT QBandSelection : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(BandMode)
public:
    explicit QBandSelection( const QString& service = QString(),
                             QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QBandSelection();

    enum BandMode
    {
        Automatic,
        Manual
    };

public slots:
    virtual void requestBand();
    virtual void requestBands();
    virtual void setBand( QBandSelection::BandMode mode, const QString& value );

signals:
    void band( QBandSelection::BandMode mode, const QString& value );
    void bands( const QStringList& list );
    void setBandResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QBandSelection::BandMode)

#endif /* QBANDSELECTION_H */
