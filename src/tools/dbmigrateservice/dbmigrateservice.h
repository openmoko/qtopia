/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextStream>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTimer>

class MigrationEngineService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    MigrationEngineService( QObject *parent );

    static bool doMigrate(const QStringList &args);
public slots:
    void doMigrate( const QDSActionRequest &request );
    void ensureTableExists( const QDSActionRequest &request );
private:
    bool ensureSchema(const QStringList &list, QSqlDatabase &db);
    bool loadSchema(QTextStream &ts, QSqlDatabase &db);

    QTimer unregistrationTimer;
private slots:
    void unregister();
};
