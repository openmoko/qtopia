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

#include "abeditor.h"
#include "ablabel.h"
#include "addressbook.h"
#include "contactsource.h"

#include <qtopiasendvia.h>
#include <qtopiaapplication.h>
#include <qdawg.h>
#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactview.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopianamespace.h>
#include <qcategoryselector.h>
#include <qtopiaservices.h>
#include <qcategorymanager.h>
#include <qcontent.h>
#include <qthumbnail.h>

#ifdef QTOPIA_CELL
#include <qphonebook.h>
#endif

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <qtopialog.h>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QButtonGroup>
#include <QRadioButton>
#include <QStringList>
#include <QToolButton>
#include <QWhatsThis>
#include <QMessageBox>
#include <QPixmapCache>
#include <QLabel>
#include <QDateTime>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QDesktopWidget>
#include <QPainter>
#include <QImage>
#include <QListWidget>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QSettings>
#include <QKeyEvent>
#include <QTimer>
#include <QDSData>
#include <QDSActionRequest>
#include <QDSServices>
#include <QDSAction>
#include <QDLLink>
#include <QMetaType>

#ifdef QTOPIA_PHONE
# include <qtopia/qspeeddial.h>
# include <qtopia/qsoftmenubar.h>
#endif

#include <qtopiaipcenvelope.h>

#include <qdl.h>

#ifndef Q_OS_WIN32
#   include <unistd.h>
#endif

#define AB_CONTACTLIST  0
#define AB_DETAILVIEW   1

static bool constructorDone = false;
static const char* const SPEEDDIAL_DATA_KEY = "SpeedDial";

static const struct
{
    const char* desc;
    const char* fmt;
} fileas_opt[] =
        {
            {
                QT_TRANSLATE_NOOP("AbSettings","First Last or Company"),
                "firstname _ lastname | firstname | lastname | company"  // No tr
            },
            {
                QT_TRANSLATE_NOOP("AbSettings","Last, First or Company"),
                "lastname , _ firstname | lastname | firstname | company"  // No tr
            },
            { 0, 0 }
};

#ifdef QTOPIA_PHONE
class QLineEditWithPreeditNotification : public QLineEdit
{
public:
    QLineEditWithPreeditNotification(QWidget*& widgetIn) : QLineEdit(widgetIn){};
protected:
    bool eventFilter(QObject *o, QEvent *e);
};
#endif

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

class LoadIndicator : public QLabel
{
public:
    LoadIndicator( QWidget *parent = 0, Qt::WFlags fl = (Qt::Tool|Qt::WindowStaysOnTopHint) );
    void center();
    void bottom();

protected:
    void showEvent( QShowEvent *e );
    void paintEvent( QPaintEvent *e );
};



/***************************
  * AddressbookWindow
  ***********************/

AddressbookWindow::AddressbookWindow( QWidget *parent, Qt::WFlags f )
    : QMainWindow( parent, f ),
      mAllowCloseEvent( false ),
      mResetKeyChars( false ),
      mCloseAfterView( false ),
      mFindMode( false ),
      mHasSim( false ),
      abEditor(0),
      mView(0),
      searchBar(0),
      actionFind(0),
      actionPersonal(0),
      actionSetPersonal(0),
      actionResetPersonal(0),
      syncing(false),
      showingPersonal(false)
#ifdef QTOPIA_PHONE
      , actionSpeedDial(0)
#endif
{
    QtopiaApplication::loadTranslations( "libqtopiapim" ); //no tr
    contacts = new QContactModel(this);
    mHasSim = !contacts->simSource().isNull();
    // Make sure when the model changes we update the icons (for the
    // initial contacts cache load, for example)
    connect(contacts, SIGNAL(modelReset()), this, SLOT(contactsChanged()));

#ifdef QTOPIA_PHONE
    mFindLE = 0;
    mToggleInternal = false;
#endif

#ifdef QTOPIA_CELL
    mSimIndicator = 0;
    mGotSimEntries = false;

    QPhoneBook *phoneBook = new QPhoneBook( QString(), this );
    connect( phoneBook,
             SIGNAL(entries(const QString&, const QList<QPhoneBookEntry>&)),
             this, SLOT(phonebookChanged(QString)) );
#endif

    //
    //  Set up and populate the window
    //

    setWindowTitle( tr("Contacts") );
    setWindowIcon(QIcon(":image/AddressBook"));

    centralView = new QStackedWidget(this);
    setCentralWidget(centralView);

    //
    //  Build the main contact list view
    //

    listView = new QWidget(0);
    centralView->addWidget(listView);
    listViewLayout = new QVBoxLayout(listView);

    listViewLayout->setMargin(0);
    listViewLayout->setSpacing(0);

#ifdef QTOPIA_PHONE

    //
    //  Create a search box for fitler contacts. For touchscreeen it will be
    //  visible at all times, but in keypad it will only to visible when the user
    //  enters a filter string via the keypad
    //

    mFindLE = new QLineEditWithPreeditNotification(listView);
    listViewLayout->addWidget(mFindLE);
#endif

    //
    //  Create the contact list itself.
    //

    abList = new QContactListView(0);
    abList->setFrameStyle(QFrame::NoFrame);

    contactDelegate = new QContactDelegate(abList);
    abList->setItemDelegate(contactDelegate);

    abList->setModel(contacts);
    listViewLayout->addWidget(abList);
    abList->installEventFilter(this);

    abList->setSelectionMode(QListView::SingleSelection);
    connect(abList, SIGNAL(clicked(const QModelIndex &)),
         this, SLOT(selectClicked()));
    connect(abList->selectionModel(),
        SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
        this, SLOT(updateIcons()));
    abList->setWhatsThis( tr("List of contacts in the selected category.  Click to view"
        " detailed information.") );

#ifdef QTOPIA_PHONE

    //
    //  If this is a touch screen phone, tie the search box into the contact list.
    //
    connect( mFindLE,
             SIGNAL( textChanged( const QString& ) ),
             this,
             SLOT( search( const QString& ) ) );
    mFindLE->setVisible( Qtopia::mousePreferred() );

    if( Qtopia::mousePreferred() )
    {
        abList->setFocusPolicy( Qt::NoFocus );
        listView->setFocusPolicy( Qt::NoFocus );
        mFindLE->installEventFilter( mFindLE ); // to act on preedit text
        mFindLE->installEventFilter( this ); // to intercept keypress events
        QtopiaApplication::setInputMethodHint( mFindLE, QtopiaApplication::Text );
    }
    else
    {
        mFindLE->setFocusPolicy( Qt::NoFocus );
    }
#endif

    //
    //  Create the menu actions.
    //

    actionNew = new QAction(QIcon(":icon/new"), tr("New"), this);
    connect(actionNew, SIGNAL(triggered()), this, SLOT(slotListNew()));
    actionNew->setWhatsThis(tr("Enter a new contact."));

    actionEdit = new QAction(QIcon(":icon/edit"), tr("Edit"), this);
    actionEdit->setWhatsThis(tr("Edit the highlighted contact."));
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(slotViewEdit()));

    actionTrash = new QAction(QIcon(":icon/trash"), tr("Delete"), this);
    actionTrash->setWhatsThis(tr("Delete the selected contacts."));
    connect( actionTrash, SIGNAL(triggered()), this, SLOT(slotListDelete()));


    actionFind = new QAction(QIcon(":icon/find"), tr("Find"), this);
    actionFind->setWhatsThis(tr("Find a contact."));
    actionFind->setCheckable(true);
    connect(actionFind, SIGNAL(toggled(bool)), this, SLOT(slotFind(bool)));

    QIcon businessIcon(":icon/business");
    actionPersonal = new QAction(businessIcon, tr("Show My Card"), this);
    actionPersonal->setWhatsThis(tr("Show your business card entry."));
    connect(actionPersonal, SIGNAL(triggered()), this, SLOT(slotPersonalView()));

    actionSetPersonal = new QAction(businessIcon, tr("Set as My Card"), this);
    actionSetPersonal->setWhatsThis(tr("Set current entry as your business card."));
    connect(actionSetPersonal, SIGNAL(triggered()), this, SLOT(markCurrentAsPersonal()));

    actionResetPersonal = new QAction(businessIcon, tr("Remove as My Card"), this);
    actionResetPersonal->setWhatsThis(tr("Remove current entry as your business card."));
    connect(actionResetPersonal, SIGNAL(triggered()), this, SLOT(markCurrentAsPersonal()));

    actionImportSim = new QAction(QIcon(":icon/sync"), tr("Import to Phone"), this);
    actionImportSim->setWhatsThis(tr("Import current entry from SIM card to phone."));
    connect(actionImportSim, SIGNAL(triggered()), this, SLOT(importCurrentFromSim()));
    if (!mHasSim)
        actionImportSim->setVisible(false);

    actionExportSim = new QAction(QIcon(":icon/sync"), tr("Export to SIM"), this);
    actionExportSim->setWhatsThis(tr("Export current entry from phone to SIM card."));
    connect(actionExportSim, SIGNAL(triggered()), this, SLOT(exportCurrentToSim()));
    if (!mHasSim)
        actionExportSim->setVisible(false);

    actionSettings = new QAction(QIcon(":icon/settings"), tr("Display Settings..."), this);
    connect(actionSettings, SIGNAL(triggered()), this, SLOT(configure()));

