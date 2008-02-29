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
#ifndef Addressbook_H
#define Addressbook_H

#include <QMainWindow>
#include <QDialog>
#include <QStringList>
#include <QSet>
#include <QAction>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QStringList>

#include <qcategorymanager.h>
#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qpimsource.h>
#include <qtopiaabstractservice.h>

#ifndef QTOPIA_PHONE
#       ifndef AB_PDA
#           define AB_PDA
#       endif
#endif

#ifdef QTOPIA_PHONE
#   include <qtopia/qsoftmenubar.h>
#ifdef QTOPIA_CELL
#   include <qtopiaphone/qphonebook.h>
#endif
#endif

class AbEditor;
class AbLabel;
class QPopupMenu;
class QToolButton;
class QLineEdit;
class QDialog;
class QLabel;
class QCategorySelector;
class QContactListView;
class QButtonGroup;
class QStackedWidget;
class QContact;
class QContactModel;
class QContactDelegate;
class QUniqueId;
class QDSData;
class QDSActionRequest;

#ifdef QTOPIA_PHONE
class QLineEditWithPreeditNotification;
#endif

class AddressbookWindow : public QMainWindow
{
    Q_OBJECT
    friend class ContactsService;
    friend class ContactsPhoneService;
public:
    AddressbookWindow(QWidget *parent = 0, Qt::WFlags f = 0);
    virtual ~AddressbookWindow();

protected:
    void resizeEvent(QResizeEvent * e);
    void newEntry();
    void newEntry(const QContact &cnt);
    void editEntry(const QContact &cnt);
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *);
    AbEditor *editor();
    void showEvent(QShowEvent *e);

#ifdef QTOPIA_PHONE
    bool eventFilter(QObject *o, QEvent *e);
#endif

public slots:
    void appMessage(const QString &, const QByteArray &);
    void setDocument(const QString &);
    void reload();
    void flush();
#ifdef QTOPIA_CELL
    void vcardDatagram( const QByteArray& data );
#endif

private slots:
    void delayedInit();
    void sendContact();
    void sendContactCat();
    void selectClicked();
    void slotListNew();
    void slotListView();
    void slotDetailView();
    void slotListDelete();
    void slotViewBack();
    void slotViewEdit();
    void slotPersonalView();
    void editPersonal();
    void addPhoneNumberToContact(const QString& phoneNumber);
    void createNewContact(const QString& phoneNumber);
    void setContactImage(const QString& filename);
    void markCurrentAsPersonal();
    void slotFind(bool);
    void search( const QString &k );
    void showCategory( const QCategoryFilter &);
    void updateIcons();
    void selectAll();
    void configure();
    void selectCategory();
    void viewOpened( const QContact &entry );
    void viewClosed();
    void addToSpeedDial();
    void setHighlightedLink(const QString&);
    void selectSources();
    void importAllFromSim();
    void exportAllToSim();

    void importCurrentFromSim();
    void exportCurrentToSim();
#ifdef QTOPIA_CELL
    void phonebookChanged( const QString& store );
#endif
    void setContactImage( const QDSActionRequest& request );
    void qdlActivateLink( const QDSActionRequest& request );
    void qdlRequestLinks( const QDSActionRequest& request );
    /* simply disabled for now although this might be useful
    void contactFilterSelected( int idx );
    */

    void contactsChanged();

private:
#ifdef QTOPIA_CELL
    void smsBusinessCard();
#endif

private:
    void createViewMenu();
    void createDetailedView();

    void receiveFile(const QString &filename, const QString &mimetype = QString());

    void readConfig();
    void writeConfig();

    void showView();

    void showJustItem(const QUniqueId &uid);

    bool checkSyncing();

    QDSData contactQDLLink( QContact& contact );
    void removeContactQDLLink( QContact& contact );
    void removeSpeedDial( QContact& contact );

    enum Panes
    {
        paneList = 0,
        paneView,
        paneEdit
    };

    QContactModel *contacts;

    bool mAllowCloseEvent;
    bool mResetKeyChars;
    bool mCloseAfterView;
    bool mFindMode;
    bool mHasSim;

    QStackedWidget *centralView;
    QToolBar *listTools;
    QToolButton *deleteButton;
    QCategorySelector *catSelect;

    QWidget *listView;
    QVBoxLayout *listViewLayout;
    AbEditor *abEditor;
    AbLabel *mView;
    QContactListView *abList;

    QToolBar *searchBar;
    QLineEdit *searchEdit;
    QPopupMenu *viewmenu;

    QAction *actionNew,
            *actionEdit,
            *actionTrash,
            *actionFind,
            *actionPersonal,
            *actionSetPersonal,
            *actionResetPersonal,
            *actionSettings
