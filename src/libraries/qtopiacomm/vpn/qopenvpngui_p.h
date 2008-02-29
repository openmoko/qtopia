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

#ifndef QOPENVPNGUI_P_H
#define QOPENVPNGUI_P_H

#ifndef QTOPIA_NO_OPENVPN

#include <QWidget>
#include <qcontent.h>

class VPNConfigWidget : public QWidget {
    Q_OBJECT
public:
    explicit VPNConfigWidget( QWidget* parent = 0 )
        : QWidget( parent )
    {
    }

    ~VPNConfigWidget()
    {
    }

    void setConfigFile( const QString& cfg )
    {
        config = cfg;
        init();
    }

    virtual void init() = 0;
    virtual void save() = 0;
protected:
    void showEvent( QShowEvent* e ) {
        init();
        QWidget::showEvent(e);
    }

    void hideEvent( QHideEvent* e ) {
        save();
        QWidget::hideEvent( e );
    }

protected:
    QString config;
};


#include "ui_generalopenvpnbase.h"
class GeneralOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    GeneralOpenVPNPage( QWidget* parent = 0 );
    ~GeneralOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void forceRemoteName();

private:
    Ui::GeneralOpenVPNBase ui;
};

#include "ui_deviceopenvpnbase.h"
class DeviceOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    DeviceOpenVPNPage( QWidget* parent = 0 );
    ~DeviceOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void resetRemoteLabel( int newDevType );
private:
    Ui::DeviceOpenVPNBase ui;
};

#include "ui_certificateopenvpnbase.h"
class CertificateOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    CertificateOpenVPNPage( QWidget* parent = 0 );
    ~CertificateOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void authenticationChanged( int idx );
    void selectFile( );
private:
    Ui::CertificateOpenVPNBase ui;

    QHash<QPushButton*,QContent> toDocument;
};

#include "ui_optionsopenvpnbase.h"
class OptionsOpenVPNPage : public VPNConfigWidget
{
    Q_OBJECT
public:
    OptionsOpenVPNPage( QWidget* parent = 0 );
    ~OptionsOpenVPNPage();

protected:
    void init();
    void save();

private slots:
    void selectConfigScript();
private:
    Ui::OptionsOpenVPNBase ui;
    QContent configScript;
};

#endif //QTOPIA_NO_OPENVPN
#endif //QOPENVPNGUI_P_H
