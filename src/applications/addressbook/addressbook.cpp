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

#include "addressbook.h"
#include "abeditor.h"
#include "contactdetails.h"
#include "contactsource.h"
#include "contactlistpane.h"

#include "groupview.h"

#include <qtopiasendvia.h>
#include <qtopiaapplication.h>
#include <qdawg.h>
#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactview.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/pim/qappointmentmodel.h>
#include <qtopianamespace.h>
#include <qcontent.h>
#include <qthumbnail.h>
#include <qpimrecord.h>
#include <quniqueid.h>
#ifdef QTOPIA_CELL
#include <qphonebook.h>
#include <qsiminfo.h>
#endif

#include "emaildialogphone.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <QDialog>
#include <QFile>
#include <QPixmap>
#include <QPixmapCache>
#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QStringList>
#include <QMessageBox>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QDesktopWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QStackedWidget>
#include <qtranslatablesettings.h>
#include <QTimer>
#include <QDSData>
#include <QDSActionRequest>
#include <QDSServices>
#include <QDSAction>
#include <QDLLink>
#include <QItemDelegate>

#include <qtopia/qspeeddial.h>
#include <qtopia/qsoftmenubar.h>
#include <qtopiaipcenvelope.h>
#include <qdl.h>

#include <QListWidgetItem>
#include "qcontactview.h"
#include "qtopiaitemdelegate.h"
#include "ui_actiondialog.h"
#include "qspeeddial.h"

static bool constructorDone = false;

typedef enum SpeedDialType {Call = QListWidgetItem::UserType, Text, Email, View};

/***************************
  * NameLearner
  ***********************/

class NameLearner {
    // Utility class for efficiently putting names from contacts into word lists.
public:
    NameLearner()
    {
    }

    NameLearner(const QContact &cnt)
    {
        learn(cnt);
    }

    ~NameLearner()
    {
        const QDawg& dawg = Qtopia::dawg("addressbook-names");
        const QDawg& gldawg = Qtopia::dawg("local");

        QStringList aw;
        QStringList glaw;

        for (QStringList::ConstIterator it=nw.begin(); it!=nw.end(); ++it) {
            if ( isLetters(*it) ) {
                if ( !dawg.contains(*it) )
                    aw.append(*it);
                if ( !gldawg.contains(*it) )
                    glaw.append(*it);
            } else {
                // Maybe just spaces (eg. multi-word names)
                QStringList ws = (*it).split(QChar(' '));
                for (QStringList::ConstIterator lit=ws.begin(); lit!=ws.end(); ++lit) {
                    if ( isLetters(*lit) && !dawg.contains(*lit) )
                        aw.append(*lit);
                    if ( isLetters(*lit) && !gldawg.contains(*lit) )
                        glaw.append(*lit);
                }
            }
        }

        Qtopia::addWords("addressbook-names",aw);
        Qtopia::addWords(glaw);
    }

    void learn(const QContact &cnt)
    {
        for (int i = 0; i < 6; i++) {
            QString n;
            switch (i) {
                case 0:
                    n = cnt.firstName();
                    break;
                case 1:
                    n = cnt.middleName();
                    break;
                case 2:
                    n = cnt.lastName();
                    break;
                case 3:
                    n = cnt.company();
                    break;
                case 4:
                    n = cnt.spouse();
                    break;
                case 5:
                    n = cnt.assistant();
                    break;
                case 6:
                    n = cnt.manager();
                    break;
            }
            if ( !n.isEmpty() && !map.contains(n) ) {
                nw.append(n);
                map.insert(n,true);
            }
        }

    }

private:
    static bool isLetters(const QString& w)
    {
        for (int i=0; i<(int)w.length(); ++i) {
            if ( !w[i].isLetter() || w[i].unicode() >= 4096/*QDawg limit*/ )
                return false;
        }
        return true;
    }

    QStringList nw;
    QMap<QString, bool> map;
};



/***************************
  * LoadIndicator
  ***********************/

class BashfulProgressBar : public QProgressBar
{
public:
    BashfulProgressBar(QWidget *p) : QProgressBar(p) {};

    void paintEvent(QPaintEvent *pe)
    {
        if (maximum() > 0)
            QProgressBar::paintEvent(pe);
        else {
            QPainter p(this);
            p.fillRect(pe->rect(), palette().background());
        }
    }
};

class LoadIndicator : public QDialog
{
public:
    LoadIndicator( QWidget *parent = 0, Qt::WFlags fl = 0/*Qt::Tool|Qt::WindowStaysOnTopHint*/ );
    void bottom();
    void setText(const QString& t) { l->setText("<qt><img src=:image/AddressBook><br>"+t); }
    void setCount(int total) { p->setMaximum(total); p->update(); }
    void setProgress(int x) { p->setValue(x); }

protected:
    void showEvent( QShowEvent *e );
    void paintEvent( QPaintEvent *e );

private:
    QLabel *l;
    QProgressBar *p;
};


LoadIndicator::LoadIndicator( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal(true);
    l = new QLabel(this);
    l->setAlignment( Qt::AlignCenter );
    p = new BashfulProgressBar(this);

    QFrame *f = new QFrame(this);
    f->setFrameShape(QFrame::StyledPanel);

    QVBoxLayout *vb = new QVBoxLayout(f);
    vb->addWidget(l);
    vb->addWidget(p);

    vb = new QVBoxLayout(this);
    vb->addWidget(f);
}

void LoadIndicator::bottom()
{
    adjustSize();
    QRect rect;
    if (parentWidget())
        rect = parentWidget()->frameGeometry();
    else
        rect = QApplication::desktop()->availableGeometry();

    QSize s = 2 * sizeHint();
    setGeometry( 0, rect.bottom()-height(), rect.width(), height() );
}

void LoadIndicator::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
}

void LoadIndicator::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.fillRect( 0, 0, width(), height(), palette().background() );
    p.end();
    QDialog::paintEvent( e );
}

/***************************
  * AddressbookWindow
  ***********************/

AddressbookWindow::AddressbookWindow( QWidget *parent, Qt::WFlags f )
    : QMainWindow( parent, f ),
      mFilterModel(0),
      mCloseAfterView( false ),
      mHasSim( false ),
#ifdef QTOPIA_CELL
      mGotSimEntries(false),
      mShowSimLabel(false),
#endif
      mContextMenuDirty(true),
      mGroupsListView(0),
      mGroupMemberView(0),
      mDetailsView(0),
      abFullEditor(0),
#ifdef QTOPIA_CELL
      abSimEditor(0),
#endif
      actionPersonal(0),
      actionSetPersonal(0),
      actionResetPersonal(0),
      actionSpeedDial(0),
      syncing(false),
      mCurrentContactDirty(false),
      loadinfo(0)
{
    QtopiaApplication::loadTranslations( "libqtopiapim" ); //no tr

    mModel = new QContactModel(this);
    mHasSim = !mModel->simSource().isNull();

    // Grab the visible sources (before we connect modelReset, to avoid unconstructed stuff)
    readConfig();

    // Make sure when the model changes we update the icons (for the
    // initial contacts cache load, for example)
    connect(mModel, SIGNAL(modelReset()), this, SLOT(contactsChanged()));

    //
    //  Set up and populate the window
    //
    setWindowTitle( tr("Contacts") );
    setWindowIcon(QIcon(":image/AddressBook"));

    centralView = new QStackedWidget;

    setCentralWidget(centralView);

    //
    //  Build the main contact list view
    //
#ifndef GREENPHONE_EFFECTS
    mListView = new ContactListPane(0, mModel);
#else
    mListView = new ContactListPane(centralView, mModel);
#endif

    connect(mListView, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentContactSelectionChanged()));

    connect(mListView, SIGNAL(contactActivated(QContact)), this, SLOT(contactActivated(QContact)));
    connect(mListView, SIGNAL(backClicked()), this, SLOT(close()));

    centralView->addWidget(mListView);

    // Cache some icons for the menu actions
    QIcon newIcon(":icon/new");
    QIcon editIcon(":icon/edit");
    QIcon trashIcon(":icon/trash");
    QIcon businessIcon(":icon/business");
    QIcon syncIcon(":icon/sync");
    QIcon beamIcon(":icon/beam");

    //
    //  Create the menu actions.
    //
    actionNew = new QAction(newIcon, tr("New contact"), this);
    connect(actionNew, SIGNAL(triggered()), this, SLOT(createNewContact()));
    actionNew->setWhatsThis(tr("Enter a new contact."));

    actionTrash = new QAction(trashIcon, tr("Delete contact"), this);
    actionTrash->setWhatsThis(tr("Delete the selected contacts."));
    connect( actionTrash, SIGNAL(triggered()), this, SLOT(deleteContact()));

    /* group actions */

    actionShowGroups = new QAction(QIcon(":icon/contactgroup"), tr("Show Groups..."), this);
    actionShowGroups->setWhatsThis(tr("Show the list of contact groups."));
    connect( actionShowGroups, SIGNAL(triggered()), this, SLOT(groupList()));

    actionAddGroup = new QAction(newIcon, tr("New group"), this);
    actionAddGroup->setWhatsThis(tr("Add new contact group."));

#if defined(QTOPIA_TELEPHONY)
    actionSetRingTone = new QAction(QIcon(), tr("Set group ringtone...", "Set ringtone to current contact group"), this);
    actionSetRingTone->setWhatsThis(tr("Set a ringtone that is played when an incoming call comes in from this group members."));