#ifdef QTOPIA_PHONE
            , *actionShowSources,
            *actionExportSim,
            *actionImportSim
#endif
            ;

    int viewMargin;


    bool syncing;
    bool showingPersonal;

    QString sel_href;

    QContactDelegate* contactDelegate;

    QAction *actionSend;
    QAction *actionSendCat;

#ifdef AB_PDA
    QAction *actionBack;
#endif

#ifdef QTOPIA_PHONE
    QAction *actionCategory, *actionSpeedDial;

    bool mToggleInternal;
    QLabel *categoryLbl;

    QLineEditWithPreeditNotification* mFindLE;
#endif

#ifdef QTOPIA_CELL
    QLabel *mSimIndicator;
    bool mGotSimEntries;
#endif
    QContact mCurrentContact;
    QList<QContact> mContactViewStack;
};

class AbDisplaySettings : public QDialog
{
    Q_OBJECT

public:
    AbDisplaySettings(QWidget *parent);

#ifdef QTOPIA_PHONE
    void saveFormat();
    QString format();

protected:
    void keyPressEvent(QKeyEvent* e);
/*#else
    void setCurrentFields(const QList<int> &);
    QValueList<int> fields() { return map->fields() }*/
#endif

private:
    QVBoxLayout* layout;
#ifdef QTOPIA_PHONE
    QButtonGroup* bg;
#endif
};

#ifdef QTOPIA_PHONE
//  Included only in Qtopia Phone for now - may be useful for PDA
//  or desktop some time down the track.
class AbSourcesDialog : public QDialog
{
    Q_OBJECT

public:
    AbSourcesDialog(QWidget *parent, const QSet<QPimSource> &availSources);
    void setSources(const QSet<QPimSource> &sources);
    QSet<QPimSource> sources() const;

signals:
    void importFromSimTriggered();
    void exportToSimTriggered();

protected:
    void keyPressEvent(QKeyEvent* e);

private:
    QVBoxLayout* layout;
    QButtonGroup* bg;
    QRadioButton* phoneButton;
    QRadioButton* simButton;
    QRadioButton* bothButton;
    QSet<QPimSource> availableSources;
};
#endif

class ContactsService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class AddressbookWindow;
private:
    ContactsService( AddressbookWindow *parent )
        : QtopiaAbstractService( "Contacts", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ContactsService();

public slots:
    void editPersonal();
    void editPersonalAndClose();
    void addContact(const QContact& contact);
    void removeContact(const QContact& contact);
    void updateContact(const QContact& contact);
    void addAndEditContact(const QContact& contact);
    void addPhoneNumberToContact(const QString& phoneNumber);
    void createNewContact(const QString& phoneNumber);
    void showContact(const QUniqueId& uid);
    void setContactImage(const QString& filename);
    void setContactImage( const QDSActionRequest& request );
    void activateLink( const QDSActionRequest& request );
    void requestLinks( const QDSActionRequest& request );

private:
    AddressbookWindow *parent;
};

#ifdef QTOPIA_PHONE

class ContactsPhoneService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class AddressbookWindow;
private:
    ContactsPhoneService( AddressbookWindow *parent )
        : QtopiaAbstractService( "ContactsPhone", parent )
        { this->parent = parent; publishAll(); }

public:
    ~ContactsPhoneService();

public slots:
#ifdef QTOPIA_CELL
    void smsBusinessCard();
    void pushVCard( const QDSActionRequest& request );
#endif

private:
    AddressbookWindow *parent;
};

#endif

#endif