#ifdef QTOPIA_PHONE
    QIcon viewCategoryIcon(":icon/viewcategory");
    actionShowSources = new QAction(viewCategoryIcon,
        tr("Show Contacts From...", "e.g. Show Contacts From Phone/SIM Card"), this);
    connect(actionShowSources, SIGNAL(triggered()), this, SLOT(selectSources()));
    if (!mHasSim)
        actionShowSources->setVisible(false);
#endif

#ifndef QTOPIA_PHONE
    actionEdit->setEnabled(false);
    actionTrash->setEnabled(false);
#endif

    QIcon beamIcon(":icon/beam");
    actionSend = new QAction(beamIcon, tr("Send..."), this);
    actionSend->setWhatsThis(tr("Send the contact to another person"));
    connect(actionSend, SIGNAL(triggered()), this, SLOT(sendContact()));

    actionSendCat = new QAction(beamIcon, tr("Send All..."), this);
    actionSendCat->setWhatsThis(tr("Send the visible contacts to another person"));
    connect(actionSendCat, SIGNAL(triggered()), this, SLOT(sendContactCat()));

#ifdef AB_PDA
    actionBack = new QAction(QIcon(":icon/i18n/back"), tr("Back"), this);
    connect(actionBack, SIGNAL(triggered()), this, SLOT(viewClosed()));
    actionBack->setEnabled(false);
#endif

#ifndef QTOPIA_PHONE

    //
    // Create Toolbars
    //

    QMenuBar *mbList = menuBar();
    QMenu *edit = mbList->addMenu(tr("Contact"));

    listTools = new QToolBar(this);
    listTools->setMovable(false);
    addToolBar(Qt::TopToolBarArea, listTools);

    edit->addAction(actionNew);
    edit->addAction(actionEdit);
    edit->addAction(actionTrash);

    listTools->addAction(actionNew);
    listTools->addAction(actionEdit);
    listTools->addAction(actionTrash);

    if( actionSend ) {
        edit->addAction(actionSend);
        edit->addAction(actionSendCat);
        listTools->addAction(actionSend);
    }

    edit->addAction(actionFind);
    listTools->addAction(actionFind);
# ifdef AB_PDA
    listTools->addSeparator();
    listTools->addAction(actionBack);
# endif

    edit->addSeparator();
# ifdef QTOPIA4_TODO
    edit->addAction(actionShowBCard);
    edit->addAction(actionSetBCard);
# endif

    catSelect = new QCategorySelector( "Address Book", QCategorySelector::Filter );
    listViewLayout->addWidget(catSelect);
    connect( catSelect, SIGNAL(filterSelected(const QCategoryFilter &)),
        this, SLOT(showCategory(const QCategoryFilter &)));
    catSelect->setWhatsThis( tr("Show contacts in this category.") );
#else

    //
    //  Create Context Menu
    //

    QMenu* contextMenu = QSoftMenuBar::menuFor(this);

    if(Qtopia::mousePreferred())
        QSoftMenuBar::addMenuTo(mFindLE, contextMenu);

    contextMenu->addAction(actionNew);

    if( actionSend )
        contextMenu->addAction(actionSendCat);

    actionSpeedDial = new QAction(QIcon(":icon/phone/speeddial"),
        tr("Add to Speed Dial..."), this);
    connect(actionSpeedDial, SIGNAL(triggered()), this, SLOT(addToSpeedDial()));

    actionCategory = new QAction(viewCategoryIcon,
        tr("View Category..."), this);
    connect(actionCategory, SIGNAL(triggered()), this, SLOT(selectCategory()));
    contextMenu->addAction(actionCategory);
    contextMenu->addAction(actionSettings);
    contextMenu->addAction(actionPersonal);
    contextMenu->addAction(actionShowSources);

    categoryLbl = new QLabel(listView);
    listViewLayout->addWidget(categoryLbl);
    categoryLbl->hide();
#endif

    readConfig();

    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
            this, SLOT(appMessage(const QString&,const QByteArray&)) );
    connect( qApp, SIGNAL(reload()), this, SLOT(reload()) );
    connect( qApp, SIGNAL(flush()), this, SLOT(flush()) );

    new ContactsService(this);

#ifdef QTOPIA_PHONE
    new ContactsPhoneService(this);
#endif

    constructorDone = true;
    updateIcons();

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
}