#endif

    actionRemoveGroup = new QAction(trashIcon, tr("Delete", "Delete current contact group"), this);
    actionRemoveGroup->setWhatsThis(tr("Delete highlighted contact group."));

    actionRenameGroup = new QAction(editIcon, tr("Rename", "Rename current contact group"), this);
    actionRenameGroup->setWhatsThis(tr("Rename highlighted contact group."));

    actionRemoveFromGroup = new QAction(QIcon(), tr("Remove from group", "Remove contact from current group"), this);
    actionRemoveFromGroup->setWhatsThis(tr("Remove highlighted contact from current group."));

    actionAddMembers = new QAction(QIcon(), tr("Add members", "Add members to current contact group"), this);
    actionAddMembers->setWhatsThis(tr("Add contacts to current group."));

    /* End group actions */

    actionPersonal = new QAction(businessIcon, tr("Show My Card"), this);
    actionPersonal->setWhatsThis(tr("Show your business card entry."));
    connect(actionPersonal, SIGNAL(triggered()), this, SLOT(showPersonalView()));

    actionSetPersonal = new QAction(businessIcon, tr("Set as My Card"), this);
    actionSetPersonal->setWhatsThis(tr("Set current entry as your business card."));
    connect(actionSetPersonal, SIGNAL(triggered()), this, SLOT(markCurrentAsPersonal()));

    actionResetPersonal = new QAction(businessIcon, tr("Remove as My Card"), this);
    actionResetPersonal->setWhatsThis(tr("Remove current entry as your business card."));
    connect(actionResetPersonal, SIGNAL(triggered()), this, SLOT(markCurrentAsPersonal()));

    actionImportSim = new QAction(syncIcon, tr("Import to Phone"), this);
    actionImportSim->setWhatsThis(tr("Import current entry from SIM card to phone."));
    connect(actionImportSim, SIGNAL(triggered()), this, SLOT(importCurrentFromSim()));
    if (!mHasSim)
        actionImportSim->setVisible(false);

    actionExportSim = new QAction(syncIcon, tr("Export to SIM"), this);
    actionExportSim->setWhatsThis(tr("Export current entry from phone to SIM card."));
    connect(actionExportSim, SIGNAL(triggered()), this, SLOT(exportCurrentToSim()));
    if (!mHasSim)
        actionExportSim->setVisible(false);

    actionSettings = new QAction(QIcon(":icon/settings"), tr("Display Settings..."), this);
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(configure()));

    actionShowSources = new QAction(QIcon(":icon/viewcategory"),
        tr("Show Contacts From...", "e.g. Show Contacts From Phone/SIM Card"), this);
    connect(actionShowSources, SIGNAL(triggered()), this, SLOT(selectSources()));
    if (!mHasSim)
        actionShowSources->setVisible(false);

    actionSend = new QAction(beamIcon, tr("Send..."), this);
    actionSend->setWhatsThis(tr("Send the contact to another person"));
    connect(actionSend, SIGNAL(triggered()), this, SLOT(sendContact()));

    actionSendCat = new QAction(beamIcon, tr("Send All..."), this);
    actionSendCat->setWhatsThis(tr("Send the visible contacts to another person"));
    connect(actionSendCat, SIGNAL(triggered()), this, SLOT(sendContactCat()));

    //
    //  Create Context Menu
    //

    QMenu* contextMenu = QSoftMenuBar::menuFor(this);
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenuIfDirty()));

    contextMenu->addAction(actionNew);
    contextMenu->addAction(actionSendCat);

    actionSpeedDial = new QAction(QIcon(":icon/phone/speeddial"), tr("Add to Speed Dial..."), this);
    connect(actionSpeedDial, SIGNAL(triggered()), this, SLOT(addToSpeedDial()));

    /* groups */
    contextMenu->addAction(actionShowGroups);
    contextMenu->addAction(actionAddGroup);
#if defined(QTOPIA_TELEPHONY)
    contextMenu->addAction(actionSetRingTone);
#endif
    contextMenu->addAction(actionRemoveGroup);
    contextMenu->addAction(actionRenameGroup);
    contextMenu->addAction(actionRemoveFromGroup);
    contextMenu->addAction(actionAddMembers);
    /* end groups */

    contextMenu->addAction(actionSettings);
    contextMenu->addAction(actionPersonal);
    contextMenu->addAction(actionShowSources);

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
            this, SLOT(appMessage(QString,QByteArray)) );
    connect( qApp, SIGNAL(reload()), this, SLOT(reload()) );
    connect( qApp, SIGNAL(flush()), this, SLOT(flush()) );

    new ContactsService(this);

    new ContactsPhoneService(this);

#ifdef QTOPIA_CELL
    QPhoneBook *phoneBook = new QPhoneBook( QString(), this );
    connect( phoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
             this, SLOT(phoneBookUpdated(QString)) );

    mSimInfo = new QSimInfo(QString(), this);
    connect (mSimInfo, SIGNAL(inserted()), this, SLOT(simInserted()));
    connect (mSimInfo, SIGNAL(notInserted()), this, SLOT(simNotInserted()));
    connect (mSimInfo, SIGNAL(removed()), this, SLOT(simNotInserted()));
#endif

    constructorDone = true;

    // trigger delayed initialization.
    QTimer::singleShot(0, this, SLOT(delayedInit()));
}

void AddressbookWindow::delayedInit()
{
    QDSServices vcardServices( QString( "text/x-vcard" ),
                               QString( "" ),
                               QStringList( QString ( "send" ) ) );

    if ( vcardServices.count() == 0 ) {
        // Not ideal, but make startup much quicker.
        delete actionSend;
        delete actionSendCat;
        actionSend = 0;
        actionSendCat = 0;
    }
#ifdef QTOPIA_CELL
    /* Show the load labels if we haven't already got the entries */
    if (!mGotSimEntries) {
        if (!mSimInfo->identity().isEmpty())
            simInserted();
    }
#endif
}

void AddressbookWindow::clearSearchBars()
{
    mModel->clearFilter();
    mListView->resetSearchText();
    if (mGroupMemberView)
        mGroupMemberView->resetSearchText();
}

void AddressbookWindow::createViewMenu()
{
    if(!mDetailsView)
        return;

    QMenu *viewMenu = QSoftMenuBar::menuFor(mDetailsView);
    connect(viewMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenuIfDirty()));

    viewMenu->addAction(actionTrash);

    if( actionSend )
        viewMenu->addAction(actionSend);

    viewMenu->addAction(actionSetPersonal);
    viewMenu->addAction(actionResetPersonal);
    viewMenu->addAction(actionSpeedDial);
    viewMenu->addAction(actionExportSim);
    viewMenu->addAction(actionImportSim);
}

void AddressbookWindow::createDetailedView()
{
    if(!mDetailsView)
    {
        mDetailsView = new ContactDetails(0);
//        connect(mDetailsView, SIGNAL(externalLinkActivated()), this, SLOT(close()));
        connect(mDetailsView, SIGNAL(backClicked()), this, SLOT(previousView()));
        connect(mDetailsView, SIGNAL(callContact()), this, SLOT(callCurrentContact()));
        connect(mDetailsView, SIGNAL(textContact()), this, SLOT(textCurrentContact()));
        connect(mDetailsView, SIGNAL(emailContact()), this, SLOT(emailCurrentContact()));
        connect(mDetailsView, SIGNAL(editContact()), this, SLOT(editCurrentContact()));

        centralView->addWidget(mDetailsView);

        createViewMenu();
    }
}

/*
   Called when the ContactModel is reset (e.g. after refreshing
   cache from SIM or changing filter).  If we have any contacts, select the first,
   otherwise we show the "new" option.
*/
void AddressbookWindow::contactsChanged()
{
    updateContextMenu();
}

void AddressbookWindow::sendContact()
{
    QContact cnt = currentContact();
    if ( cnt != QContact() )
    {
        QString description = cnt.label();
        if ( description.isEmpty() )
            description = tr( "contact" );

        QByteArray vCardArray;
        {
            QDataStream stream( &vCardArray, QIODevice::WriteOnly );
            cnt.writeVCard( &stream );
        }

        QtopiaSendVia::sendData(this, vCardArray, "text/x-vcard");
    }
}

void AddressbookWindow::sendContactCat()
{
    // Send all contacts that are currently visible.
    if ( mModel->count() )
    {
        QByteArray vCardArray;
        {
            QDataStream stream( &vCardArray, QIODevice::WriteOnly );
            for( int i = 0; i < mModel->count(); i++ )
                mModel->contact(i).writeVCard( &stream );
        }

        QtopiaSendVia::sendData(this, vCardArray, "text/x-vcard");
    }
}

void AddressbookWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Back)
    {
        e->accept();
        if ( centralView->currentWidget() == mListView)
            close();
        else
            restoreViewState();
    }
    else if ( e->key() == Qt::Key_Call )
    {
        // We always handle this, but we don't actually call if we're on the group tab
        if ( centralView->currentWidget() == mListView) {
            if (currentContact() != QContact())
                callCurrentContact();
        }
        e->accept();
    }
    else
        QMainWindow::keyPressEvent(e);
}

void AddressbookWindow::setDocument( const QString &filename )
{
    receiveFile(filename);
}

void AddressbookWindow::receiveFile( const QString &filename, const QString &mimetype )
{
    loadingFile = filename;

    // setDocument assumes the UI needs to be shown, but
    // for addressbook setting is *adding*, and possibly cancelling,
    // which DOESN'T require showing.
    loadedWhenHidden = isHidden();

    deleteLoadingFile = true;

    if(mimetype.isEmpty())
    {
        deleteLoadingFile = false;
        if ( filename.right(8) == ".desktop" ) {
            QContent lnk(filename);
            if ( !lnk.isValid() || lnk.type().toLower() != "text/x-vcard" )
                return;
            loadingFile = lnk.fileName();
        } else if ( loadingFile.right(4) != ".vcf" ) {
            return;
        }
    }
    else if(mimetype.toLower() != "text/x-vcard") {
        QFile::remove(filename);
        return;
    }

    if ( !loadinfo )
        loadinfo = new LoadIndicator( this );
    loadinfo->setCount(0);
    loadinfo->setText( tr("Reading Contacts...") );
    loadinfo->setWindowTitle( tr("Contacts") );
    loadinfo->showMaximized();
    QtopiaApplication::setMenuLike(loadinfo,true);
    connect(loadinfo,SIGNAL(rejected()),this,SLOT(cancelLoad()));
    QTimer::singleShot(1000, this, SLOT(loadMoreVcards())); // XXX 1000 needed for slower hardware so Contacts has done start-up processing
    loadState=Start;

    QtopiaApplication::instance()->registerRunningTask(QLatin1String("ReceiveContacts"));
}


void AddressbookWindow::cancelLoad()
{
    loadState = Done;
}

