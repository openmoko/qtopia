/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


#define QTOPIA_INTERNAL_FILEOPERATIONS

#include "dialup.h"

#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include "../proxiespage.h"
#ifdef QTOPIA_PHONE
#include <qtopia/phone/phone.h>
#endif

#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qfile.h>
#include <qdir.h>
#include <qobjectlist.h>
#include <qtabwidget.h>
#include <qcstring.h>
#include <qlineedit.h>
#include <qtextstream.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <qlayout.h>

#include "../addscrollbars.cpp"

Dialup::Dialup( QWidget* parent ) :
    DialupBase(parent,0,TRUE)
{
    QPEApplication::setInputMethodHint(apn,"url");
    QPEApplication::setInputMethodHint(dns_1,"netmask");
    QPEApplication::setInputMethodHint(dns_2,"netmask");
    QPEApplication::setInputMethodHint(username,"email");
    QPEApplication::setInputMethodHint(atdial,QPEApplication::Text);
    QPEApplication::setInputMethodHint(phone,QPEApplication::PhoneNumber);

    QVBoxLayout *layout = new QVBoxLayout( proxies );
    proxiespage = new ProxiesPage( proxies );
    layout->addWidget( proxiespage );

    addScrollBars(tabs);
}

DialupImpl::DialupImpl() : ref(0)
{
    logfile = Global::tempDir()+ "qpe-pppd-log";
}

DialupImpl::~DialupImpl()
{
}

QRESULT DialupImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown || uuid == IID_Network || uuid == IID_Network2 )
        *iface = this;
    else
        return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( DialupImpl )
}

static bool setSecret( QFile &paps, const QString& user, const QString& server, const QString& secret, const QString& address)
{
    QString id = user + " " + server + " ";
    QString entry = id + Global::stringQuote(secret);
    if ( !address.isEmpty() )
        entry += " " + address;
    QStringList secrets;
    if ( paps.open(IO_ReadOnly) ) {
        {
            QTextStream s(&paps);
            while ( !s.eof() ) {
                QString se = s.readLine();
                if ( se == entry )
                    return TRUE; // already in there
                if ( se.left(id.length()) != id )
                    secrets.append(se);
            }
        }
        paps.close();
    }
    if ( !secret.isNull() ) // null for deleting secrets
        secrets.append(entry);
    if ( paps.open(IO_WriteOnly) ) {
        fchmod(paps.handle(),0600);
        {
            QTextStream s(&paps);
            s << secrets.join("\n") << "\n";
        }
        paps.close();
        return TRUE;
    }
    return FALSE;
}

static bool setPapSecret(const QString& user, const QString& server, const QString& secret, const QString& address)
{
    bool retval = TRUE;
    // joespc    server    "joe's secret" joespc.my.net
    QFile paps("/etc/ppp/pap-secrets" );
    if ( !setSecret( paps, user, server, secret, address ) )
        retval = FALSE;
    QFile chaps( "/etc/ppp/chap-secrets" );
    if ( !setSecret( chaps, user, server, secret, address ) )
        retval = FALSE;
    return retval;
}

static void selectComboItem(QComboBox* combo, const QString& item)
{
    for (int i=0; i<combo->count(); ++i) {
        if ( combo->text(i) == item ) {
            combo->setCurrentItem(i);
            return;
        }
    }
    combo->insertItem(item);
    combo->setCurrentItem(combo->count()-1);
}

static void hideWidgets(QObjectList* list, bool justreadonly)
{
    if ( list ) {
        QObjectListIt it(*list);
        QObject *obj;
        while ((obj = *it)) {
            if ( obj->isWidgetType() ) {
                QWidget* w = (QWidget*)obj;
                if ( justreadonly ) {
                    if ( w->inherits("QLineEdit") ) {
                        ((QLineEdit*)w)->setReadOnly(TRUE);
                    } else {
                        w->setEnabled(FALSE);
                    }
                } else {
                    w->hide();
                }
            }
            ++it;
        }
        delete list;
    }
}