void AddressbookWindow::createViewMenu()
{
#ifdef QTOPIA_PHONE
    if(!mView)
        return;

    QMenu *viewMenu = QSoftMenuBar::menuFor(mView);

    viewMenu->addAction(actionEdit);
    viewMenu->addAction(actionTrash);

    if( actionSend )
        viewMenu->addAction(actionSend);

    viewMenu->addAction(actionSetPersonal);
    viewMenu->addAction(actionResetPersonal);
    viewMenu->addAction(actionSpeedDial);
    viewMenu->addAction(actionExportSim);
    viewMenu->addAction(actionImportSim);
#else
    //  TODO: PDA code
#warning FIXME viewMenu
#endif
}

void AddressbookWindow::createDetailedView()
{
    if(!mView)
    {
        mView = new AbLabel(0);
        connect(mView, SIGNAL(externalLinkActivated()), this, SLOT(close()));
        connect(mView, SIGNAL(highlighted(const QString&)), this,
            SLOT(setHighlightedLink(const QString&)));
        connect(mView, SIGNAL(backClicked()), this, SLOT(slotViewBack()));
        setHighlightedLink(QString());
        centralView->addWidget(mView);

#ifdef QTOPIA_PHONE
        createViewMenu();
#endif
    }
}

#ifdef QTOPIA_PHONE
bool QLineEditWithPreeditNotification::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    if ( e->type() == QEvent::InputMethod )
    {
        QInputMethodEvent* ime =  (QInputMethodEvent*)e;
        // emit textChanged()
        qLog(Input) << "AddressbookWindow::eventFilter() - mFindLE.textChanged(" << (this->text().insert(this->cursorPosition(),
                        ime->preeditString())) << ")";
        emit textChanged(this->text().
                insert(this->cursorPosition(), ime->preeditString()) );
    }
    return false;
};

