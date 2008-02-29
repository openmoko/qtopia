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

#include "proxiesconfig.h"

#include <QComboBox>
#include <QLineEdit>
#include <QFrame>
#include <QValidator>
#include <QSpinBox>
#include <QRadioButton>
#include <QLayout>

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include "ui_proxiesconfigbase.h"

class ProxiesPageBaseContainer : public QFrame, public Ui::ProxiesPageBase
{
    public:
        ProxiesPageBaseContainer(QWidget *parent = 0, Qt::WFlags fl = 0)
            :QFrame(parent, fl)
        {
            setFrameShape( QFrame::NoFrame );
            setupUi(this);
        }

    friend class ProxiesPage;
};

/*!
  \class ProxiesPage
  \brief The ProxiesPage class provides the user interface for the network proxy configuration.
  \mainclass
  \internal

  The ProxiesPage widget is exclusively used by the Qtopia network plug-ins. It 
  allows the user to edit the following proxy details:

  \list
    \o http proxy
    \o ftp proxy
  \endlist

  This is not a public class.
*/
ProxiesPage::ProxiesPage( const QtopiaNetworkProperties& cfg, QWidget* parent, Qt::WFlags flags ) :
    QWidget(parent, flags)
{
    QVBoxLayout *tl = new QVBoxLayout(this);
    d = new ProxiesPageBaseContainer( this );
    tl->addWidget(d);

    QtopiaApplication::setInputMethodHint(d->http_host,"url");
    QtopiaApplication::setInputMethodHint(d->ftp_host,"url");
    QtopiaApplication::setInputMethodHint(d->noproxies,"url");

    QSoftMenuBar::menuFor( this );
    QSoftMenuBar::setHelpEnabled( this , true );

    setObjectName("proxies");
    readConfig( cfg );
    connect(d->type,SIGNAL(activated(int)),this,SLOT(typeChanged(int)));
}

ProxiesPage::~ProxiesPage()
{
}

void ProxiesPage::typeChanged(int t)
{
    switch (t) {
     case 0: d->autopanel->hide(); d->setpanel->hide(); break;
     case 1: d->setpanel->hide(); d->autopanel->show(); break;
     case 2: d->autopanel->hide(); d->setpanel->show(); break;
    }
}

class ProxyValidator : public QValidator {
public:
    ProxyValidator( QWidget * parent ) :
        QValidator(parent)
    {
    }

    State validate( QString &s, int &pos ) const
    {
        int i;
        for (i=0; i<(int)s.length(); i++) {
            if ( s[i] == '/' || s[i] == ',' || s[i] == ';' || s[i] == '\n' || s[i] == '\r' )
                s[i] = ' ';
        }
        for (i=0; i<(int)s.length()-1; ) {
            if ( s[i] == ' ' && s[i+1] == ' ' ) {
                if (pos>i) pos--;
                s = s.left(i)+s.mid(i+2);
            } else
                i++;
        }
        return Acceptable;
    }
};

void ProxiesPage::readConfig(const QtopiaNetworkProperties& prop)
{
    int index = 0;
    if (prop.contains("Proxy/Type"))
        index = prop.value("Proxy/Type").toInt();
    d->type->setCurrentIndex(index);
    typeChanged(index);


    if (prop.contains("Proxy/AutoConfig"))
        d->autoconfig->addItem( prop.value("Proxy/AutoConfig").toString() );

    if (prop.contains("Proxy/HttpHost"))
        d->http_host->setText( prop.value("Proxy/HttpHost").toString() );

    if (prop.contains("Proxy/HttpPort"))
        d->http_port->setValue( prop.value("Proxy/HttpPort").toInt() );

    if (prop.contains("Proxy/FtpHost"))
        d->ftp_host->setText( prop.value("Proxy/FtpHost").toString() );

    if (prop.contains("Proxy/FtpPort"))
        d->ftp_port->setValue( prop.value("Proxy/FtpPort").toInt() );

    if (prop.contains("Proxy/NoProxies")) {
        d->noproxies->setValidator(new ProxyValidator(this));
        d->noproxies->setText(prop.value("Proxy/NoProxies").toString());
    }
}

QtopiaNetworkProperties ProxiesPage::properties( )
{
    QtopiaNetworkProperties prop;
    prop.insert("Proxy/Type",d->type->currentIndex());
    prop.insert("Proxy/AutoConfig", d->autoconfig->currentText());
    prop.insert("Proxy/HttpHost", d->http_host->text());
    prop.insert("Proxy/HttpPort", d->http_port->text());
    prop.insert("Proxy/FtpHost", d->ftp_host->text());
    prop.insert("Proxy/FtpPort", d->ftp_port->text());
    prop.insert("Proxy/NoProxies", d->noproxies->text());
    return prop;
}