// get the full dial string, including special initialization commands.
QString DialupImpl::dialString( Config& cfg ) const
{
    cfg.setGroup("Properties");
    QString dial = cfg.readEntry("atdial","ATDT");
    QString apn = cfg.readEntry("apn");
    bool gprs = cfg.readBoolEntry("GPRS",FALSE);
    if ( gprs ) {
#ifdef QT_QWS_TIP2
        // Override default dialstring for device.
        dial = "AT\%NRG=0,0,0 OK "
            "AT\%CGCLASS? OK "
            "AT\%CGAATT=0 OK "
            "AT\%CGPPP=0 OK "
            "AT+CGDCONT=1,\"IP\",\"" + apn + "\"" + " OK "
            "AT+CGATT=1 OK "
            " " + dial + "*99***1#";
#else
        //Replace this string with dial string suitable for modem
        dial = "AT+CGDCONT=1,\"IP\",\"" + apn + "\"" + " OK "
            "AT+CGATT=1 "
            "OK " + dial + "*99***1#";
#endif
    } else {
        dial += cfg.readEntry("phone");
    }
    return dial;
}

static struct {
    const char* id;
    /*const char* label;*/
} oma_addresstype[] = {
    {"IPV4"},//, QT_TRANSLATE_NOOP("Dialup","IPV4")},
    {"IPV6"},//, QT_TRANSLATE_NOOP("Dialup","IPV6")},
    {"E164"},//, QT_TRANSLATE_NOOP("Dialup","E164")},
    {"ALPHA"},//, QT_TRANSLATE_NOOP("Dialup","ALPHA")},
    {0}//,0}
};

static struct {
    const char* id;
    const char* label;
} oma_logintype[] = {
    {"HTTP-BASIC", QT_TRANSLATE_NOOP("Dialup","Basic")},
    {"HTTP-DIGEST", QT_TRANSLATE_NOOP("Dialup","Digest")},
    {0,0}
};

static struct {
    const char* id;
    const char* label;
} oma_servicetype[] = {
    {"CL-WSP", QT_TRANSLATE_NOOP("Dialup","WAP CL")},
    {"CO-WSP", QT_TRANSLATE_NOOP("Dialup","WAP")},
    {"CL-SEC-WSP", QT_TRANSLATE_NOOP("Dialup","WAP secure CL")},
    {"CO-SEC-WSP", QT_TRANSLATE_NOOP("Dialup","WAP secure")},
    {"CO-SEC-WTA", QT_TRANSLATE_NOOP("Dialup","WAP WTA secure")},
    {"CL-SEC-WTA", QT_TRANSLATE_NOOP("Dialup","WAP WTA secure CL")},
#ifndef QTOPIA_PHONE
    {"OTA-HTTP-TO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP TO-TCP")},
    {"OTA-HTTP-PO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP PO-TCP")},
    {"OTA-HTTP-TLS-TO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP secure TO-TCP")},
    {"OTA-HTTP-TLS-PO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP secure PO-TCP")},
#else
    {"OTA-HTTP-TO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP-TO")},
    {"OTA-HTTP-PO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP-PO")},
    {"OTA-HTTP-TLS-TO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP-TLS-TO")},
    {"OTA-HTTP-TLS-PO", QT_TRANSLATE_NOOP("Dialup","OTA-HTTP-TLS-PO")},
#endif
    {0,0}
};

