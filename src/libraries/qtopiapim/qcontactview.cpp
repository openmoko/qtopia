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

#include <qtopia/pim/qcontactview.h>

#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QAction>
#include <QDebug>
#include <QPainter>
#include <QMenu>
#include <QTextDocument>
#include <QTextFrame>
#include <QAbstractTextDocumentLayout>

#include <QKeyEvent>

#include <qtopiaapplication.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

/*!
  \class QContactDelegate
  \module qpepim
  \ingroup pim
  \brief The QContactDelegate class provides item drawing for QContactModel items.

  The QContactDelegate class provides drawing of QContactModel items to aid in consistent
  look and feel with other applications dealing with QContactItems.

  It gets from the model data for additional roles defined in QContactModel::QContactModelRole.

  QContactModel::PortraitRole is expected to be a pixmap that will be drawn to the left
  of the item centered to the height of the item.  QContactModel::StatusIconRole if provided
  is expected to be a pixmap that will be drawn to the right of the item centered to the height
  of the item. Qt::DisplayRole is expected to be plain formatted text and will be drawn
  at the top of the model start at the right of where the portrait role is, and ending to the
  left of the status icon if present, otherwise the right of the item.  If Room is available,
  text provided by QContactModel::SubLabelRole will be drawn as plain text below the label text.
*/

/*!
  Contstructs a QContactModelDelegate with parent \a parent.
*/
QContactDelegate::QContactDelegate( QObject * parent )
    : QPimDelegate(parent)
{
}

/*!
  Returns a list consisting of a single string, with the QContactModel::SubLabelRole for the
  supplied \a index.  Ignores \a option.
*/
QList<StringPair> QContactDelegate::subTexts(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    QList< StringPair > subList;
    QString subLabel = index.model()->data(index, QContactModel::SubLabelRole).toString();
    subList.append(qMakePair(QString(), subLabel));
    return subList;
}

/*!
  Returns the count of subsidiary lines for a contact, which is set to one (the QContactModel::SubLabelRole
  text).

  Ignores \a index and \a option.
 */
int QContactDelegate::subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // We usually have one row, and no harm comes from this if we have zero
    return 1;
}

/*!
  Draws the portrait picture (QContactModel::PortraitRole) for the contact
  specified by \a index on the leading side of the item, and the pixmap
  specified by QContactModel::StatusIconRole on the trailing side of the
  item.  The leading and trailing sides are determined by the \a rtl parameter.
  The pixmaps are drawn using the painter \a p.  The rectangle taken up by the
  portrait is added to \a leadingFloats, while the rectangle taken up by the
  status icon is added to \a trailingFloats.

  Ignores \a option.
*/
void QContactDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index,
                                      QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const
{
    QPixmap decoration = qvariant_cast<QPixmap>(index.model()->data(index, QContactModel::PortraitRole));
    QPixmap trailingdecoration = qvariant_cast<QPixmap>(index.model()->data(index, QContactModel::StatusIconRole));

    QRect drawRect;
    QSize ths;

    if (!decoration.isNull()) {
        drawRect = option.rect;
        ths = QContact::thumbnailSize();
        if (rtl)
            drawRect.setLeft(drawRect.right() - ths.width() - 4);
        else
            drawRect.setWidth(ths.width() + 4);

        // Center the thumbnail
        QPoint drawOffset = QPoint(drawRect.left() + (drawRect.width() - ths.width())/2, drawRect.top() + (drawRect.height() - ths.height())/2);

        p->drawPixmap(drawOffset, decoration);

        leadingFloats.append(drawRect);
    }


    if (!trailingdecoration.isNull()) {
        drawRect = option.rect;
        ths = trailingdecoration.size();
        if (rtl)
            drawRect.setWidth(ths.width() + 2);
        else
            drawRect.setLeft(drawRect.right() - ths.width() - 2);

        // Center the thumbnail
        QPoint drawOffset = QPoint(drawRect.left() + (drawRect.width() - ths.width())/2, drawRect.top() + (drawRect.height() - ths.height())/2);

        p->drawPixmap(drawOffset, trailingdecoration);

        trailingFloats.append(drawRect);
    }
}