bool AddressbookWindow::eventFilter( QObject *o, QEvent *e )
{
    if(o == abList)
    {
        if(e->type() == QEvent::KeyPress)
        {
            QKeyEvent *ke = (QKeyEvent *)e;
            switch( ke->key() )
            {
                case Qt::Key_Select:
                    selectClicked();
                    return true;
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                    return false;
                case Qt::Key_Back:
                    if ( mFindMode )
                    {
                        QString text = mFindLE->text();
                        text.truncate( text.count() - 1 );
                        mFindLE->setText( text );
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                default:
                    if (!ke->text().isEmpty()) {
                        mFindMode = true;
                        mFindLE->setText( mFindLE->text() + ke->text() );
                        return true;
                    }
                    return false;
            }
        }
    }
    else if( Qtopia::mousePreferred() )
    {
        if( o == mFindLE && e->type() == QEvent::KeyPress )
        {
            QKeyEvent *ke = (QKeyEvent *)e;
            switch( ke->key() )
            {
                case Qt::Key_Select:
                case Qt::Key_Left:
                case Qt::Key_Right:
                case Qt::Key_Up:
                case Qt::Key_Down:
                {
                    QtopiaApplication::postEvent( abList,
                    new QKeyEvent( QEvent::KeyPress, ke->key(), ke->modifiers() ) );
                    return true;
                }
          }
      }
    }
    return false;
}
#endif

/*
   Called when the ContactModel is reset (e.g. after refreshing
   cache from SIM or changing filter).  If we have any contacts, select the first,
   otherwise we show the "new" option.
*/
void AddressbookWindow::contactsChanged()
{
    if (!abList->currentIndex().isValid()) {
        QModelIndex newSel;
        if ( mView &&
            ( centralView->currentIndex() == AB_DETAILVIEW ) &&
            ( mView->entry() != QContact() ) )
                newSel = contacts->index(mView->entry().uid());
        if ( ! newSel.isValid())
            newSel = contacts->index(0,0);
        if(newSel.isValid())
        {
            abList->selectionModel()->setCurrentIndex(newSel, QItemSelectionModel::Current);
            abList->scrollTo(newSel, QAbstractItemView::PositionAtCenter);
        }
    }
    updateIcons();
}

void AddressbookWindow::sendContact()
{
    QContact cnt = abList->currentContact();
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
    if ( contacts->count() )
    {
        QByteArray vCardArray;
        {
            QDataStream stream( &vCardArray, QIODevice::WriteOnly );
            for( int i = 0; i < contacts->count(); i++ )
                contacts->contact(i).writeVCard( &stream );
        }

        QtopiaSendVia::sendData(this, vCardArray, "text/x-vcard");
    }
}

#ifdef QTOPIA_CELL
void AddressbookWindow::phonebookChanged( const QString& store )
{
    if ( store == "SM" ) {        // No tr
        mGotSimEntries = true;
        if(mSimIndicator)
            mSimIndicator->hide();
    }
}
#endif

void AddressbookWindow::keyPressEvent(QKeyEvent *e)
{
#ifdef QTOPIA_PHONE
    if ( (e->key() == Qt::Key_Back || e->key() == Qt::Key_No))
    {
        e->accept();
        if ( centralView->currentIndex() == AB_CONTACTLIST)
        {
            mAllowCloseEvent = true;
            close();
        }
        else
        {
            // slotListView();
            viewClosed();
        }
    }
    else if ( ( e->key() == Qt::Key_Call ) &&
              (centralView->currentIndex() == AB_CONTACTLIST) )
    {
        QContact cnt = abList->currentContact();
        if ( cnt != QContact() )
        {
            QMap<QContact::PhoneType, QString> numbers = cnt.phoneNumbers();
            if ( numbers.count() == 1 ) {
                QMap<QContact::PhoneType, QString>::const_iterator cit = numbers.constBegin();
                QtopiaServiceRequest req( "Dialer", "dial(QString,QUniqueId)" );
                req << cit.value();
                req << cnt.uid();
                req.send();
            }
            else {
                createDetailedView();
                mView->init( cnt );
                showView();
            }
        }
    }
    else
#endif
        QMainWindow::keyPressEvent(e);
}

void AddressbookWindow::showEvent( QShowEvent *e )
{
    QMainWindow::showEvent( e );

#ifdef QTOPIA_CELL
    if( !mGotSimEntries )
    {
        if( !mSimIndicator )
        {
            mSimIndicator = new QLabel(tr("Loading SIM..."), 0);
            listViewLayout->addWidget(mSimIndicator);
            mSimIndicator->setAlignment(Qt::AlignCenter);
        }
        mSimIndicator->show();
    }
#endif

#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() )
        mFindLE->setFocus();
#endif
}

void AddressbookWindow::setDocument( const QString &filename )
{
    receiveFile(filename);
}

LoadIndicator::LoadIndicator( QWidget *parent, Qt::WFlags fl )
    : QLabel( parent, fl )
{
    setAttribute(Qt::WA_PaintUnclipped);
    setAlignment( Qt::AlignCenter );
}

void LoadIndicator::center()
{
    QRect rect;
    if (parentWidget())
        rect = parentWidget()->geometry();
    else
        rect = QApplication::desktop()->availableGeometry();

    QSize s = 2 * sizeHint();
    s.setWidth( qMin(s.width(), rect.width()) );
    QPoint pp( rect.x() + rect.width()/2 - s.width()/2,
                rect.y() + rect.height()/ 2 - s.height()/2 );

    setGeometry( QRect(pp, s) );
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
    QLabel::showEvent( e );
}

void LoadIndicator::paintEvent( QPaintEvent *e )
{
    QPainter p( this );
    p.fillRect( 0, 0, width(), height(), palette().background() );
    p.end();
    QLabel::paintEvent( e );
}

void AddressbookWindow::receiveFile( const QString &filename, const QString &mimetype )
{
    QString targetFile = filename;

    if(mimetype.isEmpty())
    {
        if ( filename.right(8) == ".desktop" ) {
            QContent lnk(filename);
            if ( !lnk.isValid() || lnk.type().toLower() != "text/x-vcard" )
                return;
            targetFile = lnk.file();
        } else if ( targetFile.right(4) != ".vcf" ) {
            return;
        }
    }
    else if(mimetype.toLower() != "text/x-vcard")
        return;

    QWidget *mbParent = isVisible() ? this : 0;

    LoadIndicator *li = new LoadIndicator( 0 );
    li->setText( tr("Reading VCards..") );
    li->center();
    li->show();
    li->repaint();

    QList<QContact> cl = QContact::readVCard( targetFile );
    delete li;

    if ( cl.count() == 0 ) {
        QMessageBox::warning(mbParent, tr("Invalid VCard"),
            tr("<qt>The VCard document did not contain any valid VCards</qt>") );
        return;
    }

    QList<QContact> newContacts, oldContacts;
    for( QList<QContact>::Iterator it = cl.begin(); it != cl.end(); ++it ) {
        QContact c = *it;
        if ( contacts->contains( c.uid() ) ) {
            oldContacts.append( c );
        } else newContacts.append( c );
    }

    if ( newContacts.count() > 0 ) {
        QString list = "";
        uint count = 0;
        for ( QList<QContact>::Iterator it = newContacts.begin(); it != newContacts.end(); ++it) {
            if ( count < 3 ) {
                list += (*it).label() + "\n";
            } else if ( count == 3 ) {
                list += "...\n";
            }
            count++;
        }

        QString msg = tr("<qt>%1 new VCard(s) for %2<br>Do you want to add them to your addressbook?</qt>").arg( newContacts.count() ).arg(list);
        if ( QMessageBox::information(mbParent, tr("New contact(s)"), msg, QMessageBox::Ok, QMessageBox::Cancel) ==
            QMessageBox::Ok )
        {
            NameLearner namelearner;

            for ( QList<QContact>::Iterator it = newContacts.begin(); it != newContacts.end(); ++it)
            {
                //if we have a photo for this contact, save it to disk
                QContact &curCon = *it;
                QString encDataStr( curCon.customField( "phototmp" ) );
                if( !encDataStr.isEmpty() )
                {
                    QByteArray encData = QByteArray( encDataStr.toLatin1(), encDataStr.length() );
                    QByteArray decData = QByteArray::fromBase64(encData);

                    QPixmap np( (const char *)decData );
                    if( !np.isNull() )
                    {
                        curCon.changePortrait( np );
                    }
                    curCon.changePortrait( QPixmap() );
                }

                namelearner.learn(*it);
                contacts->addContact(*it);
            }
        }
    } else if ( oldContacts.count() > 0 ) {
        QString list = "";
        uint count = 0;
        for ( QList<QContact>::Iterator it = oldContacts.begin(); it != oldContacts.end(); ++it) {
            if ( count < 3 ) {
                list += (*it).label() + "\n";
            } else if ( count == 3 ) {
                list += "...\n";
            }
            count++;
        }

        QString msg = tr("<qt>%1 old VCard(s) for %2<br>The document only contained VCards already in your addressbook</qt>").arg( oldContacts.count() ).arg(list);
        QMessageBox::information(mbParent, tr("Contact(s) already registered"), msg, QMessageBox::Ok);

    }
}

void AddressbookWindow::flush()
{
    contacts->flush();
    syncing = true;
}

void AddressbookWindow::reload()
{
    syncing = false;
    //Force a reload here
    if ( centralView->currentIndex() == AB_DETAILVIEW)
    {
        createDetailedView();
        mView->init( abList->currentContact() );
    }
    updateIcons();
}

void AddressbookWindow::resizeEvent( QResizeEvent *e )
{
    QMainWindow::resizeEvent( e );

    // we receive a resize event from qtable in the middle of the constrution, since
    // QTable::columnwidth does qApp->processEvents.  Ignore this event as it causes
    // all sorts of init problems for us
    if ( !constructorDone )
        return;

    if ( centralView->currentIndex() == AB_CONTACTLIST)
        slotListView();
    else if ( centralView->currentIndex() == AB_DETAILVIEW)
        showView();
}

AddressbookWindow::~AddressbookWindow()
{
    if( !contacts->flush() )
        QMessageBox::information( this, tr( "Contacts" ),
                tr("<qt>Device full.  Some changes may not be saved.</qt>"));
    writeConfig();
    if(abEditor)
        delete abEditor;
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
        contacts->setVisibleSources(set);

    QCategoryFilter f;
    f.readConfig( config, "SelectedCategory" );
    showCategory( f );
    config.endGroup();
}

void AddressbookWindow::writeConfig(void)
{
    QSettings config( "Trolltech", "Contacts" );

    config.beginGroup( "default" );
    QCategoryFilter f = contacts->categoryFilter();
    f.writeConfig( config, "SelectedCategory" );
    config.endGroup();
}

void AddressbookWindow::updateIcons()
{
    bool details = centralView->currentIndex() == AB_DETAILVIEW;
    bool selected = abList->currentIndex().isValid();

    actionSettings->setEnabled(!details);

#ifdef QTOPIA_PHONE
//    actionSpeedDial->setVisible(details && (!sel_href.isNull() && !sel_href.startsWith("QDL")));
    actionSpeedDial->setVisible(details && ( !sel_href.isNull() && !sel_href.startsWith("QDL")));
    actionTrash->setVisible(details);
    actionEdit->setVisible(details);

    if (mHasSim) {
        if (contacts->isSIMCardContact(mCurrentContact.uid())) {
            actionExportSim->setVisible(false);
            actionImportSim->setVisible(true);
        } else {
            actionExportSim->setVisible(true);
            actionImportSim->setVisible(false);
        }
    }

    if (contacts->isPersonalDetails(mCurrentContact.uid())) {
        if (actionResetPersonal)
            actionResetPersonal->setVisible(details);
        if (actionSetPersonal)
            actionSetPersonal->setVisible(false);
    } else {
        if (actionSetPersonal)
            actionSetPersonal->setVisible(details);
        if (actionResetPersonal)
            actionResetPersonal->setVisible(false);
    }

    if(actionSend)
        actionSend->setEnabled(details);

    if(actionSendCat)
        actionSendCat->setEnabled(contacts->count() && !details);

    if(selected)
        QSoftMenuBar::setLabel( abList, Qt::Key_Select, QSoftMenuBar::View);
    else
        QSoftMenuBar::setLabel( abList, Qt::Key_Select, "new" /* <- icon filename */, tr("New") );

    if(actionPersonal)
        actionPersonal->setVisible(contacts->hasPersonalDetails());
#else
# ifdef AB_PDA
    actionBack->setEnabled( details );
# endif

    bool singlesel = selected && (abList->selectedContacts().count() == 1);

    actionTrash->setEnabled(selected);
    actionEdit->setEnabled(selected);
    if(actionSetPersonal)
        actionSetPersonal->setEnabled(singlesel);

    if(actionSend)
        actionSend->setEnabled(selected);

    if(actionSendCat)
        actionSendCat->setEnabled(!details);

    if(actionPersonal)
        actionPersonal->setEnabled(contacts->hasPersonalDetails());
#endif
}

void AddressbookWindow::showView()
{
    listView->hide();
    centralView->setCurrentIndex(1);
    createDetailedView();
    mView->show();
    mView->setFocus();
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() )
        mView->setEditFocus( true );
#endif
    updateIcons();
    setWindowTitle( tr("Contact Details") );
#ifdef QTOPIA_PHONE
    actionNew->setEnabled(false);
    actionCategory->setEnabled(false);
#endif
}

