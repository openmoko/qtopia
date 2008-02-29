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

#include "addresslist.h"

#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QStyle>
#include <QKeyEvent>
#include <QPainter>


#include <qtopiaglobal.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
// #include <qtopia/qcategoriesmanager.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#include <qtopiaapplication.h>
#endif

#include <qtopia/mail/mailmessage.h>
#include <qtopiaservices.h>
#ifdef QTOPIA_DESKTOP
#include <qcopenvelope_qd.h>
#else
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#endif

static QIcon* pm_contacts=0;
//static QPixmap* pm_email;

static AddressList *alist=0;

static void ensurePixmaps()
{
    if ( !pm_contacts )
        pm_contacts = new QIcon(":icon/addressbook/AddressBook");

}

/*  Build a simple string format that the emailclient understands */
static QString buildAddress(QString name, QString email)
{
    QString str = MailMessage::quoteString(name) + " ";
    if ( email[0] == '<' ) {
        str += email;
    } else {
        str += "<" + email + ">";
    }

    return str;
}

static QContact contact(const QString &name, const QString &email)
{
    QContact c;

    QStringList l = name.split(" ",QString::SkipEmptyParts);
    QStringListIterator lit(l);
    if ( lit.hasNext() ) {
        c.setFirstName(lit.next());

    }
    if ( lit.hasNext()) {
        c.setMiddleName(lit.next());
    }
    if ( lit.hasNext() ) {
        c.setLastName( lit.next() );
    }

    c.insertEmail( email );

    return c;
}

AddressList::AddressList()
    :
    _contactModel(0)
{


    loadEmailContacts();
    _dirty = false;
    _contactsLoaded = false;
}

QContactModel* AddressList::contactModel()
{
    if ( !_contactModel ) {
            _contactModel = new QContactModel(this);
        connect( _contactModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),SLOT(contactsUpdated(const QModelIndex&,const QModelIndex&)) );
    }

    return _contactModel;
}

AddressList::~AddressList()
{
    if ( _contactModel )
        delete _contactModel;

    save();
}

void AddressList::load()
{
    loadContacts();
    mergeLists();
}

void AddressList::contactsUpdated(const QModelIndex&, const QModelIndex&)
{
    // we don't read the addressbook until we have to.  Ignore if we haven't read it yet
    if ( _contactsLoaded ) {
        loadContacts();
        mergeLists();
        emit updated();
    }
}

void AddressList::loadContacts()
{
    _contacts.clear();


    for (int index = 0; index < contactModel()->count(); ++index) {
        QContact c(contactModel()->contact(index));

        EMailAddress a;
    a.name = c.firstName(); //TODO fullName function here
        a.categories = c.categories();
        a.fromContacts = true;
        QStringList list = c.emailList();
        for (QStringList::Iterator lit = list.begin(); lit != list.end(); ++lit ) {
            a.email = *lit;
            _contacts.append(a);
        }
    }
    _contactsLoaded = true;
}

void AddressList::loadEmailContacts()
{
    QString file = Qtopia::applicationFileName("qtmail", "emailaddresslist.txt");
    QFile f( file );
    if ( !f.open( QIODevice::ReadOnly ) )
        return;

    QStringList l = QString(f.readAll()).split("\n",QString::SkipEmptyParts);
    f.close();

    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
        EMailAddress a;
        if ( parseLine( *it, &a ) )
            _emailContacts.append( a );
    }

}

bool AddressList::parseLine(const QString &s, EMailAddress *a)
{
    QString str = s.trimmed();

    int pos;
    if ( (pos = str.indexOf(" ")) > -1 ) {
        QString action = s.left(pos);
        QString c = s.mid(pos).trimmed();
        if ( action == "list") { // No tr
        } else if ( action == "deleted" ) { // No tr
            a->deleted = true;
        } else if ( action == "merged" ) { // No tr
            a->merged = true;
        } else return false;

        MailMessage::parseEmailAddress(c, a->name, a->email);
        return true;
    }

    return false;
}

