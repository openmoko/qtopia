/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#ifndef ABLABEL_H
#define ABLABEL_H

#include <qtopia/pim/qcontact.h>
#include <qtopiaservices.h>
#ifdef QTOPIA_VOIP
#include <qpresence.h>
#endif
#include <QDLBrowserClient>

class AbLabel : public QDLBrowserClient
{
    Q_OBJECT

public:
    AbLabel( QWidget *parent, const char *name = 0 );
    ~AbLabel();

    //virtual void setSource( const QString& name );

    bool decodeHref(const QString& href, QtopiaServiceRequest* req, QString* pm) const;
    QString encodeHref() const;
    QContact entry() const;

public slots:
    void init( const QContact &entry );
    void linkClicked(const QString& link);
    void linkSelected(const QString& link);

signals:
    void okPressed();
    void previous();
    void next();
    void externalLinkActivated();
    void backClicked();

protected:
    void keyPressEvent( QKeyEvent *e );

    void setSource(const QUrl & name);

    QString contactToRichText( const QContact & contact, bool dialer );
    QString businessRichText( const QContact &contact, bool dialer );
    QString personalRichText( const QContact &contact, bool dialer );
    QString busPhoneRichText( const QContact &contact, bool dialer );
    QString homePhoneRichText( const QContact &contact, bool dialer );
    QString emailRichText( const QContact &contact );
#ifdef QTOPIA_VOIP
    QString voipIdRichText( const QContact &contact );
#endif

private slots:
#ifdef QTOPIA_VOIP
    void monitoredPresence( const QString& uri, QPresence::Status status );
#endif
private:
    QtopiaServiceRequest anchorService(const QString& name) const;
    QContact ent;

    QString mLink;

#ifdef QTOPIA_VOIP
    QPresence *presence;
    QPresence::Status monitoredVoipIdStatus;
    QString monitoredVoipId;
#endif
};

#endif // ABLABEL_H