bool AddressbookWindow::checkSyncing()
{
    if (syncing) {
        if ( QMessageBox::warning(this, tr("Contacts"),
                             tr("<qt>Can not edit data, currently syncing</qt>"),
                            QMessageBox::Ok, QMessageBox::Abort ) == QMessageBox::Abort )
        {
            // Okay, if you say so (eg. Qtopia Desktop may have crashed)....
            syncing = false;
        } else
            return true;
    }
    return false;
}


void AddressbookWindow::slotListNew()
{
    QContact cnt;
    if(!checkSyncing())
    {
        createDetailedView();
        mView->init(cnt);
        newEntry(cnt);
    }
}

void AddressbookWindow::selectClicked()
{
    if( abList->currentIndex().isValid() )
        slotDetailView();
    else
        slotListNew();
}

void AddressbookWindow::slotDetailView()
{
    if ( abList->currentIndex().isValid() ) {
        if (actionFind && actionFind->isChecked())
            actionFind->setChecked(false);
        QContact curEntry = abList->currentContact();
        viewOpened( curEntry );
        createDetailedView();
        mView->init( curEntry );
        setHighlightedLink( mView->encodeHref() );
        showView();
        mResetKeyChars = true;
#ifdef QTOPIA_PHONE
        if ( mFindMode )
            mFindLE->setText( QString() );
#endif
    }
}

void AddressbookWindow::slotListView()
{
    mCurrentContact = QContact();
#ifdef QTOPIA_PHONE
    abList->contactModel()->contact(abList->currentIndex());

    if( mResetKeyChars )
    {
        //previous view was something else
        mResetKeyChars = false;
        // XXX abList->setKeyChars( "" );
    }
#endif

    if ( centralView->currentIndex() != AB_CONTACTLIST ) {
        centralView->setCurrentIndex(0);
#ifdef QTOPIA_PHONE
        if( Qtopia::mousePreferred() )
            mFindLE->setFocus();
        else
            abList->setFocus();
#endif

        setWindowTitle( tr("Contacts") );
    }
    updateIcons();

#ifdef QTOPIA_PHONE
    actionNew->setEnabled(true);
    actionCategory->setEnabled(true);
#endif
}

void AddressbookWindow::setHighlightedLink(const QString& l)
{
#ifdef QTOPIA_PHONE
    sel_href = l;

    //
    //  Show speed dial option if link is neither null, nor a qdl link.
    //
    if(actionSpeedDial) {
        actionSpeedDial->setVisible( !l.isNull() && !l.startsWith("QDL:") );
    }
#else
    Q_UNUSED(l);
#endif
}

void AddressbookWindow::addToSpeedDial()
{
#ifdef QTOPIA_PHONE
    if ( !sel_href.isNull() ) {
        QtopiaServiceRequest req;
        QString pm;
        createDetailedView();
        mView->decodeHref(sel_href, &req, &pm);
        QString speedDialInput = QSpeedDial::addWithDialog( mCurrentContact.label(),
                                                            "addressbook/" + pm,
                                                            req,
                                                            this );
        if ( !speedDialInput.isEmpty() ) {
            QContact contact = mView->entry();
            contact.setCustomField( SPEEDDIAL_DATA_KEY, speedDialInput );
            contacts->updateContact( contact );
        }
    }
#endif
}

void AddressbookWindow::slotListDelete()
{
    if ( checkSyncing() )
        return;

    if ( false )
    {
#ifndef QTOPIA_PHONE
        QList<QUniqueId> t = abList->selectedContactIds();

        if ( !t.count() ) return;

        QString str;
        if ( t.count() > 1 )
        {
            str = tr("<qt>Are you sure you want to delete the %1 selected contacts?</qt>", "%1 = number of contacts").arg( t.count() );
        }
        else
        {
            // some duplicated code, but selected/current logic might change, so I'll leave it as it is
            QContact tmpEntry = abList->currentContact();
            QString strName = tmpEntry.label();
            if ( strName.isEmpty() ) {
                strName = tmpEntry.company();
                if ( strName.isEmpty() )
                    strName = tr("No Name");
            }

            str = tr("<qt>Are you sure you want to delete: <b>%1</b> ?</qt>").arg( strName );
        }

        switch( QMessageBox::warning( this, tr("Contacts"), str, tr("Yes"), tr("No") ) )
        {
            case 0:
            {
                contacts->removeList(t);
                updateIcons();
                slotListView();
            }
            break;
            case 1: break;
        }
#endif
    }
    else
    {
        QContact tmpEntry = abList->currentContact();

        // If we've just edited a contact we need to grab the contact object from the
        // detailed as the contact list will not be updated.
        if ( !abList->currentIndex().isValid() )
            if ( mView &&
                ( centralView->currentIndex() == AB_DETAILVIEW ) &&
                ( mView->entry() != QContact() ) )
                tmpEntry = mView->entry();

        // get a name, do the best we can...
        QString strName = tmpEntry.label();
        if ( strName.isEmpty() ) {
            strName = tmpEntry.company();
            if ( strName.isEmpty() )
                strName = tr("No Name");
        }

        if ( Qtopia::confirmDelete( this, tr( "Contacts" ), strName ) ) {
            removeContactQDLLink( tmpEntry );
            removeSpeedDial( tmpEntry );
            contacts->removeContact( tmpEntry );
            abList->clearSelection();
            if ( centralView->currentIndex() == AB_CONTACTLIST)
                viewClosed();
            else
                slotListView();
        }
    }
}

void AddressbookWindow::slotViewBack()
{
    if (mCloseAfterView) {
        mCloseAfterView = false;
        mAllowCloseEvent = true;
        close();
    } else {
        viewClosed();
    }
}