/*!
  Always returns QPimDelegate::Independent, ignoring \a option and \a index.
*/
QPimDelegate::SubTextAlignment QContactDelegate::subTextAlignment(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QPimDelegate::Independent;
}

/*!
  Always returns QPimDelegate::SelectedOnly, ignoring \a option and \a index.
 */
QPimDelegate::BackgroundStyle QContactDelegate::backgroundStyle(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QPimDelegate::SelectedOnly;
}

/*!
  Calculates the size hint for the delegate, given the \a option and \a index parameters.
  The \a textSize size hint is expanded to accommodate the portrait icon width and height.
*/
QSize QContactDelegate::decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // Note - this ignores the secondaryIconRole pixmap, since it is of variable size
    QSize ths = QContact::thumbnailSize();
    return QSize(ths.width() + 2 + textSize.width(), qMax(ths.height() + 4, textSize.height()));
}

/*!
  Destroys a QContactModelDelegate.
*/
QContactDelegate::~QContactDelegate() {}

/*!
  \class QContactListView
  \module qpepim
  \ingroup pim
  \brief The QContactListView class provides a list view widget with some convenience functions
  for use with QContactModel.

  The QContactListView is a QListView with some additional functions to aid in using a list view
  of a QContactModel.
*/

/*!
    \fn QContact QContactListView::currentContact() const

    Return the QContact for the currently selected index.
*/

/*!
  \fn QContactModel *QContactListView::contactModel() const

  Returns the QContactModel set for the view.
*/

/*!
  \fn QContactDelegate *QContactListView::contactDelegate() const

  Returns the QContactDelegate Set for the view.
*/

/*!
  Contructs a QContactListView with parent \a parent.
*/
QContactListView::QContactListView(QWidget *parent)
    : QListView(parent)
{
    setItemDelegate(new QContactDelegate(this));
    setResizeMode(Adjust);
    setLayoutMode(Batched);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

/*!
  Destorys the QContactListView.
*/
QContactListView::~QContactListView()
{
}

/*!
  \overload

  Sets the model for the view to \a model.

  Will only accept the model if it inherits or is a QContactModel.
*/
void QContactListView::setModel( QAbstractItemModel *model )
{
    QContactModel *tm = qobject_cast<QContactModel *>(model);
    if (!tm)
        return;
    QListView::setModel(model);
    /* connect the selectionModel (which is created by setModel) */
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(currentContactChanged(const QModelIndex &)));
}

/*!
  Returns the list of complete contacts selected from the view.  If a large number of contacts
  might be selected this function can be expensive, and selectedContactIds() should be used
  instead

  \sa selectedContactIds()
*/
QList<QContact> QContactListView::selectedContacts() const
{
    QList<QContact> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(contactModel()->contact(i));
    }
    return res;
}

/*!
  Returns the list of ids for contacts selected from the view.

  \sa selectedContacts()
*/
QList<QUniqueId> QContactListView::selectedContactIds() const
{
    QList<QUniqueId> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(contactModel()->id(i));
    }
    return res;
}

/*
   Try to make sure we select the current item, whenever it
   changes.
   */
void QContactListView::currentContactChanged(const QModelIndex& newIndex)
{
    if (newIndex.isValid()) {
        selectionModel()->select(newIndex, QItemSelectionModel::Select);
    }
}

/*
   This code used to be necesary, but is not any more.  Binary compatibility
   requires it here, though.
*/
/*!
  \reimp
*/
void QContactListView::keyPressEvent(QKeyEvent *event)
{
    // Ensure that the current item is changed.
    QListView::keyPressEvent(event);
}

/***************************
  * QContactSelector
  ***********************/
class QContactSelectorPrivate
{
public:
    QContactListView *view;
    bool mNewContactSelected;
    bool mContactSelected;
};

/*!
  \class QContactSelector
  \module qpepim
  \ingroup pim
  \brief The QContactSelector class provides a way of selecting a contact from a QContactModel.

  The QContactSelector dialog allows selecting an existing or new contact.
*/

