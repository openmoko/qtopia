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

#include "addwapui.h"

#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QSettings>
#include <QStackedWidget>
#include <QTabWidget>

#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qtopianetworkinterface.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

#include "ui_gatewaybase.h"
#include "ui_mmsbase.h"
#include "ui_browserbase.h"

enum PageEnum {
   Account, WAPGateway, MMS, Browsing
} Page;


class WapAccountPage : public QWidget
{
Q_OBJECT
public:
    WapAccountPage( QString file, QWidget* parent = 0, Qt::WFlags fl = 0)
        : QWidget( parent, fl )
    {
        QVBoxLayout* vb = new QVBoxLayout( this );
        vb->setMargin( 5 );
        vb->setSpacing( 4 );

        QLabel * account_label = new QLabel( tr("Name:"), this );
        vb->addWidget( account_label );

        account = new QLineEdit( this );
        vb->addWidget( account );

        QLabel * dataAccount_label = new QLabel(tr("Use data account:"), this);
        vb->addWidget(dataAccount_label);

        dataAccount = new QComboBox( this );
        vb->addWidget( dataAccount );

        QSpacerItem* spacer = new QSpacerItem( 10, 100,
                QSizePolicy::Minimum, QSizePolicy::Expanding );
        vb->addItem( spacer );

        QSettings cfg( file, QSettings::IniFormat );
        QString name = cfg.value("Info/Name").toString();
        if ( name.isEmpty() )
            account->setText( tr("WAP account", "name of default wap account") );
        else
            account->setText( name );

        QString accountConfig = cfg.value("Info/DataAccount").toString();
        dataConfigs = QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::GPRS );

        int found = -1;
        for( int i = 0; i<dataConfigs.count(); i++ ) {
            QSettings dataCfg( dataConfigs[i], QSettings::IniFormat );
            dataAccount->addItem( dataCfg.value("Info/Name").toString() );
            if (dataConfigs[i] == accountConfig)
                found = i;
        }

        if ( dataAccount->count() == 0 ) {
            dataAccount->setEnabled( false );
            dataAccount->addItem( tr("<No GPRS account>") );
            dataAccount->setCurrentIndex( 0 );
        } else {
            dataAccount->setCurrentIndex( found );
        }

#ifdef QTOPIA_PHONE
        QtopiaApplication::setInputMethodHint( account, QtopiaApplication::Words );
        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
#endif
        setObjectName("wap-account");
    }

    virtual ~WapAccountPage()
    {
    }

    QtopiaNetworkProperties properties()
    {
        QtopiaNetworkProperties prop;
        prop.insert("Info/Name", account->text());

        int idx = dataAccount->currentIndex();
        if (idx >= 0 && idx < dataAccount->count() && dataConfigs.count() )
            prop.insert("Info/DataAccount", dataConfigs[idx]);
        else
            prop.insert("Info/DataAccount", "");

        return prop;
    }

private:
    QStringList dataConfigs;
    QLineEdit* account;
    QComboBox* dataAccount;
};

class GatewayPage: public QWidget
{
public:
    GatewayPage( QString file, QWidget* parent = 0, Qt::WFlags fl = 0)
        :QWidget( parent, fl )
    {
        ui.setupUi( this );
#ifdef QTOPIA_PHONE
        QtopiaApplication::setInputMethodHint( ui.gateway, QtopiaApplication::Text );
        QtopiaApplication::setInputMethodHint( ui.username, QtopiaApplication::Text );
        QtopiaApplication::setInputMethodHint( ui.password, QtopiaApplication::Text );
#endif

        QSettings cfg( file, QSettings::IniFormat );
        ui.gateway->setText( cfg.value("Wap/Gateway").toString() );
        ui.port->setValue( cfg.value("Wap/Port").toInt());
        ui.username->setText( cfg.value("Wap/UserName").toString() );
        ui.password->setText( cfg.value("Wap/Password").toString() );
        ui.password->setEchoMode( QLineEdit::PasswordEchoOnEdit );

#ifdef QTOPIA_PHONE
        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
#endif
        setObjectName("wap-gateway");
    };