void AddressbookWindow::loadMoreVcards()
{
    QWidget *mbParent = isVisible() ? this : 0;
    switch ( loadState ) {
      case Start:
        loadState = Read;
        break;
      case Read:
        loadedcl = QContact::readVCard( loadingFile );
        if ( loadedcl.count() == 0 ) {
            QMessageBox::warning(mbParent, tr("Invalid VCard"),
                tr("<qt>The VCard document did not contain any valid Contacts</qt>") );
            loadState = Done;
        } else {
            loadState = DuplicateCheck;
            loadinfo->setText( tr("Processing Contacts...") );
            loadednewContactsCursor = 0;
            loadinfo->setCount(loadedcl.count());
            loadinfo->showMaximized();
            loadednewContacts.clear();
            loadedoldContacts.clear();
        }
        break;
      case DuplicateCheck:
        {
            if (!mFilterModel)
                mFilterModel = new QContactModel(this);
            int perloop=4;
            while (perloop-->0 && loadedcl.count() > 0) {
                loadednewContactsCursor++;
                QContact c = loadedcl.takeFirst();

                QString baseDir = Qtopia::applicationFileName( "addressbook", "contactimages/" );

                QString vcfPhoto( c.customField( "phototmp" ) );
                bool newHasPhoto = !vcfPhoto.isEmpty();
                QByteArray newPixArray;

                // get an appropriate label field from c.
                // last name more likely unique for large contact lists
                // than first name.
                QString labelText;
                if (!c.lastName().isNull())
                    labelText = c.lastName();
                else if (!c.firstName().isNull())
                    labelText = c.firstName();
                else if (!c.company().isNull())
                    labelText = c.company();

                bool found = false;
                if (!labelText.isEmpty()) {
                    mFilterModel->setFilter(labelText);
                    for (int i =0; i < mFilterModel->count(); ++i) {
                        QContact match = mFilterModel->contact(i);
                        QString oldfile = match.portraitFile();

                        // Have to set a few things to compare properly
                        match.setUid(c.uid());
                        match.setPortraitFile(c.portraitFile()); // "phototmp" has the new pixmap
                        match.setCategories(c.categories());
                        if (!vcfPhoto.isEmpty())
                            match.setCustomField("phototmp", vcfPhoto);
                        else
                            match.removeCustomField("phototmp");
                        match.removeCustomField(QDL::CLIENT_DATA_KEY);
                        match.removeCustomField(QDL::SOURCE_DATA_KEY);

                        if (match == c) {
                            if (!oldfile.isEmpty()) {
                                // At least one old contact had a photo, so we need to compare
                                // binary files
                                if (!vcfPhoto.isEmpty()) {
                                    // Store the photo file in the db
                                    QByteArray encData = QByteArray( vcfPhoto.toLatin1(), vcfPhoto.length() );
                                    QByteArray decData = QByteArray::fromBase64(encData);

                                    QPixmap np;
                                    np.loadFromData( decData );
                                    if( !np.isNull() )
                                        c.changePortrait( np );
                                    else
                                        c.changePortrait( QPixmap() );

                                    // Don't store this in the db.
                                    c.removeCustomField("phototmp");

                                    // and only store the photo once
                                    vcfPhoto.clear();

                                    // Read the bytes in
                                    QFile newPix(baseDir + c.portraitFile());
                                    newPix.open(QIODevice::ReadOnly);
                                    newPixArray = newPix.readAll();
                                }

                                // Now load the old file
                                QFile oldPix(baseDir + oldfile);
                                oldPix.open(QIODevice::ReadOnly);
                                QByteArray oldPixArray = oldPix.readAll();

                                if (newPixArray == oldPixArray) {
                                    found = true;

                                    // We know we're not going to add this, so remove the
                                    // saved portrait
                                    c.changePortrait(QPixmap());
                                    break;
                                } else {
                                    // Not the same image, so we haven't found a duplicate
                                }
                                // Not the same, so it will be added later
                            } else if (!newHasPhoto && oldfile.isEmpty()) {
                                found = true;
                                break;
                            } else {
                                // One or the other has an image, so not a duplicate
                            }
                        }
                    }
                }
                if (found)
                    loadedoldContacts.append( c );
                else
                    loadednewContacts.append( c );
            }
            if (loadedcl.count() == 0) {
                loadState = Process;
                loadinfo->setProgress(0);
                loadinfo->setCount(0);
                loadednewContactsCursor = 0;
            } else {
                loadinfo->setProgress(loadednewContactsCursor);
            }
        }
        break;
      case Process:
        if ( loadednewContacts.count() > 0 ) {
            loadState = ConfirmAdd;
        } else if ( loadedoldContacts.count() > 0 ) {
            QString list = "";
            uint count = 0;
            for ( QList<QContact>::Iterator it = loadedoldContacts.begin(); it != loadedoldContacts.end(); ++it) {
                if ( count < 3 ) {
                    list += (*it).label() + "\n";
                } else if ( count == 3 ) {
                    list += "...\n";
                    break;
                }
                count++;
            }

            QString msg = tr("<qt>All %1 contact(s) are already in your addressbook: %2</qt>", "%1=number, %2=list of names").arg( loadedoldContacts.count() ).arg(list);
            QMessageBox::information(mbParent, tr("Contacts"), msg, QMessageBox::Ok);
            loadState = Done;
        }
        break;
      case ConfirmAdd:
        {
            QString list = "";
            uint count = 0;
            for ( QList<QContact>::Iterator it = loadednewContacts.begin(); it != loadednewContacts.end(); ++it) {
                if ( count < 3 ) {
                    list += (*it).label() + "\n";
                } else if ( count == 3 ) {
                    list += "...\n";
                    break;
                }
                count++;
            }
            QString msg = tr("<qt>%1 new Contact(s): %2<br>Do you want to add them to your addressbook?</qt>", "%1=number %2=list of names").arg( loadednewContacts.count() ).arg(list);
            if ( QMessageBox::information(mbParent, tr("Contacts"), msg, QMessageBox::Ok, QMessageBox::Cancel) ==
                QMessageBox::Ok )
            {
                loadState = Add;
                loadednewContactsCursor = 0;
                loadinfo->setCount(loadednewContacts.count());
                loadinfo->setText( tr("Adding Contacts...") );
                loadinfo->showMaximized();
            } else {
                // Not going to add, so remove any portraits that we actually loaded in the comparison stage
                for ( QList<QContact>::Iterator it = loadednewContacts.begin(); it != loadednewContacts.end(); ++it) {
                    if (!(*it).portraitFile().isEmpty()) {
                        (*it).changePortrait(QPixmap());
                    }
                }
                loadState = Done;
            }
        }
        break;
      case Add:
        {
            NameLearner namelearner;
            int perloop=4;
            while ( perloop-->0 && loadednewContactsCursor < loadednewContacts.count() ) {
                QContact curCon = loadednewContacts.at(loadednewContactsCursor);

                //if we have a photo for this contact, save it to disk
                QString vcfPhoto( curCon.customField( "phototmp" ) );
                if( !vcfPhoto.isEmpty() )
                {
                    QByteArray encData = QByteArray( vcfPhoto.toLatin1(), vcfPhoto.length() );
                    QByteArray decData = QByteArray::fromBase64(encData);

                    QPixmap np;
                    np.loadFromData( decData );
                    if( !np.isNull() )
                    {
                        curCon.changePortrait( np );
                    }
                    else
                    {
                        curCon.changePortrait( QPixmap() );
                    }

                    // Don't store this in the db.
                    curCon.removeCustomField("phototmp");
                }

                namelearner.learn(curCon);
                mModel->addContact(curCon);

                ++loadednewContactsCursor;
            }
            loadinfo->setProgress(loadednewContactsCursor);
            if (loadednewContactsCursor == loadednewContacts.count())
                loadState = Done;
        }
      case Done:
        ;
    }

    if ( loadState == Done ) {
        if (loadedWhenHidden)
            hide();
        delete loadinfo;
        loadinfo = 0;

        loadednewContacts.clear();
        loadedoldContacts.clear();
        loadedcl.clear();
        QtopiaApplication::instance()->unregisterRunningTask(QLatin1String("ReceiveContacts"));
        if (deleteLoadingFile)
            QFile::remove(loadingFile);
        mFilterModel->deleteLater();
        mFilterModel = 0;
    } else {
        QTimer::singleShot(100, this, SLOT(loadMoreVcards()));
    }
}


void AddressbookWindow::flush()
{
    mModel->flush();
    syncing = true;
}

void AddressbookWindow::reload()
{
    syncing = false;
    //Force a reload here
    if ( centralView->currentWidget() == mDetailsView)
    {
        createDetailedView();
        mDetailsView->init( currentContact() );
    }
    updateContextMenu();
}

void AddressbookWindow::resizeEvent( QResizeEvent *e )
{
    QMainWindow::resizeEvent( e );

    // we receive a resize event from qtable in the middle of the constrution, since
    // QTable::columnwidth does qApp->processEvents.  Ignore this event as it causes
    // all sorts of init problems for us
    if ( !constructorDone )
        return;

    if ( centralView->currentWidget() == mListView)
        showListView(false);
    else if ( centralView->currentWidget() == mGroupsListView)
        showGroupListView(false);
    else if ( centralView->currentWidget() == mDetailsView)
        showDetailsView(false);
    else if ( centralView->currentWidget() == mGroupMemberView)
        showGroupMemberView(false);
}

AddressbookWindow::~AddressbookWindow()
{
    if( !mModel->flush() )
        QMessageBox::information( this, tr( "Contacts" ),
                tr("<qt>Device full.  Some changes may not be saved.</qt>"));
    writeConfig();
}

void AddressbookWindow::readConfig(void)
{
    QSettings config( "Trolltech", "Contacts" );

    config.beginGroup( "default" );
    int count = config.beginReadArray("SelectedSources");
    QSet<QPimSource> set;
    for(int i = 0; i < count; ++i) {
        config.setArrayIndex(i);
        QPimSource s;
        s.context = QUuid(config.value("context").toString());
        s.identity = config.value("identity").toString();
        set.insert(s);
    }
    config.endArray();
    if (count > 0)
        mModel->setVisibleSources(set);
    config.endGroup();
}

void AddressbookWindow::writeConfig(void)
{
    /*
    QSettings config( "Trolltech", "Contacts" );
    config.endGroup();
    */
}

void AddressbookWindow::setContextMenuDirty()
{
    mContextMenuDirty = true;
}

void AddressbookWindow::updateContextMenu()
{
    mContextMenuDirty = true;
    updateContextMenuIfDirty();
}

void AddressbookWindow::updateContextMenuIfDirty()
{
    if (!mContextMenuDirty)
        return;

    mContextMenuDirty = false;

    bool details = centralView->currentWidget() == mDetailsView;

    bool editable = mModel->editable(currentContact().uid());

    bool hasMembers = mModel->count();

    bool showingGroups = centralView->currentWidget() == mGroupsListView;
    bool showingContacts = !showingGroups && !details;
    bool showingFiltered = centralView->currentWidget() == mGroupMemberView;

    bool groupSelected = showingGroups && mGroupsListView->currentIndex().isValid();
    bool groupSystem = groupSelected && mGroupsListView->isCurrentSystemGroup();

    actionSettings->setVisible(!details);

    actionSpeedDial->setVisible(details);
    actionTrash->setVisible(details && editable);

    if (mHasSim) {
        if (mModel->isSIMCardContact(currentContact().uid())) {
            actionExportSim->setVisible(false);
            actionImportSim->setVisible(true);
        } else {
            actionExportSim->setVisible(true);
            actionImportSim->setVisible(false);
        }
    }

    if (mModel->isPersonalDetails(currentContact().uid())) {
        actionResetPersonal->setVisible(details);
        actionSetPersonal->setVisible(false);
    } else {
        actionSetPersonal->setVisible(details);
        actionResetPersonal->setVisible(false);
    }

    if(actionSend)
        actionSend->setVisible(details);

    if(actionSendCat)
        actionSendCat->setVisible(!details && hasMembers);

    actionPersonal->setVisible(!details && mModel->hasPersonalDetails());

    actionNew->setVisible(showingContacts || details);

    /* group actions */
    actionShowGroups->setVisible( showingContacts );
    actionAddGroup->setVisible( showingGroups );
#if defined(QTOPIA_TELEPHONY)
    actionSetRingTone->setVisible( showingGroups );
#endif
    actionRemoveGroup->setVisible( groupSelected && !groupSystem);
    actionRenameGroup->setVisible( groupSelected && !groupSystem);
    actionRemoveFromGroup->setVisible( showingFiltered && hasMembers);
    actionAddMembers->setVisible( showingFiltered );
    /* end group actions */
}

bool AddressbookWindow::checkSyncing()
{
    if (syncing) {
        if ( QMessageBox::warning(this, tr("Contacts"),
                             tr("<qt>Can not edit data, currently syncing</qt>"),
                            QMessageBox::Ok, QMessageBox::Abort ) == QMessageBox::Abort )
        {
            // Okay, if you say so (eg. Qtopia Sync Agent may have crashed)....
            syncing = false;
        } else
            return true;
    }
    return false;
}


void AddressbookWindow::createNewContact()
{
    QContact cnt;
    if(!checkSyncing())
    {
        // Since this is called from our UI (not qcop),
        // set the current category of the new contact to our
        // current category.
        cnt.setCategories(mCurrentFilter.requiredCategories());
        newEntry(cnt);
    }
}