/*!
  Constructs a QContactSelector with parent \a parent.  If \a allowNew is true will also provide
  the abilityt to select that a new contact should be created.
*/
QContactSelector::QContactSelector(bool allowNew, QWidget *parent)
    : QDialog(parent)
{
    d = new QContactSelectorPrivate();
    d->mNewContactSelected = false;
    d->mContactSelected = false;
    setWindowTitle( tr("Select Contact") );
    QVBoxLayout *l = new QVBoxLayout( this );

    d->view = new QContactListView( this );
    d->view->setItemDelegate(new QContactDelegate(d->view));
    d->view->setSelectionMode( QListView::SingleSelection );
    connect( d->view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(setSelected(const QModelIndex&)) );
    connect( d->view, SIGNAL(activated(const QModelIndex&)), this, SLOT(setSelected(const QModelIndex&)) );

    l->addWidget( d->view );

    if( allowNew )
    {
#ifndef QTOPIA_PHONE
        //TODO pda
#else
        QMenu *menu = QSoftMenuBar::menuFor( this );
        menu->addAction( QIcon(":icon/new"), tr("New"), this, SLOT(setNewSelected()) );
#endif
    }

#ifndef QTOPIA_DESKTOP
#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike( this, true );
#endif
#endif
}

/*!
  \internal
  Accepts the dialog and indicates a that a new contact should be created.
*/
void QContactSelector::setNewSelected()
{
    d->mNewContactSelected = true;
    accept();
}

/*!
  \internal
  Accepts the dialog and indicates a that a contact at \a idx in the model was selected.
*/
void QContactSelector::setSelected(const QModelIndex& idx)
{
    if (idx.isValid())
    {
        d->view->setCurrentIndex(idx);
        d->mContactSelected = true;
    }
    accept();
}

/*!
  Sets the model of representing contacts that should be selected from to \a m.
*/
void QContactSelector::setModel(QContactModel *m)
{
    d->view->setModel(m);
}

/*!
  Returns true if the dialog was accepted with a new contact selected.
  Otherwise returns false.
*/
bool QContactSelector::newContactSelected() const
{
    if (result() == Rejected)
        return false;
    return d->mNewContactSelected;
}

/*!
  Returns true if the dialog was accepted with an existing contact selected.
  Otherwise returns false.
*/
bool QContactSelector::contactSelected() const
{
    return d->mContactSelected;
}

/*!
  Returns the contact that was selected.  If no contact was selected returns a null contact.
*/
QContact QContactSelector::selectedContact() const
{
    QContactModel *m = qobject_cast<QContactModel *>(d->view->model());

    if (result() == Rejected || d->mNewContactSelected || !m || !d->view->currentIndex().isValid())
        return QContact();

    return m->contact(d->view->currentIndex());
}


/***************************
  * Phone type Selector
  ***********************/

class QPhoneTypeSelectorPrivate
{
public:
    QPhoneTypeSelectorPrivate(const QContact &cnt, const QString &number)
        : mToolTip(0), mContact( cnt ), mNumber(number) {}

    QLabel *mLabel, *mToolTip;
    QListWidget *mPhoneType;
    QMap<QListWidgetItem *, QContact::PhoneType> mItemToPhoneType;
    const QContact mContact;
    const QString mNumber;
};

/*!
  \class QPhoneTypeSelector
  \module qpepim
  \ingroup pim
  \brief The QPhoneTypeSelector class a way of selecting a single phone number from or for
  a QContact.

  QPhoneTypeSelector provides a way of selecting a single phone number from or for
  a QContact.
*/

/*!
  \fn void QPhoneTypeSelector::selected(QContact::PhoneType type)

    This signal is emitted when the user selectes a phone number \a type.
*/

/*!
  Contructs a QPhoneTypeSelector dialog with parent \a parent.
  The QPhoneTypeSelector will show phone numbers
  if \a number is empty will only show numbers for which there already exists and entry for
  the \a contact provided.  Otherwise will provide a selection of all possible phone numbers.
*/
QPhoneTypeSelector::QPhoneTypeSelector( const QContact &contact, const QString &number,
        QWidget *parent )
    : QDialog( parent )
{
    d = new QPhoneTypeSelectorPrivate(contact, number);

    setWindowTitle(tr("Phone Type"));

    QVBoxLayout *l = new QVBoxLayout( this );
    d->mLabel = new QLabel( this );
    d->mLabel->setWordWrap( true );
    l->addWidget(d->mLabel);
    d->mPhoneType = new QListWidget( this );
    l->addWidget(d->mPhoneType);
    d->mPhoneType->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    init();

    d->mPhoneType->setFocus();
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() ) {
        if( d->mPhoneType->hasEditFocus() )
            d->mPhoneType->setEditFocus( true );
    }
    QtopiaApplication::setMenuLike(this, true);
