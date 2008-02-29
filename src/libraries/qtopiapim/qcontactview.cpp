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
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QContactDelegate class provides drawing of QContactModel items (\l {QContact}{QContacts}).

  By using QContactDelegate, applications dealing with QContacts can achieve a consistent
  look and feel.

  QContacts are drawn with four major sections per item.  There are icons on
  the left and right sides of the rendered item, and top and bottom lines of text.
  The data drawn is fetched from the model (which is assumed to be a QContactModel),
  using some additional Qt::ItemDataRole values defined in QContactModel.  The
  following image illustrates a list of \l{QContact}{QContacts} being displayed
  in a QContactListView using a QContactDelegate:

  \image qcontactview.png "List of QContacts"

  QContactModel::QContactModelRole defines the additional roles used to draw the items:

  \table 80 %
   \header
    \o Role
    \o Data Type
    \o Description
   \row
    \o QContactModel::PortraitRole
    \o QPixmap
    \o Drawn vertically centered on the leading side (left for LTR languages) of the item.
   \row
    \o Qt::DisplayRole
    \o QString
    \o Plain unformatted text drawn at the top of the item, between any icons.
   \row
    \o QContactModel::StatusIconRole
    \o QPixmap
    \o Optional. Drawn vertically centered on the trailing side (right for LTR languages) of the item.
   \row
    \o QContactModel::SubLabelRole
    \o QString
    \o Drawn as plain text below the label text, if space is available.
  \endtable

  The first four contacts shown in the picture above have the following data in the QContactModel:
  \table 80 %
   \header
    \o PortraitRole
    \o DisplayRole
    \o StatusIconRole
    \o SubLabelRole
   \row
    \o Pixmap of a person
    \o Adam Zucker
    \o <empty pixmap>
    \o 12345
   \row
    \o Pixmap of a SIM card
    \o Adam Zucker/h
    \o <empty pixmap>
    \o 12345
   \row
    \o Pixmap of a SIM card
    \o Adam Zucker/m
    \o <empty pixmap>
    \o 24685
   \row
    \o Pixmap of a person
    \o Bradley Young
    \o Pixmap of a briefcase
    \o 48759
  \endtable

  \sa QContact, QContactListView, QContactModel, QPimDelegate
*/

/*!
  Constructs a QContactDelegate with the given \a parent.
*/
QContactDelegate::QContactDelegate( QObject * parent )
    : QPimDelegate(parent)
{
}

/*!
  \reimp
  Returns a list consisting of a single string, that of the QContactModel::SubLabelRole for the
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
  \reimp
  Returns 1 (the QContactModel::SubLabelRole text).

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
  \reimp
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
  \reimp
  Always returns QPimDelegate::Independent, ignoring \a option and \a index.
*/
QPimDelegate::SubTextAlignment QContactDelegate::subTextAlignment(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QPimDelegate::Independent;
}

/*!
  \reimp
  Always returns QPimDelegate::SelectedOnly, ignoring \a option and \a index.
 */
QPimDelegate::BackgroundStyle QContactDelegate::backgroundStyle(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QPimDelegate::SelectedOnly;
}

/*!
  \reimp
  Returns the size hint for the delegate, given the \a option and \a index parameters.
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
  Destroys a QContactDelegate.
*/
QContactDelegate::~QContactDelegate() {}

/*!
  \class QContactListView
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QContactListView class provides a list view widget for use with QContactModel.

  The convenience functions provided by QContactListView include functions for interpreting
  the view's model, delegate and current item as the corresponding QContactModel, QContactDelegate and
  QContact objects.  In addition, QContactListView enforces using a QContactModel (or a derivative)
  as the model.

  Upon construction, QContactListView automatically sets itself to use a QContactDelegate for drawing,
  sets \c Batch layout mode (\l setLayoutMode()), and sets the resize mode to \c Adjust
  (\l setResizeMode()).

  The following image displays a QContactListView, using the
  default QContactDelegate to render QContacts from a QContactModel.

  \image qcontactview.png "List of QContacts"

  \sa QContact, QContactModel, QContactDelegate
*/

/*!
    \fn QContact QContactListView::currentContact() const

    Returns the QContact for the currently selected index.
*/

/*!
  \fn QContactModel *QContactListView::contactModel() const

  Returns the QContactModel set for the view.
*/

/*!
  \fn QContactDelegate *QContactListView::contactDelegate() const

  Returns the QContactDelegate set for the view.  During
  construction, QContactListView  will automatically create
  a QContactDelegate to use as the delegate, but this can be
  overridden with a different delegate derived from
  QContactDelegate if necessary.
*/

