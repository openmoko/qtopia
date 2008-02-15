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

#ifndef QOPENVPN_P_H
#define QOPENVPN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTOPIA_NO_OPENVPN

#include "qvpnclient.h"
#include <QProcess>

class QOpenVPN : public QVPNClient
{
    Q_OBJECT
public:
    explicit QOpenVPN( QObject* parent = 0 );
    explicit QOpenVPN( bool serverMode, uint vpnID, QObject* parent = 0 );
    ~QOpenVPN();

    QVPNClient::Type type() const;
    QVPNClient::State state() const;
    QString name() const;

    QDialog* configure( QWidget* parent = 0 );

    void connect();
    void disconnect();

    void cleanup();

protected:
    void timerEvent( QTimerEvent* e );

private slots:
    void stateChanged( QProcess::ProcessState newState );
    void stateChanged();

private:
    int logIndex;
    int pendingID;
    int killID;
    QStringList parameter(bool *error) const;
    QVPNClient::State lastState;
};

class QLabel;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QTabWidget;

class GeneralOpenVPNPage;
class CertificateOpenVPNPage;
class OptionsOpenVPNPage;

class QOpenVPNDialog : public QDialog
{
    Q_OBJECT
public:
    QOpenVPNDialog( const QString& config, QWidget* parent = 0 );
    ~QOpenVPNDialog();

public slots:
    void accept();
private:
    void init();

private slots:
    void listSelected(QListWidgetItem*);
    void showUserHint( QListWidgetItem*, QListWidgetItem* );

private:
    QString config;
    QLabel* userHint;
    QStackedWidget* stack;
    QListWidget* list;
};
#endif //QTOPIA_NO_OPENVPN
#endif //QOPENVPN_P_H
