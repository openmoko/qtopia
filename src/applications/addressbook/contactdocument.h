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
#ifndef CONTACTDOCUMENT_H
#define CONTACTDOCUMENT_H

#include <QObject>
#ifdef QTOPIA_VOIP
#include "qpresence.h"
#endif
#include <qtopiaservices.h>
#include <QTextCharFormat>
#include <QContact>
#include <QContactModel>
#include "qtopiaapplication.h"
#include <QHash>

class QTextDocument;
class QTextCursor;
class QWidget;
class ContactAnchorData;
#if defined(QTOPIA_TELEPHONY)
class QContent;
#endif


class ContactDocument : public QObject
{
    Q_OBJECT

public:
    ContactDocument(QObject *parent);
    virtual ~ContactDocument();

    typedef enum {Details} ContactDocumentType;
    typedef enum {None, DialLink, EmailLink, QdlLink} ContactAnchorType;

    void init(QWidget *widget, QContactModel* model, const QContact& contact, ContactDocumentType docType);

    QTextDocument* textDocument() const { return mDocument; }
    QContact contact() const { return mContact; }

    ContactAnchorType getAnchorType(const QString& href);
    QString getAnchorTarget(const QString &href);

signals:
    void externalLinkActivated();

protected:
    QContact mContact;
    QTextDocument *mDocument;
    bool bDialer;
    bool voipDialer;
    bool mRtl;

    // member formats
    QTextCharFormat cfNormal;
    QTextCharFormat cfItalic;
    QTextCharFormat cfBold;
    QTextCharFormat cfBoldUnderline;
    QTextCharFormat cfSmall;
    QTextCharFormat cfSmallBold;

    QTextBlockFormat bfNormal;
    QTextBlockFormat bfCenter;

    QMap<QString,ContactAnchorData *> mFields;

    typedef enum {NoLink = 0, Dialer, Email} LinkType;

    int mIconHeight;

    // Document helpers
    void createContactDetailsDocument();

    // Fragment helpers
    void addPhoneFragment( QTextCursor& outCurs, const QString& img, const QString& num, LinkType link, QContactModel::Field type );

    QString nameFragment();
    void addBusinessFragment( QTextCursor &outCurs );
    void addPersonalFragment( QTextCursor &outCurs );
    void addBusinessPhoneFragment( QTextCursor &outCurs );
    void addHomePhoneFragment( QTextCursor &outCurs );
    void addEmailFragment( QTextCursor &outCurs );
    void addVoipFragment( QTextCursor& outCurs, const QString& img, const QString& uri, LinkType link, QContactModel::Field type );

    void addCachedPixmap(const QString& url, const QString& path);

    // QTextCursor helpers
    void addTextBreak( QTextCursor &curs);
    void addTextLine( QTextCursor& curs, const QString& text, const QTextCharFormat& cf,
                      const QTextBlockFormat &bf, const QTextCharFormat& bcf);
    void addImageAndTextLine ( QTextCursor& curs, const QTextImageFormat& imf,
                          const QString& text, const QTextCharFormat& cf, const QTextBlockFormat& bf,
                          const QTextCharFormat& bcf);
    void addTextNameValue( QTextCursor& curs, const QString& name,
                      const QTextCharFormat &ncf, const QString& value, const QTextCharFormat &vcf,
                      const QTextBlockFormat& bf, const QTextCharFormat& bcf);

#ifdef QTOPIA_VOIP
    QPresence *mPresence;
    QPixmap *mOnlinePixmap, *mOfflinePixmap;
    QPixmap *getPresencePixmap(bool online);
    QStringList mMonitoredURIs;
#endif

    QHash<QString, QPixmap> mCachedPixmaps;

    QtopiaServiceRequest anchorService(ContactAnchorData *cfd) const;

    QContactModel *mModel;

    private slots:
#ifdef QTOPIA_VOIP
    void monitoredPresence( const QString& uri, QPresence::Status status );
#endif

};

#endif // CONTACTDOCUMENT_H