bool DialupImpl::doProperties( QWidget *parent, Config& cfg )
{
    Dialup dialog(parent);
    cfg.setGroup("Info");
    QString acname = cfg.readEntry("Name");
    QString acid = cfg.readEntry("Id");
    if ( acid.isNull() ) {
        // Uniqueish ID
        acid = acname;
        acid.replace(QRegExp("[^A-Za-z]"),"");
        acid += QString::number(time(0)^(getpid()<<3));
        cfg.writeEntry("Id",acid);
    }
    dialog.acname->setText(acname);
    dialog.setCaption(acname);
    cfg.setGroup("Properties");
    bool ro = cfg.readNumEntry("ReadOnly",0);
    QString prev_user = cfg.readEntry("username");
    dialog.username->setText(prev_user);
    dialog.password->setText(cfg.readEntry("password"));
    bool gprs = cfg.readBoolEntry("GPRS",FALSE);
    if ( gprs ) {
        dialog.phone_label->hide();
        dialog.phone->hide();
    } else {
        dialog.apn_label->hide();
        dialog.apn->hide();
    }
    dialog.phone->setText(cfg.readEntry("phone"));
    dialog.apn->setText(cfg.readEntry("apn"));

    int dialmode = cfg.readNumEntry("dialmode", 0);
    int buttonId = 0;
    switch (dialmode)
    {
        case 0:
            buttonId = dialog.dialmode->id(dialog.dial_manual);
            break;
        case 1:
            buttonId = dialog.dialmode->id(dialog.dial_idle);
            break;
        case 2:
            buttonId = dialog.dialmode->id(dialog.dial_demand);
            break;
    }
    dialog.dialmode->setButton(buttonId);
    
    dialog.idletime->setValue(cfg.readNumEntry("idletime",120));
    QString s = cfg.readEntry("atdial","ATDT");
    if ( !s.isEmpty() )
        selectComboItem(dialog.atdial, s);
    dialog.defaultroute->setChecked(cfg.readBoolEntry("defaultroute",TRUE));
    dialog.usepeerdns->setChecked(cfg.readBoolEntry("usepeerdns",TRUE));
    dialog.gateway->setText(cfg.readEntry("gateway"));
    dialog.dns_1->setText(cfg.readEntry("dns1"));
    dialog.dns_2->setText(cfg.readEntry("dns2"));
    dialog.gateway_box->hide(); // UNUSED YET
    dialog.connectdelay->setValue(cfg.readNumEntry("connectdelay",5000)/1000);
    selectComboItem(dialog.speed, cfg.readEntry("speed","115200"));
    dialog.speed->setCurrentItem(dialog.speed->count()-1);
    dialog.crtscts->setChecked(cfg.readBoolEntry("crtscts",TRUE));

    QObject* stack = dialog.tabs->queryList("QWidgetStack",0,FALSE,FALSE)->first();

    if ( gprs ) {
        QString addresstype = cfg.readEntry("wapProxyAddressType",oma_addresstype[0].id);
        for (int i=0; oma_addresstype[i].id; i++) {
            //dialog.wapProxyAddressType->insertItem(tr(oma_addresstype[i].label));
            dialog.wapProxyAddressType->insertItem(Dialup::tr(oma_addresstype[i].id));
            if ( oma_addresstype[i].id == addresstype )
                dialog.wapProxyAddressType->setCurrentItem(dialog.wapProxyAddressType->count()-1);
        }
        QString logintype = cfg.readEntry("wapProxyLoginType",oma_logintype[0].id);
        for (int i=0; oma_logintype[i].id; i++) {
            dialog.wapProxyLoginType->insertItem(Dialup::tr(oma_logintype[i].label));
            if ( oma_logintype[i].id == logintype )
                dialog.wapProxyLoginType->setCurrentItem(dialog.wapProxyLoginType->count()-1);
        }
        dialog.wapProxyAddress->setText(cfg.readEntry("wapProxyAddress"));
        dialog.wapProxyAuthName->setText(cfg.readEntry("wapProxyAuthName"));
        dialog.wapProxyAuthPassword->setText(cfg.readEntry("wapProxyAuthPassword"));
        dialog.wapProxyPort->setValue(cfg.readNumEntry("wapProxyPort"));
        QString servicetype = cfg.readEntry("wapProxyService",oma_servicetype[0].id);
        for (int i=0; oma_servicetype[i].id; i++) {
            dialog.wapProxyService->insertItem(Dialup::tr(oma_servicetype[i].label));
            if ( oma_servicetype[i].id == servicetype )
                dialog.wapProxyService->setCurrentItem(dialog.wapProxyService->count()-1);
        }
        dialog.wapDefaultHomePage->setText(cfg.readEntry("wapDefaultHomePage"));
        dialog.mmsURL->setText(cfg.readEntry("mmsURL"));
    }

    for (int roandh=0; roandh<2; roandh++) {
        QStringList hidelist = QStringList::split(" ",
            cfg.readEntry(roandh ? "ReadOnly" : "Hide"));

        if (!gprs)
            hidelist.append("wap"); // Only ever for GPRS

        for (QStringList::ConstIterator it=hidelist.begin(); it!=hidelist.end(); ++it) {
            QObjectList *list = roandh ? 0 : stack->queryList(0,*it,FALSE,FALSE);
            if ( list && list->count() ) {
                QObject *t = list->first();
                if ( t->isWidgetType() )
                    dialog.tabs->removePage((QWidget*)t);
            } else {
                hideWidgets(dialog.queryList(0,*it,FALSE,TRUE),roandh);
                if ( !roandh )
                    hideWidgets(dialog.queryList(0,*it+"_.*",TRUE,TRUE),roandh);
            }
        }
    }

    cfg.setGroup("Proxy");
    dialog.proxiespage->readConfig( cfg );
    dialog.showMaximized();
    if ( ro || dialog.exec() ) {
        cfg.setGroup("Info");
        cfg.writeEntry("Name", dialog.acname->text());
        cfg.setGroup("Properties");
        QString prev_user = cfg.readEntry("username");
        QString user;
        cfg.writeEntry("username", user=dialog.username->text());
        cfg.writeEntry("password", dialog.password->text());
        cfg.writeEntry("phone", dialog.phone->text());
        cfg.writeEntry("apn", dialog.apn->text());
        int dialmode = dialog.dialmode->id(dialog.dialmode->selected());
        cfg.writeEntry("dialmode", dialog.dialmode->id(dialog.dialmode->selected()));
        cfg.writeEntry("Auto", dialmode==2);
        cfg.writeEntry("idletime", dialog.idletime->value());
        cfg.writeEntry("atdial", dialog.atdial->currentText());
        cfg.writeEntry("usepeerdns", dialog.usepeerdns->isChecked());
        cfg.writeEntry("gateway",dialog.gateway->text());
        cfg.writeEntry("dns1",dialog.dns_1->text());
        cfg.writeEntry("dns2",dialog.dns_2->text());
        cfg.writeEntry("defaultroute", dialog.defaultroute->isChecked());
        cfg.writeEntry("connectdelay", dialog.connectdelay->value()*1000);
        cfg.writeEntry("speed", dialog.speed->currentText());
        cfg.writeEntry("crtscts", dialog.crtscts->isChecked());
        if ( gprs ) {
            cfg.writeEntry("wapProxyAddress", dialog.wapProxyAddress->text());
            cfg.writeEntry("wapProxyAddressType", dialog.wapProxyAddressType->currentText());
            cfg.writeEntry("wapProxyLoginType", dialog.wapProxyLoginType->currentText());
            cfg.writeEntry("wapProxyAuthName", dialog.wapProxyAuthName->text());
            cfg.writeEntry("wapProxyAuthPassword", dialog.wapProxyAuthPassword->text());
            cfg.writeEntry("wapProxyPort", dialog.wapProxyPort->value());
            cfg.writeEntry("wapProxyService", dialog.wapProxyService->currentText());
            cfg.writeEntry("wapDefaultHomePage", dialog.wapDefaultHomePage->text());
            cfg.writeEntry("mmsURL", dialog.mmsURL->text());
        }
        
        cfg.setGroup("Proxy");
        dialog.proxiespage->writeConfig( cfg );
        if ( prev_user != user )
            setPapSecret(prev_user,acid,QString::null,"");
        return writeSystemFiles(&dialog,cfg);
    }
    return FALSE;
}

