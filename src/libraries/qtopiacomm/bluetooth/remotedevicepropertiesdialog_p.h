/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef __REMOTEDEVICEPROPERTIESDIALOG_P_H__
#define __REMOTEDEVICEPROPERTIESDIALOG_P_H__

#include <qbluetoothremotedevice.h>
#include <qsdap.h>

#include <QDialog>

class QSDAPSearchResult;
class QWaitWidget;
class QBluetoothAddress;
class QBluetoothLocalDevice;
class QLineEdit;
class QTextEdit;
class QLabel;
class QHBoxLayout;

template <class T> class QList;

class RemoteDevicePropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteDevicePropertiesDialog( const QBluetoothAddress &localAddr,
                                           QWidget *parent = 0,
                                           Qt::WFlags flags = 0 );
    ~RemoteDevicePropertiesDialog();

    void setRemoteDevice( const QBluetoothRemoteDevice &remote,
                          const QPixmap &icon );

private slots:
    void foundServices( const QSDAPSearchResult &result );
    void showServices();
    void checkAliasEdit();
    void setDeviceAlias();

private:
    void initMainInfo();
    void resetMainInfo( const QString &displayName, const QPixmap &pixmap );

    static const QString SERVICE_ERROR_MSG;

    QBluetoothRemoteDevice m_remote;
    QBluetoothLocalDevice *m_local;

    QSDAP m_sdap;
    QWaitWidget *m_waitWidget;
    QDialog *m_servicesDialog;
    QTextEdit *m_servicesText;

    QLineEdit *m_title;
    QHBoxLayout *m_headingLayout;
    QLineEdit *m_aliasLineEdit;
    QString m_prevAlias;

    QLabel *m_icon;
    QList<QLineEdit *> m_devAttrs;
};

#endif