/*!
  Constructs a QContactListView with the given \a parent.

  This also sets the layout mode to \c Batched for performance,
  the resize mode to \c Adjust, and creates a \l QContactDelegate
  to use as the delegate.
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
  Destroys the QContactListView.
*/
QContactListView::~QContactListView()
{
}

/*!
  \overload

  Sets the model for the view to \a model.

  Will only accept the model if it inherits or is a QContactModel.
  If the \a model does not inherit a QContactModel, the existing
  model will be retained.
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
  instead.

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
  Returns the list of ids for contacts selected in the view.

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
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QContactSelector class provides a way of selecting a single contact from a QContactModel.

  In addition, the user can optionally be allowed to indicate they want to create a new contact,
  if none of the existing contacts are suitable.

  The following image displays a QContactSelector with the option to
  create a new QContact highlighted.

  \image qcontactselector.png "QContactSelector with the option to create a new contact highlighted"
*/

/*!
  Constructs a QContactSelector with the given \a parent.

  If \a allowNew is true, the selector provides
  the user an option to create a new QContact (see newContactSelected()).

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
  Accepts the dialog and indicates that a new contact should be created.
*/
void QContactSelector::setNewSelected()
{
    d->mNewContactSelected = true;
    d->mContactSelected = false;
    accept();
}

/*!
  \internal
  Accepts the dialog and indicates that a contact at \a idx in the model was selected.
*/
void QContactSelector::setSelected(const QModelIndex& idx)
{
    d->mNewContactSelected = false;
    if (idx.isValid())
    {
        d->view->setCurrentIndex(idx);
        d->mContactSelected = true;
    }
    accept();
}

/*!
  Sets the model providing the choice of contacts to \a model.
*/
void QContactSelector::setModel(QContactModel *model)
{
    d->view->setModel(model);
}

/*!
  Returns true if the dialog was accepted via the option to
  create a new contact.
  Otherwise returns false.

  \sa contactSelected()
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

  \sa newContactSelected()
*/
bool QContactSelector::contactSelected() const
{
    return d->mContactSelected;
}

/*!
  Returns the contact that was selected.  If no contact was selected returns a null contact.

  \sa contactSelected(), newContactSelected()
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
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QPhoneTypeSelector class provides a way of selecting a single type of phone number.

  The phone number types correspond to those defined in \l QContact::PhoneType.

  It can be used in one of two ways - either to allow the selection of one of a
  QContact's existing phone numbers, or to allow the user to pick the type of a
  new phone number (for example, to save a phone number as a 'work mobile' number).

  \table 80 %
  \row
  \o \inlineimage qphonetypeselector-existing.png "Picking an existing phone number"
  \o Picking an existing number from a QContact
  \row
  \o \inlineimage qphonetypeselector-new.png "Picking a new phone number type"
  \o Choosing a phone number type for a new phone number.  Note that the existing
     phone numbers for the QContact are displayed.
  \endtable

  \sa QContact
*/

/*!
  \fn void QPhoneTypeSelector::selected(QContact::PhoneType type)

    This signal is emitted when the user selects a phone number \a type.
*/

/*!
  Constructs a QPhoneTypeSelector dialog with parent \a parent.

  The dialog will show phone numbers and phone number types from the given \a contact.

  The dialog can be used in two ways:
  \list
  \o To choose an existing phone number from a contact, pass an empty string as the \a number argument.
  \o To choose a phone number type for a new phone number, pass the phone number as a string as the \a number argument.
  \endlist

  In the second case, the dialog will show any existing phone numbers for the \a contact in addition to
  the available phone number types.

  \sa updateContact()
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
  If \a number is empty returns the translation of "(empty)".  Otherwise returns \a number.

  \internal
 */
QString QPhoneTypeSelector::verboseIfEmpty( const QString &number )
{
    if( number.isEmpty() )
        return tr("(empty)");
    return number;
}

/*!
  Returns the QContact::PhoneType that is selected in the dialog.
  If no phone number type is selected, this will return \c OtherPhone.
*/
QContact::PhoneType QPhoneTypeSelector::selected() const
{
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        return QContact::OtherPhone;
    return d->mItemToPhoneType[item];
}

/*!
  Returns the contact's phone number that corresponds to the phone number type
  selected in the dialog.
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
  \reimp
*/
void QPhoneTypeSelector::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (d->mToolTip)
        d->mToolTip->move((width()-d->mToolTip->width())/2, (height()-d->mToolTip->height())/2);
}

/*!
  Updates \a contact to have the given \a number for the dialog's selected phone number type.

  \sa selected()
*/
void QPhoneTypeSelector::updateContact(QContact &contact, const QString &number) const {
    contact.setPhoneNumber(selected(), number);
}

/*!
  \reimp
*/
void QPhoneTypeSelector::accept()
{
    QDialog::accept();
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        emit selected( d->mItemToPhoneType[item] );
}
