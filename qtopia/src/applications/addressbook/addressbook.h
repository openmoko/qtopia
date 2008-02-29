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

#ifndef Addressbook_H
#define Addressbook_H

#include <qmainwindow.h>
#include <qdialog.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qaction.h>
#include <qlineedit.h>

#include <qtopia/pim/private/contactxmlio_p.h>
#include <qtopia/categories.h>
#include <qtopia/fieldmapimpl.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif

#ifndef QTOPIA_DESKTOP
#include "sendcontactservice.h"
#endif

#ifndef QTOPIA_PHONE
#ifndef QTOPIA_DESKTOP
#ifndef AB_PDA
#define AB_PDA
#endif
#endif
#endif

// Keep this in sync with the copy in ablabel.h.
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

class AbEditor;
class AbLabel;
class AbTable;
class QPEToolBar;
class QPopupMenu;
class QToolButton;
class QLineEdit;
class QDialog;
class QLabel;
class Ir;
class CategorySelect;
class QVBox;
class CategorySelectDialog;
class QButtonGroup;
class SipContact;

#ifdef QTOPIA_PHONE
class LoadIndicator;
#endif

class AbSettings : public QDialog
{
    Q_OBJECT
public:
    AbSettings(QWidget *parent = 0, const char *name = 0, bool modal = TRUE, WFlags = 0);
    virtual ~AbSettings();

#ifdef QTOPIA_PHONE
    void saveFormat();

private:
    QButtonGroup* bg;
#else
    void setCurrentFields(const QValueList<int> &);
    QValueList<int> fields() { return map->fields(); }

private:
    FieldMap *map;
#endif
};

class AddressbookWindow: public QMainWindow
{
    Q_OBJECT
public:
    AddressbookWindow( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    virtual ~AddressbookWindow();

protected:
    void resizeEvent( QResizeEvent * e );
    void newEntry();
    void newEntry( const PimContact &cnt );
    void editEntry( const PimContact &cnt );
    void closeEvent( QCloseEvent *e );
    void keyPressEvent(QKeyEvent *);
    AbEditor *editor();
    void showEvent( QShowEvent *e );
    
#ifdef QTOPIA_PHONE
    bool eventFilter( QObject *o, QEvent *e );
#endif

signals:
    void updatePresenceStatusOfContact(int);

public slots:
    void appMessage(const QCString &, const QByteArray &);
    void setDocument( const QString & );
    void reload();
    void flush();

    void sendVCardViaSms(PimContact, int);
    void smsFinished( const PhoneLine &, const QString &, bool );

private slots:
    void sendContact();
    void sendContactCat();
//    void sendContactDetail();
    void selectClicked();
    void slotListNew();
    void slotListView();
    void slotDetailView();
    void slotListDelete();
    void slotViewBack();
    void slotViewEdit();
    void slotPersonalView();
    void editPersonal();
    void viewNext();
    void viewPrevious();
    void markCurrentAsPersonal();
    void beamDone( Ir * );
    void slotFind(bool);
    void search( const QString &k );
    void slotSetCategory( int );
    void catChanged();
    void updateIcons();
    /* simply disabled for now although this might be useful
    void contactFilterSelected( int idx );
    */
    void selectAll();
    void configure();
    void selectCategory();
    void viewOpened( const PimContact &entry );
    void viewClosed();
    void addToSpeedDial();
    void storeOnSim(bool);
    void setHighlightedLink(const QString&);
    void setSimFieldLimits( PhoneLine::QueryType, const QString &value );
    void phonebookChanged( const PhoneLine& line, const QString& store, const QValueList<PhoneBookEntry>& list );
    
private:
#ifdef QTOPIA_PHONE
    void smsBusinessCard();
#endif
    void receiveFile( const QString & );
    void readConfig();
    void writeConfig();
    QString categoryLabel( int id );
    AbLabel *abView();
    void showView();
    void deleteContacts(QValueList<QUuid> &);
    void beamContacts(const QString& description, const QValueList<PimContact>& list);
    void beamVCard( const QCString &filename, const QString &description = QString::null );
    void sendContacts(const QString& description, const QValueList<PimContact>& list, int field = -1);
    void showJustItem(const QUuid& uid);

#ifdef QTOPIA_DATA_LINKING
    QValueList<PimContact> mContactViewStack;
#endif
    bool allowCloseEvent;
    bool mResetKeyChars;
    bool mCloseAfterView;
    int mAwaitingSendConfirmations;
    bool mCloseAfterLastConfirm;
    ContactXmlIO contacts;
    Categories cats;
    QPEToolBar *listTools;
    QToolButton *deleteButton;
    CategorySelect *catSelect;
    enum Panes { paneList=0, paneView, paneEdit };
    QVBox *listView;
    AbEditor *abEditor;
    AbLabel *mView;
    AbTable *abList;

    QPEToolBar *searchBar;
    QLineEdit *searchEdit;
    QPopupMenu *viewmenu;

#ifndef QTOPIA_DESKTOP
    SendContactService *mSendService;
    QAction *actionSend;
    QAction *actionSendCat;
//    QAction *actionSendDetail;
#endif

    QAction *actionNew, *actionEdit, *actionTrash, *actionFind,
	*actionPersonal, *actionSetPersonal, 
	*actionSettings;

#ifdef AB_PDA
    QAction *actionBack;
#endif

#ifdef QTOPIA_PHONE
    PhoneLine *mPhoneLine;
    QValueList<PhoneBookEntry> mPhoneBookEntries;

    ContextMenu *contextMenu;
    QAction *actionCategory, *actionSpeedDial, *actionOnSim;

    bool mToggleInternal;
    CategorySelectDialog *categoryDlg;
    QLabel *categoryLbl;

    QLineEdit *mFindLE;
    
    SMSRequest* mReq;
#endif

    int viewMargin;

    bool checkSyncing();
    bool syncing;
    bool showingPersonal;
    /*
    QMap<int,int> contactMap;
    */
    
    QString sel_href;

#ifdef QTOPIA_PHONE
    QLabel *mSimIndicator;
    bool mGotSimEntries;
#endif
#ifdef Q_WS_QWS
    QString beamfile;
#endif
};

#endif