void AddressbookWindow::contactActivated(QContact c)
{
    if( !c.uid().isNull() ) {
        setCurrentContact(c);
        showDetailsView(true);
    } else
        createNewContact();
}

void AddressbookWindow::showListView(bool saveState)
{
    if (saveState)
        saveViewState();

    mModel->setCategoryFilter(QCategoryFilter());

    if (actionSendCat)
        actionSendCat->setText(tr("Send All..."));


    if ( centralView->currentWidget() != mListView) {
        centralView->setCurrentWidget(mListView);
        setWindowTitle( tr("Contacts") );
        updateContextMenu();
    }
}

/* show details view.. */
void AddressbookWindow::showDetailsView(bool saveState)
{
    if (saveState)
        saveViewState();

    if ( centralView->currentWidget() != mDetailsView) {
        createDetailedView();
        mDetailsView->init( currentContact() );
        centralView->setCurrentWidget(mDetailsView);
        setWindowTitle( tr("Contact Details") );
    } else
        mDetailsView->init( currentContact() );

    updateContextMenu();
}

void AddressbookWindow::showGroupListView(bool saveState)
{
    if (saveState)
        saveViewState();

    clearSearchBars();

    if ( centralView->currentWidget() != mGroupsListView) {
        createGroupListView();
        centralView->setCurrentWidget(mGroupsListView);
        setWindowTitle( tr("Contact Groups") );
        updateContextMenu();
    }
}

void AddressbookWindow::showGroupMemberView(bool saveState)
{
    if (saveState)
        saveViewState();

    clearSearchBars();

    if ( centralView->currentWidget() != mGroupMemberView ) {
        createGroupMemberView();
        centralView->setCurrentWidget(mGroupMemberView);
    }

    /* These need to change all the time */
    setWindowTitle( tr("Contacts") + " - " + mCurrentFilter.label("Address Book") );
    if (actionSendCat)
        actionSendCat->setText(tr("Send %1...").arg(mCurrentFilter.label("Address Book")));
    mGroupMemberView->showCategory(mCurrentFilter);
    mGroupMemberView->setCurrentContact(currentContact());

    updateContextMenu();
}

void AddressbookWindow::createGroupListView()
{
    if (!mGroupsListView) {
        mGroupsListView = new GroupView();

        connect(mGroupsListView, SIGNAL(groupHighlighted(QString)), this, SLOT(updateContextMenu()));
        connect(mGroupsListView, SIGNAL(groupActivated(QString)), this, SLOT(showCategory(QString)));
        connect(mGroupsListView, SIGNAL(backClicked()), this, SLOT(previousView()));

        connect(actionAddGroup, SIGNAL(triggered()), mGroupsListView, SLOT(addGroup()));
#if defined(QTOPIA_TELEPHONY)
        connect(actionSetRingTone, SIGNAL(triggered()), mGroupsListView, SLOT(setGroupRingTone()));
#endif
        connect(actionRemoveGroup, SIGNAL(triggered()), mGroupsListView, SLOT(removeCurrentGroup()));
        connect(actionRenameGroup, SIGNAL(triggered()), mGroupsListView, SLOT(renameCurrentGroup()));
        connect(actionRemoveFromGroup, SIGNAL(triggered()), this, SLOT(removeContactFromCurrentGroup()));
        connect(actionAddMembers, SIGNAL(triggered()), mGroupsListView, SLOT(editCurrentGroup()));

        centralView->addWidget(mGroupsListView);
    }
}

void AddressbookWindow::createGroupMemberView()
{
    if (!mGroupMemberView)
    {
        mGroupMemberView = new ContactListPane(0, mModel);

        connect(mGroupMemberView, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentContactSelectionChanged()));
        connect(mGroupMemberView, SIGNAL(contactActivated(QContact)), this, SLOT(contactActivated(QContact)));
        connect(mGroupMemberView, SIGNAL(backClicked()), this, SLOT(previousView()));

#ifdef QTOPIA_CELL
        mGroupMemberView->showLoadLabel(mShowSimLabel);
#endif

        mGroupMemberView->installEventFilter(this);
        // XXX we could also stuff this into a QStackedWidget
        mGroupMemberView->layout()->setMargin(2);
        centralView->addWidget(mGroupMemberView);
    }
}

QString AddressbookWindow::pickEmailAddress(QStringList emails)
{
    if (emails.count() > 1) {
        // Hmm, need another string picker
        QDialog emaildiag;
        Ui::ActionDialog eui;
        eui.setupUi(&emaildiag);

        emaildiag.setWindowTitle(tr("Choose address"));
        QtopiaApplication::setMenuLike(&emaildiag, true);
        eui.actionList->setItemDelegate(new QtopiaItemDelegate());

        foreach(QString e, emails) {
            eui.actionList->addItem( new QListWidgetItem(QIcon(":icon/email"), e));
        }

        eui.actionList->setCurrentRow(0);
        emaildiag.showMaximized();
        if (QtopiaApplication::execDialog(&emaildiag)) {
            return emails.value(eui.actionList->currentRow());
        }
    } else {
        // Only one email, so use it
        return emails[0];
    }
    return QString();
}

int AddressbookWindow::pickSpeedDialType(QStringList emails, QMap<QContact::PhoneType,QString> numbers)
{
    // Provide a list of choices of things to add...
    QDialog diag;
    Ui::ActionDialog ui;
    ui.setupUi(&diag);

    diag.setWindowTitle(tr("Add to Speed Dial"));
    QtopiaApplication::setMenuLike(&diag, true);
    ui.actionList->setItemDelegate(new QtopiaItemDelegate());

    QContact ent = currentContact();
    if (numbers.count() > 0) {
        if (numbers.count() == 1) {
            ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/phone/calls"),
                tr("Call %1", "Call Mr Jones").arg(ent.label()), 0, Call));
            ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/phone/sms"),
                tr("Text %1", "Text Mr Jones").arg(ent.label()), 0, Text));
        } else {
            ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/phone/calls"),
                tr("Call %1...", "Call Mr Jones...").arg(ent.label()), 0, Call));
            ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/phone/sms"),
                tr("Text %1...", "Text Mr Jones...").arg(ent.label()), 0, Text));
        }
    }

    if (emails.count() > 0) {
        if (emails.count() == 1) {
            ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/email"),
                tr("Email %1", "Email Mr Jones").arg(ent.label()), 0, Email));
        } else {
            ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/email"),
                tr("Email %1...", "Email Mr Jones...").arg(ent.label()), 0, Email));
        }
    }

    ui.actionList->addItem(new QListWidgetItem(QIcon(":icon/view"),
        tr("View details of %1", "View details of Mr Jones").arg(ent.label()), 0, View));

    ui.actionList->setCurrentRow(0);
    diag.showMaximized();
    if (QtopiaApplication::execDialog(&diag)) {
        return ui.actionList->currentItem()->type();
    }

    return -1;
}

bool AddressbookWindow::updateSpeedDialPhoneServiceDescription(QtopiaServiceDescription* desc, const QContact& ent, QContact::PhoneType phoneType, bool isSms)
{
    QString number = ent.phoneNumber(phoneType);
    if (!number.simplified().isEmpty()) {
        switch(phoneType) {
            case QContact::BusinessMobile:
                desc->setIconName(isSms ? "addressbook/businessmessage" : "addressbook/businessmobile");
                break;
            case QContact::BusinessPhone:
            case QContact::BusinessFax:
            case QContact::BusinessPager:
                desc->setIconName(isSms ? "addressbook/businessmessage" : "addressbook/businessphone");
                break;
            case QContact::HomeMobile:
            case QContact::Mobile:
            default:
                desc->setIconName(isSms ? "addressbook/homemessage" : "addressbook/homemobile");
                break;
            case QContact::HomePhone:
            case QContact::HomeFax:
            case QContact::HomePager:
            case QContact::OtherPhone:
            case QContact::Fax:
            case QContact::Pager:
                desc->setIconName(isSms ? "addressbook/homemessage" : "addressbook/homephone");
                break;
        }
        desc->setOptionalProperty("addressbook-contactid", QVariant::fromValue(ent.uid())); // no tr
        desc->setOptionalProperty("addressbook-phonenumber", number);
        desc->setOptionalProperty("addressbook-phonetype", phoneType); // no tr
        desc->setOptionalProperty("addressbook-choicecount", ent.phoneNumbers().count()); // no tr

        if (isSms) {
            QtopiaServiceRequest req = QtopiaServiceRequest("SMS", "writeSms(QString,QString)");
            req << ent.label() << number;
            desc->setLabel(tr("Text %1 (%2)", "Text Mr Jones (12345)").arg(ent.label()).arg(number)); // no tr
            desc->setOptionalProperty("addressbook-action", QString("text"));
            desc->setRequest(req);
        } else {
            QtopiaServiceRequest req = QtopiaServiceRequest("Dialer", "dial(QString,QUniqueId)");
            req << number << ent.uid();
            desc->setLabel(tr("Call %1 (%2)", "Call Mr Jones").arg(ent.label()).arg(number));
            desc->setOptionalProperty("addressbook-action", QString("call")); // no tr
            desc->setRequest(req);
        }
        return true;
    }
    return false;
}

bool AddressbookWindow::updateSpeedDialEmailServiceDescription(QtopiaServiceDescription *desc, const QContact &ent, const QString& emailAddress)
{
    if (!emailAddress.simplified().isEmpty()) {
        QtopiaServiceRequest req = QtopiaServiceRequest("Email", "writeMail(QString,QString)");
        req << ent.label() << emailAddress;
        desc->setLabel(tr("Email %1 (%2)", "Email Mr Jones").arg(ent.label()).arg(emailAddress));
        desc->setIconName("email"); // no tr
        desc->setOptionalProperty("addressbook-contactid", QVariant::fromValue(ent.uid())); // no tr
        desc->setOptionalProperty("addressbook-action", QString("email")); // no tr
        desc->setOptionalProperty("addressbook-emailaddress", emailAddress);
        desc->setOptionalProperty("addressbook-choicecount", ent.emailList().count()); // no tr
        desc->setRequest(req);
        return true;
    }
    return false;
}

bool AddressbookWindow::updateSpeedDialViewServiceDescription(QtopiaServiceDescription *desc, const QContact& ent)
{
    QtopiaServiceRequest req = QtopiaServiceRequest("Contacts", "showContact(QUniqueId)");
    req << ent.uid();
    desc->setLabel(tr("View details of %1", "View details of Mr Jones").arg(ent.label()));
    desc->setOptionalProperty("addressbook-contactid", QVariant::fromValue(ent.uid())); // no tr
    desc->setOptionalProperty("addressbook-action", QString("view"));
    desc->setIconName("view"); // no tr
    desc->setRequest(req);
    return true;
}