bool DialupImpl::writeSystemFiles( QWidget *parent, Config& cfg )
{
    cfg.setGroup("Info");
    QString acid = cfg.readEntry("Id");
    cfg.setGroup("Properties");
    mkdir("/etc/ppp/peers",0700);
    QFile options("/etc/ppp/peers/"+acid);
    if ( options.open(IO_WriteOnly) ) {
        QTextStream opt(&options);
        opt << cfg.readEntry("speed") << "\n";
        QString dial = dialString(cfg);
        int dialmode = cfg.readNumEntry("dialmode",0);
        bool internal = dev(cfg).isEmpty();
        if ( internal ) {
            qDebug("writeSystemFiles()-internal mode");
                // Internal mode only available in phone edition!
#ifdef QTOPIA_PHONE
            if (dialmode == 2) {
                opt << "connect '" // No tr
                    << QPEApplication::qpeDir() << "/bin/qtopia-dial-internal'\n";
                opt << "disconnect '" // No tr
                    << QPEApplication::qpeDir() << "/bin/qtopia-stop-internal'\n";
            }
#endif
        }
        else {
            qDebug("writeSystemFiles()-external mode");
              // External mode available in both editions!
            if ( ! cfg.readBoolEntry("GPRS",FALSE) ){
                opt << "connect '/usr/sbin/chat " // No tr
                    "-s -v "
                    "ABORT \"NO CARRIER\" " // No tr
                    "ABORT \"NO DIALTONE\" " // No tr
                    "ABORT \"BUSY\" " // No tr
                << "\"\" ATZ OK " + dial + " CONNECT'\n";
            } 
            else {
                QString path = Global::applicationFileName("Network", "chat");
                QDir dir(path);
                dir.mkdir(path);
                QString conName = path+"/connect-"+acid;
                QString disconName = path+"/disconnect-"+acid;
                QFile connectF(conName);
                QFile disconnectF(disconName);
                             
                opt << "connect '/usr/sbin/chat -s -v -f " + conName << "'" << endl;// No tr
                opt << "disconnect '/usr/sbin/chat -s -v -- -f " + disconName << "'" << endl;// No tr
                
                QString connectChat  = 
                    "ABORT \"NO CARRIER\"\n" // No tr
                    "ABORT \"NO DIALTONE\"\n" // No tr
                    "ABORT \"BUSY\"\n" // No tr
                    "\"\" " + dial + "\n";
                connectChat = connectChat.replace(QRegExp("OK"), "\nOK");
                QString disconnectChat = "\"\"";
                
                if ( connectF.open( IO_Truncate | IO_WriteOnly )) {
                    QTextStream st(&connectF);
                    st << connectChat;
                    connectF.close();
                } else {
                    return FALSE;
                }
                
                if ( disconnectF.open( IO_Truncate | IO_WriteOnly )) {
                    QTextStream st(&disconnectF);
                    st << disconnectChat;
                    disconnectF.close();
                } else {
                     return FALSE;
                }
            }
        }
    
        // GPRS modems require the the following pppd options
        if ( cfg.readBoolEntry("GPRS",FALSE) ){
            // Most GRPS modems do not respond to LCP echo's
            opt << "lcp-echo-failure 0\n"   // No tr
                 "lcp-echo-interval 0\n"; // No tr
            // Compression does not make sense on a GPRS link 
            opt << "novj\n" // No tr 
                "nobsdcomp\n" // No tr  
                "novjccomp\n" // No tr 
                "nopcomp\n" // No tr 
                "noaccomp\n"; // No tr 
        }
        if ( cfg.readBoolEntry("crtscts") )
            opt << "crtscts\n";
        else
            opt << "nocrtscts\n";
        // Accept peers idea of our local address
        opt << "ipcp-accept-local\n";
        opt << "noipdefault\n";
        // Use modem control lines, otherwise change the following "modem" to "local"
        opt << "modem\n"; // No tr
        
        QString user = cfg.readEntry("username");
        if ( !user.isEmpty() )
            opt << "user " << Global::stringQuote(user) << "\n"; // No tr
        if ( dialmode == 2 )
            opt << "demand\n"; // No tr
        int idlet = cfg.readNumEntry("idletime",120);
        if ( dialmode > 0 && idlet )
            opt << "idle " << idlet << "\n"; // No tr
        if ( cfg.readBoolEntry("usepeerdns",TRUE) )
            opt << "usepeerdns\n";
        if ( cfg.readBoolEntry("defaultroute",TRUE) )
            opt << "defaultroute\n";
        int cdel = cfg.readNumEntry("connectdelay",0);
        if ( cdel )
            opt << "connect-delay " << cdel << "\n";
        if ( !acid.isEmpty() )
            opt << "remotename " << acid << "\n";
        QString pw = cfg.readEntry("password");
        bool pwsetok = TRUE;
        if ( !pw.isEmpty() )
            pwsetok = setPapSecret(user,acid,pw,"");
        else
            pwsetok = setPapSecret(user,acid,QString::null,"");
        if ( !pwsetok ) {
            QMessageBox::critical(parent,Dialup::tr("I/O Error"),
                Dialup::tr("<P>Cannot write to PPP password files."));
            return FALSE;
        }
        return TRUE;
    } else {
        QMessageBox::critical(parent,Dialup::tr("I/O Error"),
            Dialup::tr("<P>Cannot write to %1.").arg(options.name()));
        return FALSE;
    }
}