    QtopiaNetworkProperties properties()
    {
        QtopiaNetworkProperties prop;
        prop.insert("Wap/Gateway", ui.gateway->text());
        prop.insert("Wap/Port", ui.port->value());
        prop.insert("Wap/UserName", ui.username->text());
        prop.insert("Wap/Password", ui.password->text());
        return prop;
    };
private:
    Ui::GatewayBase ui;
};

struct ExpiryTime {
    int expiry;
    const char *desc;
};

static const ExpiryTime expiryTimes[] = {
    { 0, QT_TRANSLATE_NOOP("MMSPage", "Maximum") },
    { 1, QT_TRANSLATE_NOOP("MMSPage", "1 Hour") },
    { 2, QT_TRANSLATE_NOOP("MMSPage", "2 Hours") },
    { 6, QT_TRANSLATE_NOOP("MMSPage", "6 Hours") },
    { 12, QT_TRANSLATE_NOOP("MMSPage", "12 Hours") },
    { 24, QT_TRANSLATE_NOOP("MMSPage", "1 Day") },
    { 48, QT_TRANSLATE_NOOP("MMSPage", "2 Days") },
    { 72, QT_TRANSLATE_NOOP("MMSPage", "3 Days") },
    { 0, 0 }
};


class MMSPage: public QWidget
{
public:
    MMSPage( QString file, QWidget* parent = 0, Qt::WFlags fl = 0)
        :QWidget( parent, fl )
    {
        ui.setupUi( this );
#ifdef QTOPIA_PHONE
        QtopiaApplication::setInputMethodHint( ui.mms, QtopiaApplication::Text );
#endif

        QSettings cfg( file, QSettings::IniFormat );
        QString server = cfg.value("MMS/Server").toString();
        if ( !server.isEmpty() )
            ui.mms->setText( server );

        int idx = 0;
        while (expiryTimes[idx].desc) {
            ui.expiry->addItem(qApp->translate("MMSPage", expiryTimes[idx].desc));
            idx++;
        }

        idx = 0;
        int exp = cfg.value("MMS/Expiry").toInt();
        while (expiryTimes[idx].desc && expiryTimes[idx].expiry < exp)
            idx++;
        if (!expiryTimes[idx].desc)
            idx = 0;
        ui.expiry->setCurrentIndex(idx);

        QString visibility = cfg.value("MMS/Visibility").toString();
        if ( visibility == "show" )
            ui.visibility->setCurrentIndex( 1 );
        else if ( visibility == "hidden" )
            ui.visibility->setCurrentIndex( 2 );
        else
            ui.visibility->setCurrentIndex( 0 );

        if ( cfg.value("MMS/AllowDeliveryReport").toString() != "y" )
            ui.delivery->setCheckState( Qt::Unchecked );
        else
            ui.delivery->setCheckState( Qt::Checked );

#ifdef QTOPIA_PHONE
        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
#endif
        setObjectName("wap-mms");
    };

    QtopiaNetworkProperties properties()
    {
        QtopiaNetworkProperties prop;
        prop.insert("MMS/Server", ui.mms->text());

        prop.insert("MMS/Expiry",
                expiryTimes[ui.expiry->currentIndex()].expiry );

        int idx = ui.visibility->currentIndex();
        switch (idx) {
            case 0:
            default:
                prop.insert( "MMS/Visibility", "default" );
                break;
            case 1:
                prop.insert( "MMS/Visibility", "show" );
                break;
            case 2:
                prop.insert( "MMS/Visibility", "hidden" );
                break;
        }

        if ( ui.delivery->checkState() == Qt::Checked )
            prop.insert("MMS/AllowDeliveryReport", "y");
        else
            prop.insert("MMS/AllowDeliveryReport", "n");


        return prop;
    };
private:
    Ui::MMSBase ui;
};