void AddressbookWindow::addToSpeedDial()
{
    QContact ent = currentContact();
    if ( !ent.uid().isNull() ) {
        QMap<QContact::PhoneType, QString> numbers = ent.phoneNumbers();
        QStringList emails = ent.emailList();

        int type = pickSpeedDialType(emails, numbers);
        if (type != -1) {
            QtopiaServiceDescription desc;
            bool validSD = false;

            switch(type) {
                case Call:
                case Text:
                    {
                        QContact::PhoneType ptype;

                        if (numbers.count() > 1) {
                            QPhoneTypeSelector pts(ent, QString());
                            pts.showMaximized();
                            if (QtopiaApplication::execDialog(&pts)) {
                                ptype = pts.selected();
                            } else
                                break;
                        } else {
                            ptype = numbers.begin().key();
                        }
                        validSD = updateSpeedDialPhoneServiceDescription(&desc, ent, ptype, type == Text);
                        break;
                    }

                case Email:
                    {
                        QString email = pickEmailAddress(emails);

                        validSD = updateSpeedDialEmailServiceDescription(&desc, ent, email);
                        break;
                    }

                case View:
                    {
                        validSD = updateSpeedDialViewServiceDescription(&desc, ent);
                        break;
                    }
            }

            if (validSD) {
                QString speedDialInput = QSpeedDial::addWithDialog( desc.label(), desc.iconName(), desc.request(), this);
                if ( !speedDialInput.isEmpty() ) {
                    QSpeedDial::set(speedDialInput, desc);
                }
            }
        }
    }
}

void AddressbookWindow::updateSpeedDialEntries(const QContact& cnt)
{
    // We might need to adjust the label of the speed dial entry
    // and perhaps the number dialed (if not the type)
    QList<QString> entries = QSpeedDial::assignedInputs();
    foreach (QString s, entries) {
        QtopiaServiceDescription *desc = QSpeedDial::find(s);

        if (desc) {
            QUniqueId id = desc->optionalProperty("addressbook-contactid").value<QUniqueId>();
            bool validDesc = true;
            if (cnt.uid() == id) {
                QString action = desc->optionalProperty("addressbook-action").toString();

                if (action == "view") {
                    updateSpeedDialViewServiceDescription(desc, cnt);
                    QSpeedDial::set(s, *desc);
                } else if (action == "call" || action == "text") {
                    // Ick.  If they now have no numbers, delete this entry
                    if (cnt.defaultPhoneNumber().isEmpty()) {
                        validDesc = false;
                    } else {
                        bool sms = (action == "text") ? true : false;

                        // See if the number has changed
                        QMap<QContact::PhoneType, QString> numbers = cnt.phoneNumbers();
                        QContact::PhoneType type = static_cast<QContact::PhoneType>(desc->optionalProperty("addressbook-phonetype").toUInt());
                        QString oldnumber = desc->optionalProperty("addressbook-phonenumber").toString();
                        int oldnumbers = desc->optionalProperty("addressbook-choicecount").toInt();

                        // Common case is just one phone number, which may have changed number or type
                        if (oldnumbers == 1 && numbers.count() == 1) {
                            type = numbers.begin().key();
                        } else {
                            // In general, we try to keep the same physical phone number if possible

                            // See if the old number is the same type
                            if (numbers.value(type) != oldnumber) {
                                // Nope - see if the old number still exists [really need contains(value)]
                                bool oldExists = false;
                                QMapIterator<QContact::PhoneType, QString> i(numbers);
                                while (i.hasNext()) {
                                    i.next();
                                    if (i.value() == oldnumber) {
                                        // Yep, still exists, but is a different type
                                        oldExists = true;
                                        type = i.key();
                                        break;
                                    }
                                }

                                // See if the old type still exists
                                if (!oldExists) {
                                    if (!numbers.contains(type)) {
                                        // The old number and the old type have disappeared... grab the default phonenumber instead.
                                        type = numbers.key(cnt.defaultPhoneNumber());
                                    }
                                }
                            }
                        }
                        validDesc = updateSpeedDialPhoneServiceDescription(desc, cnt, type, sms);
                    }
                } else if (action == "email") {
                    // If they don't have an email address any more, we delete this entry
                    if (cnt.defaultEmail().isEmpty()) {
                        validDesc = false;
                    } else {
                        // See if the previous email address exists...
                        QString oldemail = desc->optionalProperty("addressbook-emailaddress").toString();
                        if (!cnt.emailList().contains(oldemail))
                            validDesc = updateSpeedDialEmailServiceDescription(desc, cnt, cnt.defaultEmail()); // Hmm.. just pick the default email
                        else
                            validDesc = updateSpeedDialEmailServiceDescription(desc, cnt, oldemail);
                    }
                } else {
                    validDesc = false;
                }

                if (validDesc)
                    QSpeedDial::set(s, *desc);
                else
                    QSpeedDial::remove(s);
            }
        }
    }
}

void AddressbookWindow::removeSpeedDialEntries(const QContact& cnt)
{
    QList<QString> entries = QSpeedDial::assignedInputs();
    foreach (QString s, entries) {
        QtopiaServiceDescription *desc = QSpeedDial::find(s);

        if (desc) {
            QUniqueId id = desc->optionalProperty("addressbook-contactid").value<QUniqueId>();

            if (cnt.uid() == id) {
                // Belongs to us, so nuke it from orbit
                QSpeedDial::remove(s);
            }
        }
    }
}

bool AddressbookWindow::eventFilter(QObject* receiver, QEvent *e)
{
    if (receiver == mGroupMemberView && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent *) e;

        QModelIndex idx = mGroupsListView->currentIndex();

        switch (ke->key()) {
            case Qt::Key_Left:
                idx = idx.sibling(idx.row() - 1, idx.column());
                if (idx.isValid()) {
                    mGroupsListView->setCurrentIndex(idx);
                    showCategory(QCategoryFilter(mGroupsListView->currentGroup()), false);
                    updateGroupNavigation();
                }
                return true;

            case Qt::Key_Right:
                idx = idx.sibling(idx.row() + 1, idx.column());
                if (idx.isValid()) {
                    mGroupsListView->setCurrentIndex(idx);
                    showCategory(QCategoryFilter(mGroupsListView->currentGroup()), false);
                    updateGroupNavigation();
                }
                return true;
        }
    }
    return false;
}

void AddressbookWindow::updateGroupNavigation()
{
    QModelIndex idx = mGroupsListView->currentIndex();
    const QAbstractItemModel *model = idx.model();
    int current = idx.row();
    int max = 0;
    // XXX from e3_navipane.h
    enum Location {
        Beginning = 0, Middle, End, NA
    };
    int privateEnum = NA;
    if (model)
        max = model->rowCount();

    if (max <= 1)
        privateEnum = NA;
    else if (current > 0 && current < max - 1)
        privateEnum = Middle;
    else if (current > 0)
        privateEnum = End;
    else
        privateEnum = Beginning;

    // navipane label
    QString fulllabel;
    if (max == 1)
        fulllabel = mGroupsListView->currentGroup();
    else
        fulllabel = QString(tr("%1 (%2/%3)","%1=group, %2=current idx %3, %3=no of items in group")).arg(mGroupsListView->currentGroup()).arg(current + 1).arg(max);

    QtopiaIpcEnvelope e("QPE/NaviPane", "setLocationHint(int,int,QString,QIcon,int)");
    e << ((int)winId()) << ((int)winId()) << fulllabel << QIcon() << privateEnum;
}

void AddressbookWindow::deleteContact()
{
    if ( checkSyncing() )
        return;

    // get a name, do the best we can...
    QString strName = currentContact().label();
    if ( strName.isEmpty() ) {
        strName = currentContact().company();
        if ( strName.isEmpty() )
            strName = tr("No Name");
    }

    if ( Qtopia::confirmDelete( this, tr( "Contacts" ), strName ) ) {
        removeContactQDLLink( currentContact() );
        QUniqueId victim = currentContact().uid();
        QModelIndex idx = mModel->index(victim);
        mModel->removeContact( currentContact() );
        removeSpeedDialEntries(currentContact());

        // Try to select the same row (or the last..)
        if (idx.row() >= mModel->rowCount())
            idx = mModel->index(mModel->rowCount() - 1, 0);

        // Remove any previous views in the stack the refer to the
        // deleted contact
        QMutableListIterator<AB_State> li(mContactViewStack);
        while(li.hasNext()) {
            AB_State s = li.next();
            if (s.contact.uid() == victim) {
                // If it's a list view, try to select the contact at the same row.
                // If the current filter is different from the filter for the pane,
                // then the first entry will be selected instead (in setCurrentContact)
                if (s.pane == AB_State::List || s.pane == AB_State::GroupMembers) {
                    s.contact = mModel->contact(idx);
                    li.setValue(s);
                } else {
                    li.remove();
                }
            }
        }

        restoreViewState();
    }
}

void AddressbookWindow::previousView()
{
    if (mCloseAfterView) {
        mCloseAfterView = false;
        close();
    } else {
        restoreViewState();
    }
}

void AddressbookWindow::callCurrentContact()
{
    /* See if we need to pop a selector */
    QMap<QContact::PhoneType, QString> numbers = currentContact().phoneNumbers();

#if !defined(QTOPIA_VOIP)
    // If we don't have VOIP, we can't dial/text VOIP numbers
    numbers.remove(QContact::HomeVOIP);
    numbers.remove(QContact::BusinessVOIP);
    numbers.remove(QContact::VOIP);
#endif

    if (numbers.count() > 0) {
        /* XXX we could check to see if we can make calls here before popping a selector */
        if (numbers.count() > 1) {
            QPhoneTypeSelector *pts = new QPhoneTypeSelector(currentContact(), QString(), this);
            pts->setModal(true);
            pts->showMaximized();
            if (QtopiaApplication::execDialog(pts) && !pts->selectedNumber().isEmpty()) {
                QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" ); // No tr
                req << pts->selectedNumber();
                req << currentContact().uid();
                req.send();
            }
            pts->hide();
            pts->deleteLater();
        } else {
            /* Just the one, so call it */
            QMap<QContact::PhoneType, QString>::iterator it = numbers.begin();
            QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" ); // No tr
            req << it.value();
            req << currentContact().uid();
            req.send();
        }
    }
}

void AddressbookWindow::textCurrentContact()
{
    /* Allow any number to be texted... */
    QMap<QContact::PhoneType, QString> numbers = currentContact().phoneNumbers();

#if !defined(QTOPIA_VOIP)
    // If we don't have VOIP, we can't dial/text VOIP numbers
    numbers.remove(QContact::HomeVOIP);
    numbers.remove(QContact::BusinessVOIP);
    numbers.remove(QContact::VOIP);
#endif

    if ( numbers.count() > 0) {
        if ( numbers.count() > 1) {
            QPhoneTypeSelector *pts = new QPhoneTypeSelector(currentContact(), QString(), this);
            pts->setModal(true);
            pts->showMaximized();
            if (QtopiaApplication::execDialog(pts) && !pts->selectedNumber().isEmpty()) {
                QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)" );
                req << currentContact().label() << pts->selectedNumber();
                req.send();
            }
            pts->hide();
            pts->deleteLater();
        } else {
            QMap<QContact::PhoneType, QString>::iterator it = numbers.begin();
            QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)" );
            req << currentContact().label() << it.value();
            req.send();
        }
    }
}

