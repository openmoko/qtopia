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
#ifndef __CONTACTVIEW_H__
#define __CONTACTVIEW_H__

#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactmodel.h>

#include <QListView>
#include <QAbstractItemDelegate>
#include <QMap>
#include <QDialog>

class QFont;
class QKeyEvent;

class QTOPIAPIM_EXPORT QContactDelegate : public QAbstractItemDelegate
{
public:
    explicit QContactDelegate( QObject * parent = 0 );
    virtual ~QContactDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option,
            const QModelIndex &index) const;

    virtual QFont mainFont(const QStyleOptionViewItem &) const;
    virtual QFont secondaryFont(const QStyleOptionViewItem &) const;
private:
    QFont differentFont(const QFont& start, int step) const;
};

class QTOPIAPIM_EXPORT QContactListView : public QListView
{
    Q_OBJECT

public:
    explicit QContactListView(QWidget *parent = 0);
    ~QContactListView();

    void setModel( QAbstractItemModel * );

    QContact currentContact() const
    {
        if (contactModel() && currentIndex().isValid())
            return contactModel()->contact(currentIndex());
        return QContact();
    }

    QList<QContact> selectedContacts() const;
    QList<QUniqueId> selectedContactIds() const;

    QContactModel *contactModel() const { return qobject_cast<QContactModel *>(model()); }

    QContactDelegate *contactDelegate() const { return qobject_cast<QContactDelegate *>(itemDelegate()); }

protected slots:
    void slotCurrentChanged(const QModelIndex& newIdx);

protected:

    // This is a binary compat stub now.
    void keyPressEvent(QKeyEvent *event);
};

class QContactSelectorPrivate;
class QTOPIAPIM_EXPORT QContactSelector : public QDialog
{
    Q_OBJECT
public:
    QContactSelector(bool allowNew, QWidget *);
    void setModel(QContactModel *);

    bool newContactSelected() const;
    bool contactSelected() const;
    QContact selectedContact() const;

private slots:
    void setNewSelected();
    void setSelected(const QModelIndex&);

private:
    QContactSelectorPrivate *d;
};


class QPhoneTypeSelectorPrivate;
class QTOPIAPIM_EXPORT QPhoneTypeSelector : public QDialog
{
    Q_OBJECT
public:
    QPhoneTypeSelector( const QContact &cnt, const QString &number,
        QWidget *parent = 0);

    QContact::PhoneType selected() const;
    QString selectedNumber() const;

    void updateContact(QContact &, const QString &) const;
protected slots:
    void accept();
signals:
    void selected(QContact::PhoneType);
protected:
    void resizeEvent(QResizeEvent *resizeEvent);
private:
    void init();

    QPhoneTypeSelectorPrivate *d;

    QString verboseIfEmpty( const QString &number );
};

#endif//__CONTACTVIEW_H__
