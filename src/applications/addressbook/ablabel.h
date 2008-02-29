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
#ifndef ABLABEL_H
#define ABLABEL_H

#include <qtopia/pim/qcontact.h>
#include <qtopiaservices.h>
#ifdef QTOPIA_VOIP
#include <qpresence.h>
#endif
#include <QDLBrowserClient>

class QTextCharFormat;

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
    void linkHighlighted(const QString& link);

signals:
    void okPressed();
    void previous();
    void next();
    void externalLinkActivated();
    void backClicked();

protected:
    void keyPressEvent( QKeyEvent *e );

    void setSource(const QUrl & name);

    QTextDocument* createContactDocument(const QContact &contact, bool dialer);

    typedef enum {NoLink = 0, Dialer, Messaging, Email} LinkType;
    void addPhoneFragment( QTextCursor& outCurs, const QString& img, const QString& num, LinkType link, int phoneType = -1 );
    void addNameFragment( QTextCursor &curs, const QContact &contact);

    QTextCharFormat cfNormal;
    QTextCharFormat cfItalic;
    QTextCharFormat cfBold;
    QTextCharFormat cfBoldUnderline;
    QTextCharFormat cfSmall;
    QTextCharFormat cfSmallBold;
    QTextCharFormat cfAnchor;

    QTextBlockFormat bfNormal;
    QTextBlockFormat bfCenter;

    QTextTableFormat tfNoBorder;

    void addBusinessFragment( QTextCursor &outCurs, const QContact &contact, bool dialer );
    void addPersonalFragment( QTextCursor &outCurs, const QContact &contact, bool dialer );
    void addBusinessPhoneFragment( QTextCursor &outCurs, const QContact &contact, bool dialer );
    void addHomePhoneFragment( QTextCursor &outCurs, const QContact &contact, bool dialer );
    void addEmailFragment( QTextCursor &outCurs, const QContact &contact );
#ifdef QTOPIA_VOIP
    void addVoipFragment( QTextCursor &outCurs, const QContact &contact );
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

