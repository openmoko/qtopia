/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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



#ifndef ADDRESSLIST_H
#define ADDRESSLIST_H

#include <qtopia/pim/qcontactview.h>
#include <QDialog>
#include <QContact>

class AddressPickerData;
class AddressPickerItem;

class AddressPicker : public QContactListView
{
    Q_OBJECT
public:
    AddressPicker(QWidget *parent = 0);
    ~AddressPicker();

    void clear();
    void setFilterFlags(QContactModel::FilterFlags);
    void resetFilterFlags();

    bool isEmpty() const;

    QStringList addressList() const;
    QContactItemModel *contactItemModel() const;

signals:
    void selectionChanged();

public slots:
    void addAddress();
    void removeAddress();
    void addressSelected(const QContact&, const QString&);
    void phoneTypeSelected(QContact::PhoneType);
    void contactSelected(const QContact&);
    void addTextAddress(const QString&);
    void selectionCanceled();

private:
    void addItem(AddressPickerItem*);

    AddressPickerData *d;
};

#endif