/*  Sync email addresslist and contacts.*/
void AddressList::mergeLists()
{
    qLog(Messaging) << "merging email addressList and contact list";

    for (QList<EMailAddress>::Iterator it = _emailContacts.begin(); it != _emailContacts.end(); ++it ) {
        // deleted stays deleted no matter what
        if ( !(*it).deleted ) {
            // if we've added it to the addressbook, set it as deleted if it no longer exists there
            if ( (*it).merged ) {
                bool removed = true;
                for ( QList<EMailAddress>::Iterator cit = _contacts.begin(); cit != _contacts.end(); ++cit) {
                    if ( (*cit).email == (*it).email ) {
                        removed = false;
                        break;
                    }
                }
                if ( removed ) {
                    qLog(Messaging) << "rec" << (*it).email.data() << "was merged and now removed";
                    (*it).deleted = true;
                    _dirty = true;
                }
            } else {
                bool added = false;
                for ( QList<EMailAddress>::Iterator cit = _contacts.begin(); cit != _contacts.end(); ++cit) {
                    if ( (*cit).email == (*it).email ) {
                        added = true;
                        break;
                    }
                }
                if ( added ) {
                    qLog(Messaging) << "rec" << (*it).email.data() << "was in email list only, but now exists in contacts as well";
                    (*it).merged = true;
                    _dirty = true;
                }
            }
        }
    }
}

void AddressList::save()
{
    if ( !_dirty )
        return;

    QString file = Qtopia::applicationFileName("qtmail", "emailaddresslist.txt");
    QFile f( file );
    if ( !f.open( QIODevice::WriteOnly ) ) {
        qWarning("could not save email address list");
        return;
    }
    QTextStream t(&f);
    t << "# This is the internal address list for the email application\n"; // No tr
    t << "# merged = contact is in addressbook.  #deleted = deleted from internal list or addressbook\n"; // No tr
    t << "# list = believed to be in internal list only\n"; // No tr

    QString str;
    for (QList<EMailAddress>::Iterator it = _emailContacts.begin(); it != _emailContacts.end(); ++it) {
        if ( (*it).deleted ) {
            str = "deleted "; // No tr
        } else if ( (*it).merged ) {
            str = "merged "; // No tr
        } else {
            str = "list "; // No tr
        }
        str += buildAddress( (*it).name, (*it).email );
        t << str << "\n";
    }

    _dirty = false;
    f.close();
}

QList<EMailAddress> AddressList::contacts()
{
    return _contacts;
}

QList<EMailAddress> AddressList::emailContacts()
{
    return _emailContacts;
}

void AddressList::deleteAddress(EMailAddress adr)
{
    if ( _emailContacts.contains(adr) ) {
        int index = _emailContacts.indexOf(adr);
        EMailAddress& eaddr = _emailContacts[index];
        eaddr.deleted = true;
        _dirty = true;
    }
}

void AddressList::addAddress(const QString &mailAdr)
{
    QString name, email;
    MailMessage::parseEmailAddress(mailAdr, name, email);
    addAddress(name, email);
}

bool AddressList::addToContacts(const QString &mailAdr)
{
    QString name, email;
    MailMessage::parseEmailAddress(mailAdr, name, email);

    // add to internal list first it it isn't there
    addAddress(name, email);

    if ( !_contactsLoaded )
        load();

    for ( QList<EMailAddress>::Iterator cit = _contacts.begin(); cit != _contacts.end(); ++cit) {
        if ( (*cit).email == email )
            return false;
    }

    QContact c = contact(name, email);
    QtopiaServiceRequest e( "Contacts", "addAndEditContact(QContact)" );
    e << c;
    e.send();

    return true;
}