void AddressbookWindow::emailCurrentContact()
{
    /* See if we need to pop a selector */
    QStringList emails = currentContact().emailList();

    if (emails.count() > 0) {
        if (emails.count() > 1) {
            EmailDialog * ed = new EmailDialog(this, true);
            ed->setEmails(currentContact().defaultEmail(), emails);
            ed->setModal(true);
            ed->showMaximized();
            QtopiaApplication::setMenuLike(ed, true);
            if (QtopiaApplication::execDialog(ed) && !ed->selectedEmail().isEmpty()) {
                QtopiaServiceRequest req( "Email", "writeMail(QString,QString)" );
                req << currentContact().label() << ed->selectedEmail();
                req.send();
            }
            ed->hide();
            ed->deleteLater();
        } else {
            /* Just the one, so call it */
            QtopiaServiceRequest req( "Email", "writeMail(QString,QString)" );
            req << currentContact().label() << emails.first();
            req.send();
        }
    }
}

void AddressbookWindow::editCurrentContact()
{
    if(!checkSyncing()) {
        Q_ASSERT(!currentContact().uid().isNull());
        editEntry(currentContact());
    }
}

/*
  Show item defined by uid, then close once user goes back.
*/
void AddressbookWindow::showJustItem(const QUniqueId& uid)
{
    if( isHidden() ) // only close after view if hidden on first activation
    {
        mCloseAfterView = true;
    }

    QContact cnt = mModel->contact(uid);

    /* Only save the state if this isn't our current contact or if we're not viewing details already */
    if (!mCloseAfterView && (cnt != currentContact() || centralView->currentWidget() != mDetailsView))
        saveViewState();
    setCurrentContact(cnt);
    showDetailsView(false);
    if (!isVisible())
        showMaximized();

    updateContextMenu();
}

void AddressbookWindow::addPhoneNumberToContact(const QString& phoneNumber)
{
    QContactSelector *s = new QContactSelector( false, this );
    s->setModel(mModel);
    s->setModal(true);
    s->showMaximized();

    if (s->exec())
    {
        QContact cnt;
        cnt = s->selectedContact();

        //get the user to choose the type of the number
        QPhoneTypeSelector *pts = new QPhoneTypeSelector( cnt, phoneNumber, this );
        pts->setModal(true);
        pts->showMaximized();
        if(pts->exec())
        {
            pts->updateContact(cnt, phoneNumber);
            mModel->updateContact( cnt );
        }
    }
}

void AddressbookWindow::setContactImage( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
    if (processingRequest.requestData().type().id() != "image/jpeg") {
        processingRequest.respond( "Invalid mimetype!" );
        return;
    }

    QContactSelector *s = new QContactSelector( false, this );

    // Create a different model to exclude sim contacts
    QContactModel *model = new QContactModel( this );
    QSet<QPimSource> sources = model->availableSources();
    sources.remove( model->simSource() );
    model->setVisibleSources( sources );

    if ( model->count() == 0 ) {
        QMessageBox::warning(
            this,
            tr( "Contacts" ),
            "<qt>" + tr( "No phone contacts available." ) + "</qt>",
            QMessageBox::Ok );

        processingRequest.respond( "No phone contacts available." );
    } else {
        s->setModel(model);
        s->showMaximized();
        if ( s->exec() )
        {
            QContact cnt = s->selectedContact();
            QIODevice *stream = processingRequest.requestData().toIODevice();
            QImage image;
            if (image.load(stream, "JPEG")) {
                QPixmap np = QPixmap::fromImage(image);
                cnt.changePortrait( np );
                mModel->updateContact( cnt );
            }
        }

        processingRequest.respond();
    }

    delete s;
    delete model;
}

void AddressbookWindow::setContactImage(const QString& filename)
{
    QContactSelector *s = new QContactSelector( false, this );
    QContactModel *model =new QContactModel(this); // different filtering from app view.
    // exlcude sim contacts
    QSet<QPimSource> sources = model->availableSources();
    sources.remove(model->simSource());
    model->setVisibleSources(sources);

    s->setModel(model);
    s->showMaximized();

    if(
        s->exec()
      )
    {
        QContact cnt = s->selectedContact();
        cnt.changePortrait( filename );
        model->updateContact( cnt );
    }
    delete s;
    delete model;
}

void AddressbookWindow::createNewContact(const QString& phoneNumber)
{
    QContact cnt;
    QPhoneTypeSelector *pts = new QPhoneTypeSelector( cnt, phoneNumber, this );
    pts->setModal(true);
    pts->showMaximized();
    if (pts->exec())
    {
        pts->updateContact(cnt, phoneNumber);
        newEntry(cnt);
    }
}

void AddressbookWindow::appMessage(const QString &msg, const QByteArray &data)
{
    /*@ \service Contacts
    This service provides simple operations on the addressbook that
    are usefully invoked from outside the application.
    */

    if ( msg == "receiveData(QString,QString)" )
    {
        QDataStream stream(data);
        QString f,t;
        stream >> f >> t;
        receiveFile(f, t);
    }
}

#ifdef QTOPIA_CELL
void AddressbookWindow::simInserted()
{
    if (!mGotSimEntries) {
        if( mHasSim )
        {
            /* See if the sim is in the list of visible sources.
               Since 4.3 has multiple sim sources under the sim context,
               we take a slightly convoluted approach, since we can't access
               the sim context directly.
               */
            QSet<QPimSource> vis = mModel->visibleSources();
            QUuid simContext = mModel->simSource().context;
            bool showLabel = false;
            foreach (QPimSource p, vis) {
                if (p.context == simContext) {
                    showLabel = true;
                    break;
                }
            }
            if (showLabel) {
                mShowSimLabel = true;
                mListView->showLoadLabel(true);
                if (mGroupMemberView)
                    mGroupMemberView->showLoadLabel(true);
            }
        }
    }
}

void AddressbookWindow::simNotInserted()
{
    mGotSimEntries = false;
    mShowSimLabel = false;
    mListView->showLoadLabel(false);
    if (mGroupMemberView)
        mGroupMemberView->showLoadLabel(false);
}

void AddressbookWindow::phoneBookUpdated(const QString& store)
{
    if ( store == "SM" ) {        // No tr
        mGotSimEntries = true;
        mShowSimLabel = false;
        mListView->showLoadLabel(false);
        if (mGroupMemberView)
            mGroupMemberView->showLoadLabel(false);
    }
}

void AddressbookWindow::vcardDatagram( const QByteArray& data )
{
    QList<QContact> vcardContacts;
    QString question;
    QContact newEntry;
    int n;

    vcardContacts = QContact::readVCard( data );

    if( vcardContacts.count() > 0 )
    {
        if( vcardContacts.count() == 1 )
        {
            question = tr("<qt>Contact \"%1\" received. Add to "
            "Address Book?").arg(vcardContacts[0].label());
        }
        else
        {
            question = tr("<qt>%1 contacts received. Add to Address"
            "Book?").arg(vcardContacts.count());
        }

        QMessageBox box( tr("Contacts"), question, QMessageBox::NoIcon,
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No, QMessageBox::NoButton, isVisible() ? this : 0,
             Qt::Dialog | Qt::WindowStaysOnTopHint);

        if(box.exec() == QMessageBox::Yes)
        {
            for(n = 0; n < vcardContacts.count(); n++)
            {
                newEntry = vcardContacts[n];

                NameLearner learner(newEntry);
                QUniqueId ui = mModel->addContact(newEntry);

                updateContextMenu();
                //abList->reload();
                //abList->setCurrentEntry(ui);
            }
        }
    }
}

#endif

void AddressbookWindow::groupList()
{
    if ( centralView->currentWidget() == mGroupMemberView )
        restoreViewState();
    else
        showGroupListView(true);
}

void AddressbookWindow::showPersonalView()
{
    saveViewState();

    setWindowTitle( tr("Contacts - My Personal Details") );

    QContact me = mModel->personalDetails();
    setCurrentContact( me );
    showDetailsView(false);
}

// hacky static bool
static bool newPersonal = false;

void AddressbookWindow::editPersonal()
{
    if (mModel->hasPersonalDetails())
    {
        QContact me = mModel->personalDetails();
        newPersonal = false;
        editEntry( me );
    }
    else
    {
        newPersonal = true;
        newEntry();
    }
    newPersonal = false;
}

void AddressbookWindow::markCurrentAsPersonal()
{
    QContact c = currentContact();

    if (c.uid().isNull()) {
        qWarning("ab::markCurrentAsPersonal called with no selection");
        return;
    }
    if (mModel->isPersonalDetails(c.uid())) {
        mModel->clearPersonalDetails();
        updateContextMenu();
    }
    else {
        if (QMessageBox::information(this, tr("Contacts"),
                    tr("<qt>Set \"%1\" as your Business Card?</qt>").arg( c.label() ),
                    tr("Yes"), tr("No"), 0, 0, 1) == 0)
        {
            mModel->setPersonalDetails(c.uid());
            updateContextMenu();
        }
    }
}

void AddressbookWindow::newEntry()
{
    newEntry( QContact() );
}

AbEditor *AddressbookWindow::editor(const QUniqueId &id)
{
#ifdef QTOPIA_CELL
    if (mModel->isSIMCardContact(id)) {
        if (!abSimEditor)
            abSimEditor = new AbSimEditor;
        return abSimEditor;
    }
    else
#endif
    {
        if (!abFullEditor)
            abFullEditor = new AbFullEditor;
        return abFullEditor;
    }
}

void AddressbookWindow::updateDependentAppointments(const QContact& src, AbEditor* editor)
{
    // Update the appointment, if it exists
    QUniqueId aId = src.dependentChildrenOfType("anniversary").value(0); // no tr
    QUniqueId bId = src.dependentChildrenOfType("birthday").value(0); // no tr
    if (!aId.isNull() || !bId.isNull()) {
        QAppointmentModel am;
        QAppointmentContext *context = qobject_cast<QAppointmentContext*>(am.context(aId));
        if (context != NULL) {
            // Copy the alarm stuff over
            QAppointment generated = am.appointment(aId);
            if (generated.alarmDelay() != editor->anniversaryReminderDelay() || generated.alarm() != editor->anniversaryReminder()) {
                generated.setAlarm(editor->anniversaryReminderDelay(), editor->anniversaryReminder());
                context->updateAppointment(generated);
            }
        }
        context = qobject_cast<QAppointmentContext*>(am.context(bId));
        if (context != NULL) {
            QAppointment generated = am.appointment(bId);
            if (generated.alarmDelay() != editor->birthdayReminderDelay() || generated.alarm() != editor->birthdayReminder()) {
                generated.setAlarm(editor->birthdayReminderDelay(), editor->birthdayReminder());
                context->updateAppointment(generated);
            }
        }
    }
}

void AddressbookWindow::newEntry( const QContact &cnt )
{

    QContact entry = cnt;
    AbEditor *abEditor = editor(cnt.uid());

    abEditor->setEntry( entry, true );

    if( abEditor->exec() == QDialog::Accepted )
    {
        setFocus();
        QContact newEntry(abEditor->entry());

        NameLearner learner(newEntry);
        QUniqueId ui = mModel->addContact(newEntry);
        newEntry.setUid(ui);
        if(newPersonal)
            mModel->setPersonalDetails(ui);
        updateDependentAppointments(newEntry, abEditor);
        setCurrentContact(newEntry);
        updateContextMenu();
    }
}

