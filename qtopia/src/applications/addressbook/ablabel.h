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
#ifndef ABLABEL_H
#define ABLABEL_H

#include <qtopia/pim/contact.h>
#include <qtopia/services.h>
#include <qtextbrowser.h>

#ifdef QTOPIA_DATA_LINKING
class QDLClient;
#endif

// Keep this in sync with the copy in addressbook.h.
#ifndef	QTOPIA_PHONE_HEADER_HACK
#define	QTOPIA_PHONE_HEADER_HACK 1
#ifdef QTOPIA_PHONE
#include <qtopia/phone/phone.h>
#include <qtopia/phone/phonebook.h>
#else
/*
HACK - not including the above headers yields errors for
phonebookChanged() because it takes PhoneLine and PhoneBookEntry 
objects as arguments, but it's a slot so I can't ifdef it out. define these 
dummy objects on non phone so the metaobject references on non QTOPIA_PHONE 
still work.
*/
#ifndef PHONELINE_HACK
#define PHONELINE_HACK
class PhoneLine { public: enum QueryType {}; };
class PhoneBookEntry {};
#endif
#endif
#endif

class AbLabel : public QTextBrowser
{
    Q_OBJECT

public:
    AbLabel( QWidget *parent, const char *name = 0 );
    ~AbLabel();

    virtual void setSource( const QString& name );

    bool AbLabel::decodeHref(const QString& href, ServiceRequest* req, QString* pm) const;
    PimContact entry() const;

public slots:
    void init( const PimContact &entry );
    void sync();

signals:
    void okPressed();
    void previous();
    void next();
    void externalLinkActivated();

protected:
    void keyPressEvent( QKeyEvent * );
    QString contactToRichText( const PimContact & contact, bool dialer );
    QString businessRichText( const PimContact &contact, bool dialer );
    QString personalRichText( const PimContact &contact, bool dialer );
    QString busPhoneRichText( const PimContact &contact, bool dialer );
    QString homePhoneRichText( const PimContact &contact, bool dialer );
    QString emailRichText( const PimContact &contact );
    QString voipIdRichText( const PimContact &contact, bool dialer );    
    
private slots:
    void notification( PhoneLine::QueryType type, const QString &value );

private:
#ifdef QTOPIA_DATA_LINKING
    QDLClient *mNotesQC;
#endif
    ServiceRequest anchorService(const QString& name) const;
    PimContact ent;
    bool dirty;
    bool mOnlyActivateDialLinks;
#ifdef QTOPIA_VOIP
    PhoneLine *line;
    bool available;
    QString monitoredUri;
#endif
};

#endif // ABLABEL_H