void AddressList::addAddress(const QString &name, const QString &email)
{
    EMailAddress a;
    a.name = name;
    a.email = email;

    //should do a mergeitem first though
    if ( !_emailContacts.contains(a) ) {
        _emailContacts.append(a);
        _dirty = true;
    } else {
    // let's see if we're updated
        int index = _emailContacts.indexOf(a);
        EMailAddress eaddr = _emailContacts.at(index);
        if ( name.length() > 0 && eaddr.name != name ) {
            eaddr.name = name;
            _dirty = true;
        }
    }
}

void AddressList::addContact(const QContact &c)
{
    QtopiaServiceRequest e( "Contacts", "addContact(QContact)" );
    e << c;
    e.send();
}

AddressView::AddressView(QWidget *parent)
    : QTreeWidget(parent)
{
}

void AddressView::keyPressEvent(QKeyEvent* e)
{
    switch( e->key() ) {
#ifdef QTOPIA_PHONE
        case Qt::Key_Select:
#endif
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            const QTreeWidgetItem* current = currentItem();
            setItemSelected(current,!isItemSelected(current));
        }
        break;
        default:  QTreeWidget::keyPressEvent( e );
    }
}

AddressItem::AddressItem(QTreeWidget *parent, const EMailAddress &a, int i)
    : QTreeWidgetItem(parent),
    _address(a),
    show(i)
{
    ensurePixmaps();
    setColumns();
}

QString AddressItem::key(int c, bool) const
{
    if ( c == 0 ) {
        QString s = QString::number( fromContacts() );
        s += text(1).toLower();
        return s;
    }

    return text(c).toLower();
}

void AddressItem::setColumns()
{
    if ( _address.fromContacts )
        setIcon(0, *pm_contacts);
//    else
//      setPixmap(0, *pm_email);


//TODO fix multiple columns focus problem
//  if ( show == (int) AddressPicker::Name || show == (int) AddressPicker::NameAndEmail )
//      setText(1, _address.name );

    setText(0, _address.name );
    if ( show == (int) AddressPicker::Email || show == (int) AddressPicker::NameAndEmail )
        setText(2, _address.email );
}
#ifdef QTOPIA4_TODO
void AddressItem::paintCell( QPainter *p, const QColorGroup &cg,
                                int column, int width, int alignment )
{
    QColorGroup _cg( cg );

    //while testing
    if ( _address.deleted )
        _cg.setColor( QColorGroup::Text, Qt::red );
//    else if ( !_address.fromContacts )
//        _cg.setColor( QColorGroup::Text, Qt::blue );

    QListWidgetItem::paintCell( p, _cg, column, width, alignment );
    _cg.setColor( QColorGroup::Text, column );
}
#endif


AddressPicker::AddressPicker(AddressType atype, QWidget *parent, const QString name, Qt::WFlags fl)
    : QDialog(parent, fl)
{
    setObjectName( name );
    _atype = atype;
    list = addressList();
    init();

//     m_Category = -2;
    list->load();
    connect(list, SIGNAL(updated()), this, SLOT(listAddresses()) );
    listAddresses();

    setWindowTitle( tr("Select Contact") );

    addressView->setFocus();
    //hack to get focus on first item
    QModelIndex m = addressView->model()->index( 0, 0 );
    addressView->setCurrentIndex(m);
    addressView->setItemSelected(addressView->topLevelItem(0),false);

}

AddressPicker::~AddressPicker()
{
    // Don't really have to save here, but let's do it to ensure we don't lose many addresses
    // in case of a crash happening somewhere
    addressList()->save();
}

AddressList* AddressPicker::addressList()
{
    if ( !alist )
        alist = new AddressList();
    return alist;
}