void AddressbookWindow::slotViewEdit()
{
    if(!checkSyncing()) {
        if (mCurrentContact.uid().isNull())
            editEntry(abList->currentContact());
        else
            editEntry(mCurrentContact);
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
#ifdef QTOPIA_PHONE
    //abList->setKeyChars( "" );//make the whole list visible
#endif
    QContact cnt = contacts->contact(uid);
    viewOpened(cnt);
    setHighlightedLink( QString() );
    createDetailedView();
    mView->init(cnt);
    showView();
    if (!isVisible())
        showMaximized();

    //mResetKeyChars = true;
    mResetKeyChars = false;
    //abList->setCurrentIndex( contacts->index(uid) );
    //slotDetailView();
    updateIcons();
}

void AddressbookWindow::addPhoneNumberToContact(const QString& phoneNumber)
{
    QContactSelector *s = new QContactSelector( false, this );
    s->setModel(contacts);
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
            contacts->updateContact( cnt );
        }
    }
}

void AddressbookWindow::setContactImage( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
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
            QDataStream stream( processingRequest.requestData().toIODevice() );
            QPixmap np;
            stream >> np;
            cnt.changePortrait( np );
            contacts->updateContact( cnt );
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
        QPixmap np( filename );
        cnt.changePortrait( np );
        contacts->updateContact( cnt );
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
        createDetailedView();
        mView->init(cnt);
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
        if ( t.toLower() == "text/x-vcard" )
            receiveFile(f, t);
        QFile::remove(f);
    }
}

#ifdef QTOPIA_CELL

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

        QMessageBox box( tr("Contact Received"), question, QMessageBox::NoIcon,
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No, QMessageBox::NoButton, isVisible() ? this : 0,
             Qt::Dialog | Qt::WindowStaysOnTopHint);

        if(box.exec() == QMessageBox::Yes)
        {
            for(n = 0; n < vcardContacts.count(); n++)
            {
                newEntry = vcardContacts[n];

                NameLearner learner(newEntry);
                QUniqueId ui = contacts->addContact(newEntry);

                //abList->setKeyChars("");
                updateIcons();
                //abList->reload();
                //abList->setCurrentEntry(ui);
            }
        }
    }
}

#endif

void AddressbookWindow::slotPersonalView()
{
    if (showingPersonal) {
        // we just turned it off
        setWindowTitle( tr("Contacts") );
        updateIcons();
        slotListView();
        showingPersonal = false;
        return;
    }

    setWindowTitle( tr("Contacts - My Personal Details") );

    QContact me = contacts->personalDetails();
    abList->setCurrentIndex( contacts->index(me.uid()) );

    viewOpened( me );
    createDetailedView();
    mView->init( me );
    centralView->setCurrentIndex(1);
    updateIcons();
}

// hacky static bool
static bool newPersonal = false;

