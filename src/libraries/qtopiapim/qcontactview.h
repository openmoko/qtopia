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
#ifndef __CONTACTVIEW_H__
#define __CONTACTVIEW_H__

#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/pim/qpimdelegate.h>

#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QMap>
#include <QDialog>

//class QFont;
//class QKeyEvent;

class QTOPIAPIM_EXPORT QContactItem : public QStandardItem
{
public:
    QContactItem();
    QContactItem(const QContact &, const QString &sublabel = QString(), const QPixmap &statusIcon = QPixmap());
    ~QContactItem();

    QString label() const;
    void setLabel(const QString &text);

    QPixmap portrait() const;
    void setPortrait(const QPixmap &image);

    QString subLabel() const;
    void setSubLabel(const QString &text);

    QPixmap statusIcon() const;
    void setStatusIcon(const QPixmap &image);
};

class QTOPIAPIM_EXPORT QContactItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    using QStandardItemModel::appendRow;
    QContactItemModel(QObject *parent = 0);
    ~QContactItemModel();

    void appendRow(const QContact &, const QString &sublabel = QString(), const QPixmap &statusIcon = QPixmap());

    QStringList labels() const;
    QStringList subLabels() const;
};

class QTOPIAPIM_EXPORT QContactDelegate : public QPimDelegate
{
    Q_OBJECT

public:
    explicit QContactDelegate( QObject * parent = 0 );
    virtual ~QContactDelegate();

    BackgroundStyle backgroundStyle(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    SubTextAlignment subTextAlignment(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    QList<StringPair> subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const;
};

class QTextEntryProxy;
class QContactListViewPrivate;
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

    void setTextEntryProxy(QTextEntryProxy *);
    QTextEntryProxy *textEntryProxy() const;

private slots:
    void currentContactChanged(const QModelIndex& newIdx);

private slots:
    void setFilterText();

protected:

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

private:
    QContactListViewPrivate *d;
};

class QContactSelectorPrivate;
class QTOPIAPIM_EXPORT QContactSelector : public QDialog
{
    Q_OBJECT
public:
    QContactSelector(bool allowNew, QWidget * = 0);
    QContactSelector(QWidget * = 0);
    ~QContactSelector();

    void setCreateNewContactEnabled(bool);
    void setAcceptTextEnabled(bool);

    void setModel(QContactModel *);

    bool newContactSelected() const;
    bool contactSelected() const;
    bool textSelected() const;

    QContact selectedContact() const;
    QString selectedText() const;

    bool eventFilter(QObject *, QEvent *);

signals:
    void contactSelected(const QContact&);
    void textSelected(const QString&);

private slots:
    void setNewSelected();
    void setSelected(const QModelIndex&);
    void filterList(const QString &);
    void contactModelReset();
    void completed();

private:
    void init();
    QContactSelectorPrivate *d;
};


class QPhoneTypeSelectorPrivate;
class QTOPIAPIM_EXPORT QPhoneTypeSelector : public QDialog
{
    Q_OBJECT
public:
    QPhoneTypeSelector( const QContact &cnt, const QString &number,
        QWidget *parent = 0);
    QPhoneTypeSelector( const QContact &cnt, const QString &number,
        QList<QContact::PhoneType> allowedTypes, QWidget *parent = 0);
    ~QPhoneTypeSelector();

    QContact::PhoneType selected() const;
    QString selectedNumber() const;

    void updateContact(QContact &, const QString &) const;
public slots:
    void accept();
signals:
    void selected(QContact::PhoneType);
protected:
    void resizeEvent(QResizeEvent *resizeEvent);
    void keyPressEvent(QKeyEvent *ke);
private:
    void init();

    QPhoneTypeSelectorPrivate *d;

    QString verboseIfEmpty( const QString &number );
};

#endif//__CONTACTVIEW_H__