QString DialupImpl::dev( Config& cfg ) const
{
    cfg.setGroup("Properties");
    QString d = cfg.readEntry("SerialDevice");
    if ( d.isEmpty() ) {
        QString ct = cardType(cfg);
        if ( !ct.isEmpty() ) {
            FILE* f = fopen("/var/run/stab", "r");
            if (!f) f = fopen("/var/state/pcmcia/stab", "r");
            if (!f) f = fopen("/var/lib/pcmcia/stab", "r");
            if ( f ) {
                char line[1024];
                char devtype[80];
                char devname[80];
                while ( fgets( line, 1024, f ) ) {
                    // 0       serial  serial_cs       0       ttyS0   4       64
                    if ( sscanf(line,"%*d %s %*s %*s %s", devtype, devname )==2 )
                    {
                        if ( ct == devtype ) {
                            fclose(f);
                            return QString("/dev/")+devname;
                        }
                    }
                }
                fclose(f);
            }
        }
    }
    return d;
}

bool DialupImpl::create( Config& )
{
    // nothing special
    return TRUE;
}

bool DialupImpl::remove( Config&cfg )
{
    stop(cfg);
    cfg.setGroup("Info");
    QString peer = cfg.readEntry( "Id" );
    if ( !peer.isEmpty() ) {
        peer = "/etc/ppp/peers/" + peer;
        unlink( peer.latin1() );
    }
    return TRUE;
}