void AddressbookWindow::editPersonal()
{
    if (contacts->hasPersonalDetails())
    {
        QContact me = contacts->personalDetails();
        createDetailedView();
        mView->init( me );
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
    QContact c;
    if( mView && centralView->currentIndex() == AB_DETAILVIEW )
        c = mView->entry();
    else
        c = abList->currentContact();

    if (c.uid().isNull()) {
        qWarning("ab::markCurrentAsPersonal called with no selection");
        return;
    }
    if (contacts->isPersonalDetails(c.uid())) {
        contacts->clearPersonalDetails();
        updateIcons();
    }
    else {
        if (QMessageBox::information(this, tr("Contacts"),
                    tr("<qt>Set \"%1\" as your Business Card?</qt>").arg( c.label() ),
                    tr("Yes"), tr("No"), 0, 0, 1) == 0)
        {
            contacts->setPersonalDetails(c.uid());
            updateIcons();
        }
    }
}

void AddressbookWindow::newEntry()
{
    newEntry( QContact() );
}

void AddressbookWindow::newEntry( const QContact &cnt )
{
    if ( abEditor != 0 )
        return;

    QContact entry = cnt;
    abEditor = new AbEditor( parentWidget() );
    abEditor->setObjectName("edit");
    abEditor->setModal(true);
#ifdef QTOPIA_PHONE
    abEditor->setEntry( entry, true, contacts->isSIMCardContact(cnt.uid()));
#else
    abEditor->setEntry( entry, true );
#endif

    if( abEditor->exec() == QDialog::Accepted )
    {
        setFocus();
        QContact newEntry = abEditor->entry();

        NameLearner learner(newEntry);
        QUniqueId ui = contacts->addContact(newEntry);
        if(newPersonal)
            contacts->setPersonalDetails(ui);

        updateIcons();
    }

    delete abEditor;
    abEditor = 0;
}

AbEditor *AddressbookWindow::editor()
{
    if( !abEditor ) {
        abEditor = new AbEditor( isVisible() ? this : 0);
        abEditor->setObjectName("edit");
        abEditor->setModal(true);
    }

    return abEditor;
}

void AddressbookWindow::editEntry( const QContact &cnt )
{
    if ( abEditor != 0 )
        return;

    QContact entry = cnt;
    abEditor = new AbEditor( parentWidget() );
    abEditor->setObjectName("edit");
    abEditor->setModal(true);

#ifdef QTOPIA_PHONE
    bool simContact = contacts->isSIMCardContact(cnt.uid());
    abEditor->setEntry( entry, false, simContact );

    if(!simContact)
        abEditor->setNameFocus();
#else
    abEditor->setEntry( entry );
    abEditor->setNameFocus();
#endif

    if ( abEditor->exec() )
    {
        if ( abEditor->isEmpty() )
        {   // Delete if empty
            removeContactQDLLink( entry );
            removeSpeedDial( entry );
            sel_href = QString();
            contacts->removeContact( entry );
            if( mView && centralWidget() == mView )
                viewClosed();
            else
                slotListView();
        }
        else
        {   // Update if not empty
            setFocus();
            QContact editedEntry( abEditor->entry() );

            QUniqueId entryUid = editedEntry.uid();
            if( !(entry == editedEntry) || abEditor->imageModified() )
            {// only do update operations if the data has actually changed
                QPixmap *cached = QPixmapCache::find( "pimcontact" +
                    entryUid.toString() + "-cfl" );
                if( abEditor->imageModified() && cached )
                {
                    //update the contact field list image cache
                    QString photoFile( editedEntry.customField( "photofile" ) );
                    QPixmap p;
                    if( !photoFile.isEmpty() )
                    {
                        QSize pSize = QContact::portraitSize();
                        QString baseDirStr = Qtopia::applicationFileName( "addressbook",
                            "contactimages/" );
                        QString pFileName( baseDirStr + photoFile  );
                        QThumbnail thumbnail( pFileName );
                        p = thumbnail.pixmap( pSize, Qt::KeepAspectRatioByExpanding );
                        p = p.copy( ( pSize.width() - p.width() ) / 2,
                                    ( pSize.height() - p.height() ) / 2,
                                    pSize.width(), pSize.height() );
                    }
                    QPixmapCache::insert( "pimcontact" + entryUid.toString() + "-cfl", p );
                }
                //regular contact, just update
                NameLearner learner(editedEntry);
                contacts->updateContact(editedEntry);
                abList->setCurrentIndex(contacts->index(entryUid));
                mCurrentContact = editedEntry;

                if( mView && centralView->currentIndex() == AB_DETAILVIEW )
                {
                    // don't call slotDetailView because with QDL that would push the same
                    // entry onto the view stack again. just refresh
                    // need to get entry again for contact list model data.
                    editedEntry = contacts->contact(editedEntry.uid());
                    mView->init( editedEntry );
                    setHighlightedLink( mView->encodeHref() );
                }
            }
        }
    }

    delete abEditor;
    abEditor = 0;
}

void AddressbookWindow::closeEvent( QCloseEvent *e )
{
    //don't ignore a closeEvent - trap closing before we get to this point on QTOPIA_PHONE
#ifdef QTOPIA_CELL
    if( mSimIndicator )
        mSimIndicator->hide();
#endif
#ifndef AB_PDA
    if( !mAllowCloseEvent && centralView->currentIndex() != AB_CONTACTLIST)
    {
        e->ignore();
        return;
    }
#endif
    QMainWindow::closeEvent( e );
}

void AddressbookWindow::viewOpened( const QContact &entry)
{
    mCurrentContact = entry;
    /*
       This code should not be implemented again as such.
        For starters, the function should be called by what it does, not when it happens
        Also sim contacts are nothing special.  just contacts that are gotten from one place
        rather than another (there are no more dual source contacts).

    QContact last;
    if( !mContactViewStack.count() ||
            (((last=mContactViewStack.last()).customField("SIM_CONTACT").isEmpty()
                    &&  mContactViewStack.last().uid() != entry.uid() )) ||
            (last.customField("SIM_CONTACT").length() &&
                !AbUtil::compareContacts(last, entry))) // compare data of sim contacts
        mContactViewStack.append( entry );
   */
}

void AddressbookWindow::viewClosed()
{
    if ( mContactViewStack.count() > 0 )
    {
        mAllowCloseEvent = false;
        QContact prevContact = mContactViewStack.last();
        mContactViewStack.removeLast();
        if ( prevContact.uid().isNull() ) {
             slotListView();
             mContactViewStack.clear();
        } else {
            abList->setCurrentIndex( contacts->index( prevContact.uid() ) );
            slotDetailView();
        }
    }
    else if( mCloseAfterView )
    {
        mCloseAfterView = false;
        mAllowCloseEvent = true;
        slotListView();
        close();
    }
    else
    {
        mAllowCloseEvent = false;
        slotListView();
    }
}

void AddressbookWindow::slotFind(bool s)
{
#ifndef QTOPIA_PHONE
    if ( !searchBar ) {
        // Search bar
        searchBar = new QToolBar(this);
        searchBar->setMovable(false);
        addToolBar(searchBar);

        searchEdit = new QLineEdit(0);
        searchBar->addWidget(searchEdit);
        searchEdit->setObjectName("searchEdit");
        QtopiaApplication::setInputMethodHint(searchEdit,QtopiaApplication::Text);
        connect(searchEdit, SIGNAL(textChanged(const QString&)),
                this, SLOT(search(const QString &)));
    }
    if ( s )
    {
        if(centralView->currentIndex() == AB_DETAILVIEW)
            slotListView();
        searchBar->show();
        searchEdit->setFocus();
    }
    else
    {
        searchBar->hide();
        abList->clearSelection();
        bool hasCurrentEntry = abList->currentIndex().isValid();
        QUniqueId uid;
        if( hasCurrentEntry )
            uid = abList->currentContact().uid();
        abList->setFocus();
        if( !uid.isNull() )
            abList->setCurrentIndex(contacts->index( uid ));
    }
#else
    Q_UNUSED(s);
#endif
}

void AddressbookWindow::search( const QString &text )
{
    if (text.isEmpty()) {
        contacts->clearFilter();
        if ( !Qtopia::mousePreferred() )
        {
            mFindMode = false;
        }
    }
    else
    {
        contacts->setFilter( text );
    }
}

void AddressbookWindow::showCategory( const QCategoryFilter &c )
{
    contacts->setCategoryFilter( c );
    abList->update();

#ifndef QTOPIA_PHONE
    setWindowTitle( tr("Contacts") + " - " + c.label() );
#else
    if(c == QCategoryFilter(QCategoryFilter::All))
    {
        if(actionSendCat)
            actionSendCat->setText(tr("Send All..."));
        categoryLbl->hide();
    }
    else
    {
        QString lbl = c.label();
        if(actionSendCat)
            actionSendCat->setText(tr("Send %1...").arg(lbl));
        categoryLbl->setText(tr("Category: %1").arg(lbl));
        categoryLbl->show();
    }
#endif
}

/* simply disabled for now although this might be useful
void AddressbookWindow::contactFilterSelected( int idx )
{
    int cid = contactMap[idx];
    abList->setPreferredField( cid );
}
*/

void AddressbookWindow::selectAll()
{
//    abList->selectAll();
//    updateIcons();
}

void AddressbookWindow::configure()
{
#ifdef QTOPIA_PHONE
    AbDisplaySettings settings(this);
    settings.setModal(true);
    if(QtopiaApplication::execDialog(&settings) == QDialog::Accepted)
    {
        settings.saveFormat();
        QContact::setLabelFormat( settings.format() );
    }
#endif
}

void AddressbookWindow::selectCategory()
{
#ifdef QTOPIA_PHONE
    QCategoryDialog *categoryDlg;

    categoryDlg = new QCategoryDialog("Address Book", QCategoryDialog::Filter, this);
    categoryDlg->setText( tr("Only contacts which have <i>all</i> the selected categories will be shown...") );
    categoryDlg->selectFilter(contacts->categoryFilter());

    if(QtopiaApplication::execDialog(categoryDlg) == QDialog::Accepted)
        showCategory(categoryDlg->selectedFilter());

    updateIcons();

    delete categoryDlg;
#endif
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
    QContact my((const QContact &)contacts->personalDetails());
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
#ifdef QTOPIA_PHONE
    ContactSourceDialog diag(this);
    diag.setContactModel(contacts);

    if (QtopiaApplication::execDialog(&diag)) {
        QSet<QPimSource> set = contacts->visibleSources();
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
#endif
}

void AddressbookWindow::importAllFromSim()
{
    // won't work for 4.2, contexts id's have changed.
    // possibly error checking would be required.
    contacts->mirrorAll(contacts->simSource(), contacts->phoneSource());
}

void AddressbookWindow::exportAllToSim()
{
    // won't work for 4.2, contexts id's have changed.
    // possibly error checking would be required.
    if (!contacts->mirrorAll(contacts->phoneSource(), contacts->simSource())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not export contacts to SIM Card.  Please ensure sufficient"
                    " space is available on SIM Card.</qt>"));
    }
}

