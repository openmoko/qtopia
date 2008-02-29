/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/**********************************************************************
** This file is part of the KPhone project.
** 
** KPhone is a SIP (Session Initiation Protocol) user agent for Linux, with
** which you can initiate VoIP (Voice over IP) connections over the Internet,
** send Instant Messages and subscribe your friends' presence information.
** Read INSTALL for installation instructions, and CHANGES for the latest
** additions in functionality and COPYING for the General Public License
** (GPL).
** 
** More information about Wirlab available at http://www.wirlab.net/
** 
** Note: "This program is released under the GPL with the additional
** exemption that compiling, linking, and/or using OpenSSL is allowed."
***********************************************************************/
#ifndef KSIPUSERINFO_H_INCLUDED
#define KSIPUSERINFO_H_INCLUDED

#include <qdatetime.h>
#include <qdialog.h>
#include <qptrdict.h>
#include <qxml.h>
#include <qstack.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include "../dissipate2/sipcall.h"
#include "menulistview.h"

class QListView;
class QListViewItem;
class QString;
class QPushButton;


class IncomingCall
{
public:
	IncomingCall( QString s, QDateTime time );
	~IncomingCall( void );
	QString getContact( void ) const { return contact; }
	QDateTime getDateTime( void ) const { return dt; }
private:
	QString contact;
	QDateTime dt;
};

class PhoneBookAddIdentity : public QDialog
{
	Q_OBJECT
public:
	PhoneBookAddIdentity( QWidget *parent = 0, const char *name = 0 );
	void clean( void );
	void setPerson( QString );
	void setIdentity( QString );
	void setDescription( QString );
	void setContact( bool );
	QString getIdentity( void );
	QString getDescription( void );
	QString getPerson( void );
	QString getContact( void );
signals:
	void done( void );
	void update( void );
protected slots:
	void slotOk( void );
	void slotCancel( void );
private:
	QLineEdit *identity;
	QLineEdit *description;
	QLineEdit *person;
	QCheckBox *contact;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

class PhoneBookEditIdentity : public QDialog
{
	Q_OBJECT
public:
	PhoneBookEditIdentity( QWidget *parent = 0, const char *name = 0 );
	~PhoneBookEditIdentity( void );
	QString getIdentity( void );
	QString getDescription( void );
	QString getContact( void );
	void setIdentity( QString );
	void setDescription( QString );
	void setContact( QString );
signals:
	void done( void );
	void update( void );
protected slots:
	void slotOk( void );
	void slotCancel( void );
private:
	QLineEdit *identity;
	QLineEdit *description;
	QCheckBox *contact;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

class PhoneBookEditPerson : public QDialog
{
	Q_OBJECT
public:
	PhoneBookEditPerson( QWidget *parent = 0, const char *name = 0 );
	~PhoneBookEditPerson( void );
	QString getPerson( void );
	void setPerson( QString );
signals:
	void done( void );
	void update( void );
protected slots:
	void slotOk( void );
	void slotCancel( void );
private:
	QLineEdit *person;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
};

class PhoneBookParser : public QXmlDefaultHandler
{
public:
	PhoneBookParser( QListView * t, bool CallRegister = true, bool rejectedContact = false );
	bool startDocument();
	bool startElement( const QString&, const QString&, const QString&, const QXmlAttributes& );
	bool endElement( const QString&, const QString&, const QString& );
	QListViewItem *getListPhoneBook( void ) { return listPhoneBook; }
	QListViewItem *getListReceivedCalls( void ) { return listReceivedCalls; }
	QListViewItem *getListMissedCalls( void ) { return listMissedCalls; }
	QListViewItem *getRejectedContacts( void ) { return rejected; }
private:
	QStack<QListViewItem> stack;
	QListViewItem *rejected;
	QListViewItem *listPhoneBook;
	QListViewItem *listReceivedCalls;
	QListViewItem *listMissedCalls;
};

class PhoneBook : public QDialog
{
	Q_OBJECT
public:
	PhoneBook( QString fileUserInfo, QWidget *parent, const char *name ,
		QList<IncomingCall> &receivedCalls, QList<IncomingCall> &missedCalls,
		QStringList &rejectedContacts );
	~PhoneBook( void );
	void addContact( SipCallMember *member );
	QStringList getRejectedContactsList();
private slots:
	bool setIdentity( QListViewItem * );
	void setIdentityAndDone( QListViewItem * );
	void clickAddButton( void );
	void clickEditButton( void );
	void clickRemoveButton( void );
	void clickHideButton( void );
public slots:
	QString getUri();
private:
	void save( void );
	QString clean( QString s );
	MenuListView *phoneBook;
	QPushButton *add_button;
	QPushButton *edit_button;
	QPushButton *remove_button;
	QPushButton *save_button;
	QString remote;
	PhoneBookAddIdentity *addIdentity;
	PhoneBookEditIdentity *editIdentity;
	PhoneBookEditPerson *editPerson;
	QString filePhoneBook;
	PhoneBookParser *parser;
	QString caption;
};

#endif // KSIPUSERINFO_H_INCLUDED