class PppMon : public QObject {
    Q_OBJECT

    enum { Initialize, Connect, Authenticate, Timing, Disappearing } state;
    int age;
    int start;
    bool demand;
    QString logfile;
    int tid;

public:
    PppMon(bool demanddialing) :
        demand(demanddialing)
    {
        age = 0;
        start = 0;
        state = Initialize;
        logfile= Global::tempDir()+ "qpe-pppd-log";
        progress(demand ? tr("Waiting for activity") : tr("Initializing modem"),1);
        tid = startTimer(demand ? 8000 /* progress less interesting */ : 2000);
    }
    ~PppMon()
    {
    }

protected:
    void timerEvent(QTimerEvent*)
    {
        if ( state == Disappearing ) {
            killTimers();
            delete this;
            return;
        }
        QFile logf(logfile);
        if ( logf.open(IO_ReadOnly) ) {
            QString log = logf.readAll();
            log.replace(QRegExp("\r"),"");
            if (start > (int)log.length())
                start = 0;
            switch (state) {
                case Initialize:
                    if ( log.find(QRegExp("\nFailed"), start)>=start ) { // No tr
                        progress(tr("Connection failed"),0);
                        state = Disappearing;
                    } else if ( demand ) {
                        int ns;
                        if ( (ns=log.find(QRegExp("Starting link\n"), start))>=start ) { // No tr
                            progress(tr("Connecting"),5);
                            state = Connect;
                            start = ns+1;
                            killTimer(tid);
                            tid = startTimer(2000); // Better response during startup
                        }
                    } else {
                        int ns;
                        if ( (ns=log.find(QRegExp("\nOK\n -- got it"), start))>=start  
                         || (ns=log.find(QRegExp("Using interface ppp"), start))>=start){
                            progress(tr("Connecting"),1);
                            state = Connect;
                            start = ns+1;
                        }
                    }
                    break;
                case Connect:
                    if ( log.find(QRegExp("\nCONNECT[^\n]*\n -- got it"), start)>=start
                             || log.find(QRegExp("\nConnect: ppp\\d <--> "), start) >= start) {
                        progress(tr("Authenticating"),1); // No tr
                        state = Authenticate;
                    } else if ( log.find(QRegExp("\nNO DIALTONE"), start)>=start ) {
                        progress(tr("No dialtone"),0);
                        state = Disappearing;
                    } else if ( log.find(QRegExp("\nNO CARRIER"), start)>=start ) {
                        progress(tr("No carrier"),0);
                        state = Disappearing;
                    } else if ( log.find(QRegExp("\nBUSY"), start)>=start ) {
                        progress(tr("Number is busy"),0);
                        state = Disappearing;
                    } else if ( log.find(QRegExp("\nalarm"), start)>=start ) { // No tr
                        progress(tr("Timed out"),0);
                        state = Disappearing;
                    } else if ( log.find(QRegExp("\nFailed"), start)>=start ) { // No tr
                        progress(tr("Connection failed"),0);
                        state = Disappearing;
                    }
                    break;
                case Authenticate:
                    if ( log.find(QRegExp("\nPAP authentication failed"), start)>=start ) { // No tr
                        progress(tr("Authentication Failed"),0);
                        state = Disappearing;
                    } else if ( log.find(QRegExp("\nCHAP authentication failed"), start)>=start ) { // No tr
                        progress(tr("Authentication Failed"),0);
                        state = Disappearing;
                    } else if ( log.find(QRegExp("\nipcp: up"), start)>=start
                            || log.find(QRegExp("\nPAP authentication succeeded"), start) >= start
                            || log.find(QRegExp("\nCHAP authentication succeeded"), start) >= start) { // No tr
                        progress(tr("Connected"),5);
                        if ( demand ) {
                            state = Timing;
                            killTimer(tid);
                            tid = startTimer(8000); // Back to old update speed
                        } else {
                            state = Disappearing;
                        }
                    }
                    break;
                case Timing:
                    if ( log.find(QRegExp("\nConnection terminated"), start)>=start ) { // No tr
                        progress(tr("Waiting for activity"),1);
                        state = Initialize;
                    }
                        break;
                case Disappearing:
                    if ( demand ) {
                        killTimer(tid);
                        tid = startTimer(8000); // Back to old update speed
                    }
                    break;
            }
            if ( !demand && age++ > 120 ) // fail-safe self destruction
                state = Disappearing;
        } else
            state = Disappearing;
    }