void AddressbookWindow::editEntry( const QContact &cnt )
{
    QContact entry = cnt;
    AbEditor *abEditor = editor(cnt.uid());

    abEditor->setEntry( entry, false);

    if( abEditor->exec() == QDialog::Accepted )
    {
        setFocus();
        QContact editedEntry( abEditor->entry() );

        if( !(entry == editedEntry)) {
            // only do update operations if the data has actually changed
            //regular contact, just update
            NameLearner learner(editedEntry);
            mModel->updateContact(editedEntry);
            updateSpeedDialEntries(editedEntry);
        }

        updateDependentAppointments(editedEntry, abEditor);
        setCurrentContact(editedEntry);

        if( mDetailsView && centralView->currentWidget() == mDetailsView)
        {
            // don't call slotDetailView because with QDL that would push the same
            // entry onto the view stack again. just refresh
            // need to get entry again for contact list model data.
            editedEntry = mModel->contact(editedEntry.uid());
            mDetailsView->init( editedEntry );
        }
    }
}

void AddressbookWindow::removeContactFromCurrentGroup()
{
    if (!checkSyncing()) {
        Q_ASSERT(!currentContact().uid().isNull());
        Q_ASSERT(mCurrentFilter.requiredCategories().count() == 1);
        Q_ASSERT(mGroupMemberView);

        QContact contact = currentContact();
        QString mId = mCurrentFilter.requiredCategories()[0];
        QStringList categories = currentContact().categories();

        categories.removeAll(mId);
        contact.setCategories(categories);
        mModel->updateContact(contact);
    }
}

void AddressbookWindow::setCurrentContact( const QContact &entry)
{
    mListView->setCurrentContact(entry);
    if (mGroupMemberView)
        mGroupMemberView->setCurrentContact(entry);
    mCurrentContactDirty = false;
    mCurrentContact = entry;
}

void AddressbookWindow::currentContactSelectionChanged()
{
    mCurrentContactDirty = true;
}

QContact AddressbookWindow::currentContact() const
{
    if (mCurrentContactDirty) {
        // update via list view.  reset
        QContact entry;
        if (centralView->currentWidget() == mListView)
            entry = mListView->currentContact();
        else if (centralView->currentWidget() == mGroupMemberView)
            entry = mGroupMemberView->currentContact();
        else if (centralView->currentWidget() == mDetailsView)
            entry = mDetailsView->entry();
        else if (centralView->currentWidget() == mGroupsListView) // just use the list view contact
            entry = mListView->currentContact();
        mCurrentContactDirty = false;
        mCurrentContact = entry;
        mListView->setCurrentContact(entry);
        if (mGroupMemberView)
            mGroupMemberView->setCurrentContact(entry);
    }
    return mCurrentContact;
}

void AddressbookWindow::saveViewState()
{
    QWidget *w = centralView->currentWidget();
    if (w) {
        AB_State state;
        state.contact = currentContact();
        if (mListView == w) {
            state.pane = AB_State::List;
        } else if (mGroupsListView == w) {
            state.pane = AB_State::Groups;
        } else if (mDetailsView == w) {
            state.pane = AB_State::Details;
        } else if (mGroupMemberView == w) {
            state.pane = AB_State::GroupMembers;
        } else {
            state.pane = AB_State::List;
        }
        mContactViewStack.append(state);
    }
}

void AddressbookWindow::restoreViewState()
{
    if (mContactViewStack.count() > 0) {
        AB_State prevState = mContactViewStack.last();
        mContactViewStack.removeLast();
        setCurrentContact(prevState.contact); // XXX - if we pop a group member list, select a contact, and go back to all contacts, we lose the selected contact

        if ( prevState.contact.uid().isNull() ) {
             showListView(false);
             mContactViewStack.clear();
        } else {
            switch (prevState.pane) {
                case AB_State::List:
                    showListView(false);
                    break;
                case AB_State::Groups:
                    showGroupListView(false);
                    break;
                case AB_State::GroupMembers:
                    showGroupMemberView(false);
                    break;
                case AB_State::Details:
                    showDetailsView(false);
                    break;
            }
        }
    } else if( mCloseAfterView ) {
        mCloseAfterView = false;
        showListView(false);
        close();
    } else
        showListView(false);
}

void AddressbookWindow::showCategory( const QString & s)
{
    showCategory(QCategoryFilter(s), true);
}

void AddressbookWindow::showCategory( const QCategoryFilter &c, bool saveState )
{
    if (saveState)
        saveViewState();

    // and filter
    mCurrentFilter = c;
    mModel->setCategoryFilter(c);
    showGroupMemberView(false);

    updateGroupNavigation();
}

void AddressbookWindow::configure()
{
    AbDisplaySettings settings(this);
    settings.setModal(true);
    if(QtopiaApplication::execDialog(&settings) == QDialog::Accepted)
    {
        settings.saveFormat();
        QContact::setLabelFormat( settings.format() );
    }
}

#ifdef QTOPIA_CELL
void AddressbookWindow::smsBusinessCard()
{
    // Find a suitable QDS service provider
    QDSServices services( QString( "text/x-vcard" ),
                          QString( "" ),
                          QStringList( QString( "sms" ) ) );

    if ( services.count() == 0 ) {
        close();
        return;
    }

    // 160 chars limitation for a sms
    // business numbers and home mobile will be transmitted
    QContact my((const QContact &)mModel->personalDetails());
    QContact bCard;
    bCard.setFirstName(my.firstName());
    bCard.setLastName(my.lastName());
    bCard.setBusinessPhone(my.businessPhone());
    bCard.setBusinessFax(my.businessFax());
    bCard.setBusinessMobile(my.businessMobile());
    bCard.setHomeMobile(my.homeMobile());

    QByteArray vCardArray;
    {
        QDataStream stream( &vCardArray, QIODevice::WriteOnly );
        bCard.writeVCard( &stream );
    }
    QDSData vCardData( vCardArray, QMimeType( "text/x-vcard" ) );

    // Create the action and make the request
    QDSAction action( services[0] );
    if ( action.exec( vCardData ) != QDSAction::Complete )
        qWarning( "SMS business card did not occur as expected" );

    // Finish up
    close();
}
#endif // QTOPIA_CELL

void AddressbookWindow::selectSources()
{
    ContactSourceDialog diag(this);
    diag.setPimModel(mModel);
    diag.showMaximized();

    if (QtopiaApplication::execDialog(&diag)) {
        QSet<QPimSource> set = mModel->visibleSources();
        QSettings cfg( "Trolltech", "Contacts" );
        cfg.beginGroup( "default" );
        cfg.beginWriteArray("SelectedSources", set.count());
        int i = 0;
        foreach(QPimSource s, set) {
            cfg.setArrayIndex(i++);
            cfg.setValue("context", s.context.toString());
            cfg.setValue("identity", s.identity);
        }
        cfg.endArray();
    }
}

void AddressbookWindow::importAllFromSim()
{
    // won't work for 4.2, contexts id's have changed.
    // possibly error checking would be required.
    mModel->mirrorAll(mModel->simSource(), mModel->defaultSource());
}

void AddressbookWindow::exportAllToSim()
{
    // won't work for 4.2, contexts id's have changed.
    // possibly error checking would be required.
    if (!mModel->mirrorAll(mModel->defaultSource(), mModel->simSource())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not export contacts to SIM Card.  Please ensure sufficient"
                    " space is available on SIM Card.</qt>"));
    }
}

void AddressbookWindow::importCurrentFromSim()
{
    if (!mModel->mirrorToSource(mModel->defaultSource(), currentContact().uid())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not import contact.</qt>"));
    }
}

void AddressbookWindow::exportCurrentToSim()
{
    if (!mModel->mirrorToSource(mModel->simSource(), currentContact().uid())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not export contact to SIM Card.  Please ensure sufficient"
                    " space is available on SIM Card for all numbers stored in contact.</qt>"));
    }
}

void AddressbookWindow::qdlActivateLink( const QDSActionRequest& request )
{
    // Grab the link from the request and check that is one of ours
    QDLLink link( request.requestData() );
    if ( link.service() != "Contacts" ) {
        QDSActionRequest( request ).respond( "Link doesn't belong to Contacts" );
        return;
    }

    const QByteArray data = link.data();
    QDataStream refStream( data );
    QUniqueId contactId;
    refStream >> contactId;
    if ( mModel->exists( contactId ) ) {
        showJustItem( contactId );
        showMaximized();
        activateWindow();
        raise();
        QDSActionRequest( request ).respond();
    } else {
        QMessageBox::warning(
            this,
            tr("Contacts"),
            "<qt>" + tr("The selected contact no longer exists.") + "</qt>" );
        QDSActionRequest( request ).respond( "Contact doesn't exist" );
    }
}

void AddressbookWindow::qdlRequestLinks( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
    if ( mModel->count() == 0 ) {
        QMessageBox::warning(
            this,
            tr( "Contacts" ),
            "<qt>" + tr( "No contacts available." ) + "</qt>",
            QMessageBox::Ok );

        processingRequest.respond( "No contacts available." );

        return;
    }

    QContactSelector *s = new QContactSelector( false, ( isVisible() ? this : 0 ) );
    s->setModal( true );
    s->setModel(mModel);
    s->showMaximized();

    if ( ( s->exec() == QDialog::Accepted ) && ( s->contactSelected() ) ) {
#ifndef QT_NO_QCOP
        QContact contact = s->selectedContact();
        QList<QDSData> links;
        links.push_back( contactQDLLink( contact ) );

        QByteArray array;
        {
            QDataStream ds( &array, QIODevice::WriteOnly );
            ds << links;
        }

        processingRequest.respond( QDSData( array, QDLLink::listMimeType() ) );
#endif
    }
    delete s;
}

QDSData AddressbookWindow::contactQDLLink( const QContact& c )
{
    if ( c == QContact() )
        return QDSData();

    QContact contact(c);

    // Check if we need to create the QDLLink
    QString keyString = contact.customField( QDL::SOURCE_DATA_KEY );

    if ( keyString.isEmpty() ||
         !QDSData( QUniqueId( keyString ) ).isValid() )
    {
        QByteArray dataRef;
        QDataStream refStream( &dataRef, QIODevice::WriteOnly );
        refStream << contact.uid();

        QDLLink link( "Contacts",
                      dataRef,
                      contact.label(),
                      "pics/addressbook/AddressBook" );

        QDSData linkData = link.toQDSData();
        QUniqueId key = linkData.store();
        contact.setCustomField( QDL::SOURCE_DATA_KEY, key.toString() );
        QString keyString2 = contact.customField( QDL::SOURCE_DATA_KEY );

        mModel->updateContact( contact );

        return linkData;
    }

    // Get the link from the QDSDataStore
    return QDSData( QUniqueId( keyString ) );
}

void AddressbookWindow::removeContactQDLLink( const QContact& c )
{
    if ( c == QContact() )
        return;

    QContact contact(c);

    // Release any client QDLLinks
    QString links = contact.customField( QDL::CLIENT_DATA_KEY );
    if ( !links.isEmpty() ) {
        QDL::releaseLinks( links );
    }

    // Check if the QDLLink is stored
    QString key = contact.customField( QDL::SOURCE_DATA_KEY );
    if ( !key.isEmpty() ) {
        // Break the link in the QDSDataStore
        QDSData linkData = QDSData( QUniqueId( key ) );
        QDLLink link( linkData );
        link.setBroken( true );
        linkData.modify( link.toQDSData().data() );

        // Now remove our reference to the link data
        linkData.remove();

        // Finally remove the stored key
        contact.removeCustomField( QDL::SOURCE_DATA_KEY );
        mModel->updateContact( contact );
    }
}