void AddressPicker::init()
{
    QGridLayout *grid = new QGridLayout(this);

#ifdef QTOPIA_PHONE
    grid->setMargin(0);
    grid->setSpacing(0);
#else
#ifdef QTOPAI4_TODO
    grid->setMargin(3);
    grid->setSpacing(4);

    combo = new QComboBox(this);
    connect(combo, SIGNAL(activated(int)), this, SLOT(comboSelected(int)) );

    combo->insertItem( tr("Contacts and Email address list") );
    combo->insertItem( tr("Contacts only") );
    combo->insertItem( tr("Email address list only") );
#endif
#endif

    addressView = new AddressView(this);
    connect(addressView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(updateListButtons()) );
    addressView->setSelectionMode( QAbstractItemView::MultiSelection );
    addressView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList columns;

//TODO multiple columns focus problem
//     columns << "@";

    if ( _atype == Name || _atype == NameAndEmail )
        columns << tr( "Name" );
    if ( _atype == Email || _atype == NameAndEmail )
        columns << tr (" Email ");
    addressView->setColumnCount(columns.count());
    addressView->setHeaderLabels(columns);
    addressView->setRootIsDecorated(false);
    //addressView->setSortingEnabled(true);
    setTabOrder(addressView,addressView);
#ifdef QTOPIA4_TODO
#ifndef QTOPIA_PHONE
    sel = new CategorySelect(this);

    Categories c;
    c.load( categoryFileName() );
    QArray<int> vl( 0 );
    sel->setCategories(vl, tr("Email"), tr("Email") );
    sel->setAllCategories( true );
    connect(sel, SIGNAL(signalSelected(int)), this, SLOT(setCategoryFilter(int)) );

    deleteButton = new QToolButton(this);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(removeFromList()) );
    deleteButton->setPixmap( QIcon( ":icon/trash" ).pixmap() );

    addButton = new QToolButton(this);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addToContacts()) );
    addButton->setText( tr( "Add to Contacts" ) );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );

    grid->addMultiCellWidget( combo, 0, 0, 0, 2);
    grid->addMultiCellWidget(addressView, 1, 1, 0, 2);
    grid->addMultiCellWidget(sel, 2, 2, 0, 2);
    grid->addWidget(deleteButton, 3, 0);
    grid->addWidget(addButton, 3, 1);
    grid->addItem(spacer, 3, 2);
//#else
#endif
#endif
    grid->addWidget(addressView, 0, 0 );
// #endif
}

void AddressPicker::listAddresses()
{
    addressView->clear();


#ifdef QTOPIA_PHONE
    int filter = 0;
#else
    int filter = combo->currentIndex();
#endif
    if ( filter != 2 ) {
        QList<EMailAddress> cList = list->contacts();
        for (QList<EMailAddress>::Iterator it = cList.begin(); it != cList.end(); ++it) {
            listEntry( *it );
        }
    }

    if ( filter != 1 ) {
        QList<EMailAddress> cList = list->emailContacts();
        for (QList<EMailAddress>::Iterator eit = cList.begin(); eit != cList.end(); ++eit) {
            listEntry( *eit );
        }
   }

    updateListButtons();
    for(int i =0; i < addressView->columnCount();i++)
        addressView->resizeColumnToContents(i);
    //addressView->fitHeaders( addressView->width() );
}

void AddressPicker::listEntry(const EMailAddress &a)
{
#ifdef QTOPIA4_TODO
//     if ( useCategories() ) {
//      QStringList cats = a.categories;
//      if ( m_Category == -1 ) {
//          if ( cats.count() > 0 )
//              return;
//      }  else if ( m_Category != -2 ) {
//          if (cats.indexOf(m_Category) == -1)
//              return;
//      }
//     }
#endif
    if ( a.fromContacts ) {
        (void) new AddressItem(addressView, a, _atype);
    } else {
        if ( !a.deleted && !a.merged )
            (void) new AddressItem(addressView, a, _atype);
    }
}

bool AddressPicker::useCategories()
{
#ifdef QTOPIA_PHONE
    return false;
#else
    return combo->currentIndex() != 2;
#endif
};