    void progress(const QString& p, int flag)
    {
#ifndef QT_NO_COP
        QCopEnvelope e("QPE/Network", "progress(QString,int)");
        if ( demand ) flag |= 2;
        e << p << flag;
#endif
    }
};

bool DialupImpl::start( Config& cfg )
{
    return start(cfg,QString::null);
}

bool DialupImpl::start( Config& cfg, const QString& password )
{
    // start pppd
    cfg.setGroup("Properties");
    QString cmd = cfg.readEntry("script");
    QStringList args;
    bool internal = dev(cfg).isEmpty();
    if ( cmd.isEmpty() ) {
        if ( !cfg.readBoolEntry("usepeerdns",TRUE) ) {
            bool any=FALSE;
            for (int i=1; i<5 && !any; i++)
                any = !cfg.readEntry("dns"+QString::number(i)).isEmpty();
            if ( any ) {
                QFile rc("/etc/ppp/resolv.conf");
                if ( rc.open(IO_WriteOnly) ) {
                    QTextStream stream(&rc);
                    for (int i=1; i<5; i++) {
                        QString dns = cfg.readEntry("dns"+QString::number(i));
                        if ( !dns.isEmpty() )
                            stream << "nameserver " << dns << "\n";
                    }
                } else {
                    qWarning("Cannot write to /etc/ppp/resolv.conf file.");
                }
            }
        }
        args.append("/usr/sbin/pppd");   // No tr
        if ( internal ) {
            // Internal mode only available in phone edition!
#ifdef QTOPIA_PHONE
            args.append("nodetach");    // No tr
            args.append("notty");    // No tr
            QString d = dev(cfg);
            if ( !d.isEmpty() ) {
                qDebug("Dialup network is ignoring device %s",d.latin1());
            }
#endif
        }
        else {
            // External mode available in both editions!
#ifndef QTOPIA_PHONE
            QString d = dev(cfg);
            if ( d.isEmpty() ) {
                qWarning("No device for card type %s",cardType(cfg).latin1());
                return FALSE;
            }
            args.append(d);
#else
            args.append(dev(cfg));   // No tr
#endif
            args.append("updetach");    // No tr
        }

        args.append("debug");    // No tr
        args.append("call");    // No tr
        cfg.setGroup("Info");
        args.append(cfg.readEntry("Id"));   // No tr
    } else {
        args = QStringList::split(" ", cmd);
        if ( internal ) args.append(dev(cfg));
    }
    if ( !password.isNull() ) {
        args.append("password");    // No tr
        args.append(password);
    }
    args.append("logfile");    // No tr
    args.append(logfile);
    QFile::remove(logfile);

    // Construct the full dial control string.
    QString dial = dialString(cfg);

    if ( internal ) {
        // Internal mode only available in phone edition!
#ifdef QTOPIA_PHONE
        // Split the dial control string into setup commands and phone number.
        QStringList init = QStringList::split(" ", dial);
        QStringList::Iterator iter;
        QString number;
        for ( iter = init.begin(); iter != init.end(); ) {
            if ( *iter == "OK" ) {
                // The phone library will take care of looking for "OK".
                iter = init.remove(iter);
            } else if ( (*iter).startsWith("ATD") ) {
                // The dial command, starting with ATD, ATDT, or ATDP.
                number = (*iter).mid(3);
                if ( number.startsWith("T") || number.startsWith("P") ) {
                    number = number.mid(1);
                }
                iter = init.remove(iter);
            } else {
                ++iter;
            }
        }
        // Create a data call.
        PhoneLine line;
        bool demand = cfg.readNumEntry("dialmode",0) == 2;

        qWarning("starting dialup number: %s\n, init:%s\n, args:%s\n demand:%d", number.latin1(),
                         init.join(":").latin1(), args.join(":").latin1(), demand );

        line.startData( number, init, args, demand);
#endif
    }
    else {
        // External mode
        // Quote the command-line before passing it to "system".
        cmd = args[0];
        for ( uint posn = 1; posn < args.count(); ++posn ) {
            cmd += " " + Global::shellQuote(args[posn]);
        }
        cmd += " &";
        system("cardctl resume");
        qWarning("dialup command: %s", cmd.latin1());
        system(cmd.latin1());
    }

    cfg.setGroup("Properties");
    int dialmode = cfg.readNumEntry("dialmode",0);
    (void)new PppMon(dialmode==2);
    return TRUE;
}