#endif
}

/*!
  if \a number is empty returns the translation of "(empty)".  Otherwise returns \a number

  \internal
 */
QString QPhoneTypeSelector::verboseIfEmpty( const QString &number )
{
    if( number.isEmpty() )
        return tr("(empty)");
    return number;
}

/*!
  Returns the QContact::PhoneType that matches the phone number selected.
*/
QContact::PhoneType QPhoneTypeSelector::selected() const
{
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        return QContact::OtherPhone;
    return d->mItemToPhoneType[item];
}

/*!
  Returns the value set for the phone number selected.
*/
QString QPhoneTypeSelector::selectedNumber() const
{
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        return QString();
    return item->text();
}

/*!
  \internal
  Initializes state of the QPhoneTypeSelector
*/
void QPhoneTypeSelector::init()
{
    QListWidgetItem *item = 0;

    if (!d->mNumber.isEmpty()) //assume an empty number means we want to select a number
    {
        d->mLabel->setText( "<qt>"+tr("Please select the phone type for '%1'.", "%1 = mobile number or land line number").arg( d->mNumber )+"</qt>" );

        QList<QContact::PhoneType> phoneTypes = QContact::phoneTypes();

        bool first = true;
        foreach(QContact::PhoneType type, phoneTypes) {
            QString phoneNumber = d->mContact.phoneNumber(type);
            item =new QListWidgetItem(verboseIfEmpty(phoneNumber), d->mPhoneType, type);
            item->setIcon(QContact::phoneIcon(type));
            d->mItemToPhoneType[item] = type;
            if (first) {
                d->mPhoneType->setCurrentItem( item );
                first = false;
            }
        }
    } else {
        bool haveNumber = false;
        d->mPhoneType->installEventFilter(this);
        d->mLabel->setText( "<qt>"+tr("Choose phone number:")+"</qt>" );

        bool first = true;
        QList<QContact::PhoneType> ptypes = QContact::phoneTypes();
        foreach(QContact::PhoneType type, ptypes) {
            QString number = d->mContact.phoneNumber(type);
            if (!number.isEmpty()) {
                item = new QListWidgetItem( number, d->mPhoneType );
                item->setIcon(QContact::phoneIcon(type));
                d->mItemToPhoneType[item] = type;
                haveNumber = true;
                if (first) {
                    d->mPhoneType->setCurrentItem( item );
                    first = false;
                }
            }
        }

        if (!haveNumber) {
            d->mToolTip = new QLabel(tr("No phone number for this contact"), this);
            d->mToolTip->setWordWrap( true );
            d->mToolTip->setAlignment(Qt::AlignCenter);
            d->mToolTip->show();
        }
    }
    connect( d->mPhoneType, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(accept()) );
}

/*!
  \overload

  This event handler can be reimplemented in a subclass to receive widget
  resize events which are passed in the \a event parameter. When resizeEvent()
  is called, the widget already has its new geometry. The old size is
  accessible through QResizeEvent::oldSize().

  The widget will be erased and receive a paint event immediately after
  processing the resize event. No drawing need be (or should be) done inside
  this handler.
*/
void QPhoneTypeSelector::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (d->mToolTip)
        d->mToolTip->move((width()-d->mToolTip->width())/2, (height()-d->mToolTip->height())/2);
}

/*!
  Updates the \a contact so that the phone number selected is set to \a number.
*/
void QPhoneTypeSelector::updateContact(QContact &contact, const QString &number) const {
    contact.setPhoneNumber(selected(), number);
}

/*!
  \overload
*/
void QPhoneTypeSelector::accept()
{
    QDialog::accept();
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        emit selected( d->mItemToPhoneType[item] );
}
