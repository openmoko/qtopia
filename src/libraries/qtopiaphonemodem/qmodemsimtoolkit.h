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
#ifndef QMODEMSIMTOOLKIT_H
#define QMODEMSIMTOOLKIT_H


#include <qsimtoolkit.h>
#include <qatresult.h>

class QModemService;
class QModemSimToolkitPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemSimToolkit : public QSimToolkit
{
    Q_OBJECT
public:
    explicit QModemSimToolkit( QModemService *service );
    ~QModemSimToolkit();

    QModemService *service() const;

public slots:
    virtual void initialize();
    virtual void begin();
    virtual void end();
    virtual void sendResponse( const QSimTerminalResponse& resp );
    virtual void sendEnvelope( const QSimEnvelope& env );

protected:
    void initializationStarted();
    void initializationDone();
    void fetchCommand( int size );

private slots:
    void fetch( bool ok, const QAtResult& result );

private:
    QModemSimToolkitPrivate *d;
};

#endif // QMODEMSIMTOOLKIT_H