void AddressbookWindow::importCurrentFromSim()
{
    if (!contacts->mirrorToSource(contacts->phoneSource(), mCurrentContact.uid())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not import contact.</qt>"));
    }
}

void AddressbookWindow::exportCurrentToSim()
{
    if (!contacts->mirrorToSource(contacts->simSource(), mCurrentContact.uid())) {
        QMessageBox::warning(this, tr("Contacts"),
                tr("<qt>Could not export contact to SIM Card.  Please ensure sufficient"
                    " space is available on SIM Card for all numbers stored in contact.</qt>"));
    }
}

void AddressbookWindow::qdlActivateLink( const QDSActionRequest& request )
{
    // Check if we're already showing a contact, if so push it onto the contact
    // stack
    if ( centralView->currentIndex() == AB_DETAILVIEW ) {
        mContactViewStack.append( mCurrentContact );
    }

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
    if ( contacts->exists( contactId ) ) {
        showJustItem( contactId );
        showMaximized();
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
    if ( contacts->count() == 0 ) {
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
    s->setModel(contacts);
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

QDSData AddressbookWindow::contactQDLLink( QContact& contact )
{
    if ( contact == QContact() )
        return QDSData();

    // Check if we need to create the QDLLink
    QString keyString = contact.customField( QDL::SOURCE_DATA_KEY );

    if ( keyString.isEmpty() ||
         !QDSData( QLocalUniqueId( keyString ) ).isValid() )
    {
        QByteArray dataRef;
        QDataStream refStream( &dataRef, QIODevice::WriteOnly );
        refStream << contact.uid();

        QDLLink link( "Contacts",
                      dataRef,
                      contact.label(),
                      "pics/addressbook/AddressBook" );

        QDSData linkData = link.toQDSData();
        QLocalUniqueId key = linkData.store();
        contact.setCustomField( QDL::SOURCE_DATA_KEY, key.toString() );
        QString keyString2 = contact.customField( QDL::SOURCE_DATA_KEY );

        contacts->updateContact( contact );

        return linkData;
    }

    // Get the link from the QDSDataStore
    return QDSData( QLocalUniqueId( keyString ) );
}

void AddressbookWindow::removeSpeedDial( QContact& contact )
{
    QString speedDialInput = contact.customField( SPEEDDIAL_DATA_KEY );
    if ( speedDialInput.isEmpty() )
        return;

    QSpeedDial::remove( speedDialInput );
    contact.removeCustomField( SPEEDDIAL_DATA_KEY );
    contacts->updateContact( contact );
}

void AddressbookWindow::removeContactQDLLink( QContact& contact )
{
    if ( contact == QContact() )
        return;

    // Release any client QDLLinks
    QString links = contact.customField( QDL::CLIENT_DATA_KEY );
    if ( !links.isEmpty() ) {
        QDL::releaseLinks( links );
    }

    // Check if the QDLLink is stored
    QString key = contact.customField( QDL::SOURCE_DATA_KEY );
    if ( !key.isEmpty() ) {
        // Break the link in the QDSDataStore
        QDSData linkData = QDSData( QLocalUniqueId( key ) );
        QDLLink link( linkData );
        link.setBroken( true );
        linkData.modify( link.toQDSData().data() );

        // Now remove our reference to the link data
        linkData.remove();

        // Finally remove the stored key
        contact.removeCustomField( QDL::SOURCE_DATA_KEY );
        contacts->updateContact( contact );
    }
}

/***************************
  * AbSourcesDialog
  ***********************/

#ifdef QTOPIA_PHONE
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

#endif

/***************************
  * AbDisplaySettings
  ***********************/

AbDisplaySettings::AbDisplaySettings(QWidget *parent)
    : QDialog(parent)
{
    layout = new QVBoxLayout(this);

#ifdef QTOPIA_PHONE
    QSettings cfg( "Trolltech", "Contacts" );
    cfg.beginGroup( "formatting" );
    QString curfmt = cfg.value( "NameFormat" ).toString();

    setWindowTitle(tr("Display Settings"));
    bg = new QButtonGroup(0);

    for(int i = 0; fileas_opt[i].desc; ++i)
    {
        QRadioButton* rb = new QRadioButton(qApp->translate("AbSettings", fileas_opt[i].desc), 0);
        bg->addButton(rb);
        layout->addWidget(rb);
        if(fileas_opt[i].fmt == curfmt || i == 0 && curfmt.isEmpty())
            rb->setChecked(true);
    }
#endif
}

#ifdef QTOPIA_PHONE
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
            cfg.setValue("NameFormat", fileas_opt[index].fmt);
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
        if(index > -1)
            return fileas_opt[index].fmt;
    }

    return QString();
}
/*#else
void AbDisplaySettings::setCurrentFields(const QList<int> &);
{
    QMap<int,QString> m = PimContact::trFieldsMap();
    m.remove( PimRecord::Categories );
    m.remove( PimContact::LastNamePronunciation );
    m.remove( PimContact::FirstNamePronunciation );
    m.remove( PimContact::CompanyPronunciation );

    m.insert( AbTable::FREQ_CONTACT_FIELD, tr("Contact") );
    map->setFields( m, f);
}*/
#endif

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
    Adds a \a contact. This is not normally used by applications,
    which should use the AddressBookAccess class.

    This slot corresponds to the QCop service message
    \c{Contacts::addContact(QContact)}.
*/
void ContactsService::addContact(const QContact& contact)
{
    QContact c(contact);
    NameLearner learner(c);
    parent->contacts->addContact(c);
}

/*!
    Removes a \a contact. This is not normally used by applications,
    which should use the AddressBookAccess class.

    This slot corresponds to the QCop service message
    \c{Contacts::removeContact(QContact)}.
*/
void ContactsService::removeContact(const QContact& contact)
{
    QContact cnt( contact );
    parent->removeContactQDLLink( cnt );
    parent->removeSpeedDial( cnt );
    parent->contacts->removeContact( contact );
}

/*!
    Updates a \a contact. This is not normally used by applications,
    which should use the AddressBookAccess class.

    This slot corresponds to the QCop service message
    \c{Contacts::updateContact(QContact)}.
*/
void ContactsService::updateContact(const QContact& contact)
{
    QContact c(contact);
    NameLearner learner(c);
    parent->contacts->updateContact(c);
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
    parent->createDetailedView();
    parent->mView->init( contact );
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

#ifdef QTOPIA_PHONE

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
    if( parent->abList->model()->rowCount() == 0 || !parent->contacts->hasPersonalDetails()) {
        QMessageBox::warning( parent, tr("Contacts"), "<qt>"+tr("No business card set.")+"</qt");
    }
    else
    {
        parent->smsBusinessCard();
    }
}

/*!
    Receive a WAP push message of type \c{text/x-vcard}.
*/
void ContactsPhoneService::pushVCard( const QDSActionRequest& request )
{
    parent->vcardDatagram( request.requestData().data() );
    QDSActionRequest( request ).respond();
}

#endif // QTOPIA_CELL

#endif // QTOPIA_PHONE