class BrowserPage : public QWidget
{
public:
    BrowserPage( QString file, QWidget* parent = 0, Qt::WFlags fl = 0)
        :QWidget( parent, fl )
    {
        ui.setupUi( this );

        QSettings cfg( file, QSettings::IniFormat );
        if ( cfg.value("Browser/ShowPictures").toString() != "y" )
            ui.showPics->setCheckState(Qt::Unchecked);
        else
            ui.showPics->setCheckState( Qt::Checked );

        QString cookies = cfg.value("Browser/Cookies").toString();
        if ( cookies == "Confirm" )
            ui.confirm->setChecked( true );
        else if ( cookies == "Accept" )
            ui.accept->setChecked( true );
        else
            ui.reject->setChecked( true );
#ifdef QTOPIA_PHONE
        QSoftMenuBar::menuFor( this );
        QSoftMenuBar::setHelpEnabled( this, true );
#endif
        setObjectName("wap-browser");

    }

    QtopiaNetworkProperties properties()
    {
        QtopiaNetworkProperties prop;
        if ( ui.showPics->checkState() == Qt::Checked )
            prop.insert("Browser/ShowPictures", "y");
        else
            prop.insert("Browser/ShowPictures", "n");

        if ( ui.confirm->isChecked() )
            prop.insert("Browser/Cookies", "Confirm");
        else if ( ui.accept->isChecked() )
            prop.insert("Browser/Cookies", "Accept");
        else
            prop.insert("Browser/Cookies", "Reject");

        return prop;
    }

private:
    Ui::BrowserBase ui;
};



AddWapUI::AddWapUI( QString file, QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal( true );

#ifdef QTOPIA_PHONE
    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this, true );
#endif
    setObjectName("wap-menu");

    if ( file.isEmpty() || !QFile::exists(file) ) {
        QString path = Qtopia::applicationFileName("Network", "wap") + '/';
        QString newName = path +"wap.conf";
        int index = 0;
        while ( QFile::exists(newName) ) {
            index++;
            newName = path+"wap"+QString::number(index)+".conf";
        }
        configFile = newName;
    } else {
        configFile = file;
    }

    init();
}

AddWapUI::~AddWapUI()
{
}

void AddWapUI::init()
{
    setWindowTitle( tr("WAP") );
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setMargin( 0 );
    mainLayout->setSpacing( 0 );
#ifndef QTOPIA_PHONE
    QTabWidget* tabWidget = new QTabWidget( this );
    accountPage = new WapAccountPage( configFile );
    tabWidget->addTab( accountPage, tr("Account") );
    tabWidget->setTabIcon(0, QIcon(":icon/account") );

    gatewayPage = new GatewayPage( configFile );
    tabWidget->addTab( gatewayPage, tr("Gateway") );
    tabWidget->setTabIcon(1, QIcon(":icon/wap") );

    mmsPage = new MMSPage( configFile );
    tabWidget->addTab( mmsPage, tr("MMS") );
    tabWidget->setTabIcon(2, QIcon(":icon/mms") );

    browserPage = new BrowserPage( configFile );
    tabWidget->addTab( browserPage, tr("Misc") );

    mainLayout->addWidget( tabWidget );
#else
    stack = new QStackedWidget( this );

    QWidget* page = new QWidget();
    QVBoxLayout *vb = new QVBoxLayout(page);
    vb->setMargin( 2 );
    vb->setSpacing( 2 );
    options = new QListWidget( page );
    options->setSpacing( 1 );
    options->setAlternatingRowColors( true );
    options->setSelectionBehavior( QAbstractItemView::SelectRows );
    QListWidgetItem* item = new QListWidgetItem( QIcon(":icon/account"),
            tr("Account"), options, Account );
    item->setTextAlignment( Qt::AlignHCenter);
    item = new QListWidgetItem( QIcon(":icon/wap"),
            tr("WAP Gateway"), options,  WAPGateway );
    item->setTextAlignment( Qt::AlignHCenter);
    item = new QListWidgetItem( QIcon(":icon/mms"),
            tr("MMS"), options, MMS );
    item->setTextAlignment( Qt::AlignHCenter);
    item = new QListWidgetItem( tr("Misc"), options, Browsing );
    item->setTextAlignment( Qt::AlignHCenter);
    vb->addWidget( options );

    QHBoxLayout* hb = new QHBoxLayout();
    hint = new QLabel( page );
    hint->setWordWrap( true );
    hint->setMargin( 2 );
    hb->addWidget( hint );

    QSpacerItem* spacer = new QSpacerItem( 1, 60, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );
    hb->addItem( spacer );

    vb->addLayout( hb );

    connect( options, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateUserHint(QListWidgetItem*,QListWidgetItem*)));

    options->setCurrentRow( 0 );
    stack->addWidget( page );

    QScrollArea* scroll = 0;

    accountPage = new WapAccountPage( configFile );
    stack->addWidget( accountPage );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    gatewayPage = new GatewayPage( configFile );
    scroll->setWidget( gatewayPage );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    mmsPage = new MMSPage( configFile );
    scroll->setWidget( mmsPage );
    stack->addWidget( scroll );

    scroll = new QScrollArea();
    scroll->setWidgetResizable( true );
    scroll->setFocusPolicy( Qt::NoFocus );
    browserPage = new BrowserPage( configFile );
    scroll->setWidget( browserPage );
    stack->addWidget( scroll );

    stack->setCurrentIndex( 0 );

    mainLayout->addWidget( stack );
    connect(options, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(optionSelected(QListWidgetItem*)));