QStringList AddressPicker::addressesSelected()
{
    QStringList list;

    QList<QTreeWidgetItem*> selected = addressView->selectedItems();
    QList<QTreeWidgetItem*>::Iterator it;
    for (it = selected.begin() ; it != selected.end(); ++it) {
        EMailAddress a = ((AddressItem *)(*it))->address();
        switch ( _atype ) {
        case Name: list.append( a.name ); break;
        case Email: list.append(a.email ); break;
        default: list.append( buildAddress( a.name, a.email ) ); break;
        }
    }

    return list;
}

void AddressPicker::setPreviousAddresses(QStringList &l)
{
    QString str;
    AddressItem *currentItem = 0;
    int count = 0;
    QTreeWidgetItem* firstItem = addressView->headerItem();

    for ( int index = 0; index < firstItem->childCount(); ++index){
        currentItem = (AddressItem *) firstItem->child(index);
        EMailAddress a = currentItem->address();
            str = buildAddress( a.name, a.email );
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            if ( str == *it ) {
                addressView->setItemSelected((QTreeWidgetItem *) currentItem,true);
                it = l.erase( it );
                count++;
                break;
            }
        }
    }

    if ( currentItem && count == 1 )
        addressView->scrollToItem( (QTreeWidgetItem *) currentItem);
}

void AddressPicker::setCategoryFilter(int i)
{
    Q_UNUSED(i)
#ifdef QTOPIA4_TODO
    if ( m_Category != i ) {
        m_Category = i;
        listAddresses();
    }
#endif
}

void AddressPicker::comboSelected(int)
{
#ifndef QTOPIA_PHONE
    if ( !useCategories() )
        sel->hide();
    else
        sel->show();
#endif
    listAddresses();
}

void AddressPicker::updateListButtons()
{
#ifdef QTOPIA4_TODO
    bool enable = false;

    QListWidgetItemIterator it(addressView);
    for ( ; it.current(); ++it) {
        if ( it.current()->isSelected() ) {
            if ( ((AddressItem *) it.current())->fromContacts() ) {
                enable = false;
                break;
            } else {
                enable = true;
            }
        }
    }

#ifndef QTOPIA_PHONE
    deleteButton->setEnabled(enable);
    addButton->setEnabled(enable);
#endif
#endif
}

void AddressPicker::addToContacts()
{
    QList<EMailAddress> adr;

    QList<QTreeWidgetItem*> selected = addressView->selectedItems();
    QList<QTreeWidgetItem*>::iterator itView;
    for (itView = selected.begin() ; itView != selected.end(); ++itView) {
        EMailAddress a = ((AddressItem *) (*itView))->address();
        if ( !a.fromContacts ) {
            adr.append(a);
        }
    }

    //avoid uodate signals while we're adding contacts
    disconnect(list, SIGNAL(updated()), this, SLOT(listAddresses()) );
    for ( QList<EMailAddress>::Iterator it = adr.begin(); it != adr.end(); ++it) {
        QContact c = contact((*it).name, (*it).email);

        if ( adr.count() == 1 ) {
            QtopiaServiceRequest e( "Contacts", "addAndEditContact(QContact)" );
            e << c;
            e.send();
        } else {
            list->addContact( c );
        }
    }
    if ( adr.count() )
        listAddresses();

    // reconnect signals as we're done adding addresses
    connect(list, SIGNAL(updated()), this, SLOT(listAddresses()) );
}

void AddressPicker::removeFromList()
{
    QList<QTreeWidgetItem*> selected = addressView->selectedItems();
    QList<QTreeWidgetItem*>::iterator it;
    for (it = selected.begin() ; it != selected.end(); ++it) {
          //strictly not necessary, but lets be safe
        if ( !((AddressItem *) (*it))->fromContacts() ) {
            list->deleteAddress( ((AddressItem *) (*it))->address() );
        }
    }
    listAddresses();
}


