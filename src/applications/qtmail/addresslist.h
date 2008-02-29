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



#ifndef ADDRESSLIST_H
#define ADDRESSLIST_H

#include <QDialog>
#include <QTreeWidget>
#include <QComboBox>
#include <QToolButton>
#include <QList>

#include <qcategoryselector.h>
#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactmodel.h>



class EMailAddress
{
public:
    EMailAddress()
    {
        deleted = false;
        merged = false;
        fromContacts = false;
    }

    // we define the email address to be unique (correct in 99.9999 cases)
    bool operator==(const EMailAddress &other) const
    {
        return (email == other.email);
    }

    QString email;
    QString name;
    QStringList categories;
    bool deleted;
    bool merged;
    bool fromContacts;
};

class AddressList: public QObject
{
    Q_OBJECT
public:
    AddressList();
    ~AddressList();

    void load();
    void save();

    QList<EMailAddress> contacts();
    QList<EMailAddress> emailContacts();

    void deleteAddress(EMailAddress);
    void addAddress(const QString &mailAdr);
    bool addToContacts(const QString &mailAdr);
    void addAddress(const QString &name, const QString &email);
    void addContact(const QContact &c);

signals:
    void updated();

protected slots:
    void contactsUpdated(const QModelIndex&,const QModelIndex&);

private:
    void mergeLists();
    void loadContacts();
    void loadEmailContacts();
    bool parseLine(const QString &, EMailAddress *);
    QContactModel* contactModel();

private:
    bool _dirty;
    bool _contactsLoaded;

    QContactModel*      _contactModel;
    QList<EMailAddress> _contacts;
    QList<EMailAddress> _emailContacts;
};

class AddressView : public QTreeWidget
{
    Q_OBJECT
public:
    AddressView(QWidget *parent = 0);
protected:
    void keyPressEvent(QKeyEvent* e);
};

class AddressItem: public QTreeWidgetItem
{
public:
    AddressItem(QTreeWidget *parent, const EMailAddress &, int);

    EMailAddress address() {return _address; };
    bool fromContacts() const { return _address.fromContacts; }

    QString key(int c, bool) const;
protected:
    void setColumns();
#ifdef QTOPIA4_TODO
    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
#endif
private:
    EMailAddress _address;
    int show;
};

class AddressPicker : public QDialog
{
    Q_OBJECT

public:
    enum AddressType {
        Name,
        Email,
        NameAndEmail
    };

    static AddressList* addressList();

    AddressPicker(AddressType atype, QWidget *parent = 0, const QString name = QString(), Qt::WFlags fl = 0);
    ~AddressPicker();

    QStringList addressesSelected();
    void setPreviousAddresses(QStringList &);

protected slots:
    void setCategoryFilter(int);
    void comboSelected(int);
    void updateListButtons();

    void addToContacts();
    void removeFromList();
    void listAddresses();

private:
    void init();
    void listEntry(const EMailAddress &);
    bool useCategories();

private:
    AddressType _atype;
#ifdef QTOPIA4_TODO
    int m_Category;
#endif

    AddressList *list;
    AddressView *addressView;
#ifndef QTOPIA_PHONE
    QComboBox *combo;
    QCategorySelector *sel;
    QToolButton *deleteButton, *addButton;
#endif
};

#endif