#endif
}

void AddWapUI::accept()
{
#ifdef QTOPIA_PHONE
    if ( stack->currentIndex() == 0 ) {

        QtopiaNetworkProperties props = accountPage->properties();
        writeConfig(props);
        props = gatewayPage->properties();
        writeConfig(props);
        props = mmsPage->properties();
        writeConfig(props);
        props = browserPage->properties();
        writeConfig(props);
        QDialog::accept();
    } else {
        stack->setCurrentIndex( 0 );
    }
#else
    QtopiaNetworkProperties props = accountPage->properties();
    writeConfig(props);
    props = gatewayPage->properties();
    writeConfig(props);
    props = mmsPage->properties();
    writeConfig(props);
    props = browserPage->properties();
    writeConfig(props);
    QDialog::accept();
#endif
}

void AddWapUI::writeConfig(const QtopiaNetworkProperties prop)
{
    QSettings cfg( configFile, QSettings::IniFormat );
    QMapIterator<QString,QVariant> i(prop);
    QString key;
    while (i.hasNext()) {
        i.next();
        QString key = i.key();
        cfg.setValue( key, i.value() );
    }
}

#ifdef QTOPIA_PHONE
void AddWapUI::optionSelected(QListWidgetItem* item)
{
    if (item) {
        switch( item->type() )
        {
            case Account:
                stack->setCurrentIndex( 1 );
                accountPage->setFocus();
                break;
            case MMS:
                stack->setCurrentIndex( 3 );
                mmsPage->setFocus();
                break;
            case Browsing:
                stack->setCurrentIndex( 4 );
                browserPage->setFocus();
                break;
            case WAPGateway:
                stack->setCurrentIndex( 2 );
                gatewayPage->setFocus();
                break;
            default:
                break;
        }
    }
}


void AddWapUI::updateUserHint( QListWidgetItem* newItem, QListWidgetItem* /*prev*/ )
{
    if (!newItem)
        return;

    QString text;
    switch( newItem->type() ) {
        case Account:
            text = tr("General account information.");
            break;
        case MMS:
            text = tr("MMS server and message details.");
            break;
        case Browsing:
            text = tr("General browser settings");
            break;
        case WAPGateway:
            text = tr("WAP server login details.");
            break;
        default:
            break;
    }
    hint->setText( text );
}


#endif

#include "addwapui.moc"