bool DialupImpl::stop( Config& cfg )
{
    bool internal = dev(cfg).isEmpty();
    if ( internal ) {
        // Internal mode, phone edition only.
#ifdef QTOPIA_PHONE
        Q_CONST_UNUSED(cfg);
        PhoneLine line;
        line.stopData();
        return TRUE;
#endif
    }
    else {
        // kill pppd, external mode
        QString d = device(cfg);
        if ( !QFile::exists("/var/run/"+d+".pid") )
            d += "0";
        QFile pid("/var/run/"+d+".pid");
        if ( pid.open(IO_ReadOnly) ) {
            QString s(pid.readAll());
            int p = s.toInt();
            if ( p ) {
                kill(p,SIGTERM); // SIGHUP not enough for demand dialing pppd
                system("cardctl suspend");
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool DialupImpl::isAvailable( Config& cfg ) const
{
#ifdef QTOPIA_PHONE
    Q_CONST_UNUSED(cfg);
    return TRUE;
#else
    QString d = dev(cfg);
    if ( d.isEmpty() )
        return FALSE;
    return QFile::exists(d);
#endif
}

bool DialupImpl::needPassword( Config& cfg ) const
{
    return cfg.readEntry("password").isEmpty();
}

bool DialupImpl::setProperties( Config& cfg, const NetworkInterfaceProperties& settings )
{
    // XXX relationship between these proxies and WWW proxies???
    cfg.writeEntry("wapProxyAddress", settings["proxyAddress"].toString());
    cfg.writeEntry("wapProxyAddressType", settings["proxyAddressType"].toString());
    cfg.writeEntry("wapProxyLoginType", settings["proxyLoginType"].toString());
    cfg.writeEntry("wapProxyAuthName", settings["proxyAuthName"].toString());
    cfg.writeEntry("wapProxyAuthPassword", settings["proxyAuthPassword"].toString());
    cfg.writeEntry("wapProxyPort", settings["proxyPort"].toString());
    cfg.writeEntry("wapProxyService", settings["proxyService"].toString());
    cfg.writeEntry("wapDefaultHomePage", settings["homePage"].toString());
    cfg.writeEntry("mmsURL", settings["mmsUrl"].toString());

    QString acid = cfg.readEntry("Id");

    // Remove old password
    QString prev_user = cfg.readEntry("username");
    QString user = settings["authName"].toString();
    if ( prev_user != user ) {
        cfg.writeEntry("username",user);
        setPapSecret(prev_user,acid,QString::null,"");
    }

    cfg.writeEntry("apn", settings["accessPoint"].toString());
    cfg.writeEntry("password", settings["authPassword"].toString());

    return writeSystemFiles(0,cfg);
}

#include "dialup.moc"