/***************************
  * AbSourcesDialog
  ***********************/

//  FIXME: This dialog is designed for phones to select between SIM
//          and PHONE contacts. If there are more sources to be added later,
//          this dialog will need to be reworked.
AbSourcesDialog::AbSourcesDialog(QWidget *parent, const QSet<QPimSource> &availSources)
    : QDialog(parent)
{
    layout = new QVBoxLayout(this);

    availableSources = availSources;

    setWindowTitle(tr("Show Contacts From", "e.g. Show Contacts From Phone/SIM Card"));
    bg = new QButtonGroup(0);

    phoneButton = new QRadioButton(tr("Phone"));
    bg->addButton(phoneButton);
    layout->addWidget(phoneButton);

    simButton = new QRadioButton(tr("SIM Card"));
    bg->addButton(simButton);
    layout->addWidget(simButton);

    bothButton = new QRadioButton(tr("Both Phone and SIM"));
    bg->addButton(bothButton);
    layout->addWidget(bothButton);

    QMenu* contextMenu = QSoftMenuBar::menuFor(this);

    QAction *actionCopyFromSim = new QAction(QIcon(":icon/sync"), tr("Import from SIM"), this);
    actionCopyFromSim->setWhatsThis(tr("Copy all entries from the SIM card to the Phone"));
    connect(actionCopyFromSim, SIGNAL(triggered()), this, SIGNAL(importFromSimTriggered()));

    QAction *actionCopyToSim = new QAction(QIcon(":icon/sync"), tr("Export to SIM"), this);
    actionCopyToSim->setWhatsThis(tr("<qt>Copy all currently shown entries from the Phone to the SIM card. "
                "Entries not shown due to category filtering will not be copied.</qt>"));
    connect(actionCopyToSim, SIGNAL(triggered()), this, SIGNAL(exportToSimTriggered()));

    contextMenu->addAction(actionCopyFromSim);
    contextMenu->addAction(actionCopyToSim);
}

void AbSourcesDialog::setSources(const QSet<QPimSource> &sources)
{
    QPimSource simSource;
    bool hasSim = false;
    bool hasPhone = false;
    foreach (QPimSource s, sources) {
        if (s.context == QUuid("b63abe6f-36bd-4bb8-9c27-ece5436a5130"))
            hasSim = true;
        else
            hasPhone = true;
    }

    if(hasSim && hasPhone)
        bothButton->setChecked(true);
    else if(hasSim)
        simButton->setChecked(true);
    else
        phoneButton->setChecked(true);
}

QSet<QPimSource> AbSourcesDialog::sources() const
{
    QAbstractButton *selected = bg->checkedButton();
    if(selected == bothButton)
        return availableSources;

    bool sim = selected == simButton;

    QSet<QPimSource> result;
    foreach(QPimSource s, availableSources) {
        if (s.context == QUuid("b63abe6f-36bd-4bb8-9c27-ece5436a5130")) {
            if (sim)
                result.insert(s);
        } else {
            if (!sim)
                result.insert(s);
        }
    }

    return result;
}

void AbSourcesDialog::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
        case Qt::Key_Back:
            accept();
            break;
    }
}

/***************************
  * AbDisplaySettings
  ***********************/

AbDisplaySettings::AbDisplaySettings(QWidget *parent)
    : QDialog(parent)
{
    layout = new QVBoxLayout(this);

    QTranslatableSettings cfg( "Trolltech", "Contacts" );
    cfg.beginGroup( "formatting" );
    int curfmt = cfg.value( "NameFormat" ).toInt();

    setWindowTitle(tr("Display Settings"));
    bg = new QButtonGroup(0);

    for (int i = 0; ; ++i)
    {
        QString desc = cfg.value("NameFormatName"+QString::number(i)).toString();
        if ( desc.isEmpty() )
            break;

        QRadioButton* rb = new QRadioButton(desc, 0);
        bg->addButton(rb);
        layout->addWidget(rb);
        if (curfmt == i)
            rb->setChecked(true);
    }
}

void AbDisplaySettings::saveFormat()
{
    QAbstractButton* button = bg->checkedButton();
    if(button)
    {
        int index = layout->indexOf(button);
        if(index > -1)
        {
            QSettings cfg("Trolltech","Contacts");
            cfg.beginGroup("formatting");
            cfg.setValue("NameFormat", index);
        }
    }
}

void AbDisplaySettings::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
        case Qt::Key_Back:
            accept();
            break;
    }
}

QString AbDisplaySettings::format()
{
    QAbstractButton* button = bg->checkedButton();
    if(button)
    {
        int index = layout->indexOf(button);
        if(index > -1) {
            QSettings cfg("Trolltech","Contacts");
            cfg.beginGroup("formatting");
            return cfg.value("NameFormatFormat"+QString::number(index)).toString();
        }
    }

    return QString();
}

/*!
    \service ContactsService Contacts
    \brief Provides the Qtopia Contacts service.

    The \i Contacts service enables applications to access features of
    the Contacts application.
*/

/*!
    \internal
*/
ContactsService::~ContactsService()
{
}

/*!
    Invokes a dialog with the user, allowing them to enter their
    personal details as a Contact entry.

    This slot corresponds to the QCop service message
    \c{Contacts::editPersonal()}.
*/
void ContactsService::editPersonal()
{
    parent->editPersonal();
}

/*!
    Invokes a dialog with the user, allowing them to enter their
    personal details as a Contact entry.  The main application window
    will be closed, with only the dialog displayed.

    This slot corresponds to the QCop service message
    \c{Contacts::editPersonalAndClose()}.
*/
void ContactsService::editPersonalAndClose()
{
    parent->editPersonal();
    parent->close();
}

/*!
    Adds a \a contact.

    This slot corresponds to the QCop service message
    \c{Contacts::addContact(QContact)}.
*/
void ContactsService::addContact(const QContact& contact)
{
    QContact c(contact);
    NameLearner learner(c);
    parent->mModel->addContact(c);
}

/*!
    Removes a \a contact.

    This slot corresponds to the QCop service message
    \c{Contacts::removeContact(QContact)}.
*/
void ContactsService::removeContact(const QContact& contact)
{
    QContact cnt( contact );
    parent->removeContactQDLLink( cnt );
    parent->mModel->removeContact( contact );
    parent->removeSpeedDialEntries(contact);
}

/*!
    Updates a \a contact.

    This slot corresponds to the QCop service message
    \c{Contacts::updateContact(QContact)}.
*/
void ContactsService::updateContact(const QContact& contact)
{
    QContact c(contact);
    NameLearner learner(c);
    parent->mModel->updateContact(c);
    parent->updateSpeedDialEntries(c);
}

/*!
    Adds a \a contact, and invokes a dialog with the user to then
    edit the contact, for example to fill in any additional fields
    not supplied by the caller.

    This slot corresponds to the QCop service message
    \c{Contacts::addAndEditContact(QContact)}.
*/
void ContactsService::addAndEditContact(const QContact& contact)
{
    parent->newEntry( contact );
}

/*!
    Prompts the user to assign the given \a phoneNumber to a contact of
    their choice, or to a new contact they then create.  The phone number
    might for example have been supplied by phone network
    calling-number-identification.

    This slot corresponds to the QCop service message
    \c{Contacts::addPhoneNumberToContact(QString)}.
*/
void ContactsService::addPhoneNumberToContact(const QString& phoneNumber)
{
    parent->addPhoneNumberToContact(phoneNumber);
}

/*!
    Prompts the user to create a new contact with given \a phoneNumber.
    The user will need to select a phone type for \a phoneNumber first
    and then edit the new contact.

    This slot corresponds to the QCop service message
    \c{Contacts::createNewContact(QString)}.
*/
void ContactsService::createNewContact(const QString& phoneNumber)
{
    parent->createNewContact(phoneNumber);
}

/*!
    Shows details of the contact identified by \a uid to the user.

    This slot corresponds to the QCop service message
    \c{Contacts::showContact(QUniqueId)}.
*/
void ContactsService::showContact(const QUniqueId& uid)
{
    parent->showJustItem(uid);
    parent->raise();
}

/*!
    Prompts the user to assign the image in \a filename to a contact of their
    choice.  The image might, for example, have been recently taken by
    a built-in camera.

    The file is owned by the application that sent it.  The Contacts
    application will make a copy if it could later delete the image.

    This slot corresponds to the QCop service message
    \c{Contacts::setContactImage(QString)}.
*/
void ContactsService::setContactImage(const QString& filename)
{
    parent->setContactImage(filename);
}

/*!
    Prompts the user to assign the image in \a request to a contact of their
    choice.  The image might, for example, have been recently taken by
    a built-in camera. The Contacts application will make a copy of the image.

    This slot corresponds to a QDS service with a request data type of
    "image/x-qpixmap" and no response data.

    This slot corresponds to the QCop service message
    \c{Contacts::setContactImage(QDSActionRequest)}.

*/
void ContactsService::setContactImage( const QDSActionRequest& request )
{
    parent->setContactImage( request );
}

/*!
    Activate the QDL link contained within \a request.

    This slot corresponds to a QDS service with a request data type of
    QDLLink::mimeType() and no response data.

    This slot corresponds to the QCop service message
    \c{Contacts::activateLink(QDSActionRequest)}.
*/
void ContactsService::activateLink( const QDSActionRequest& request )
{
    parent->qdlActivateLink( request );
}

/*!
    Request for one or more QDL links using the hint contained within
    \a request.

    This slot corresponds to a QDS service with a request data type of
    "text/x-qstring" and response data type of QDLLink::listMimeType().

    This slot corresponds to the QCop service message
    \c{Contacts::requestLinks(QDSActionRequest)}.

*/
void ContactsService::requestLinks( const QDSActionRequest& request )
{
    parent->qdlRequestLinks( request );
}

/*!
    \service ContactsPhoneService ContactsPhone
    \brief Provides the Qtopia ContactsPhone service.

    The \i ContactsPhone service enables applications to access phone-specific
    features of the Contacts application.
*/

/*!
    \internal
*/
ContactsPhoneService::~ContactsPhoneService()
{
}

#ifdef QTOPIA_CELL

/*!
    Sends the user's personal details, via SMS.

    This slot corresponds to the QCop service message
    \c{ContactsPhone::smsBusinessCard()}.
*/
void ContactsPhoneService::smsBusinessCard()
{
    if( parent->mModel->rowCount() == 0 || !parent->mModel->hasPersonalDetails()) {
        QMessageBox::warning( parent, tr("Contacts"), "<qt>"+tr("No business card set.")+"</qt");
    }
    else
    {
        parent->smsBusinessCard();
    }
}

/*!
    Receive a WAP push message \a request of type \c{text/x-vcard}.
*/
void ContactsPhoneService::pushVCard( const QDSActionRequest& request )
{
    parent->vcardDatagram( request.requestData().data() );
    QDSActionRequest( request ).respond();
}

#endif // QTOPIA_CELL
