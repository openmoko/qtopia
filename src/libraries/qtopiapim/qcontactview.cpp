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
  of the item. QContactModel::LabelRole is expected to be html formatted text and will be drawn
  at the top of the model start at the right of where the portrait role is, and ending to the
  left of the status icon if present, otherwise the right of the item.  If Room is available,
  text provided by QContactModel::SubLabelRole will be drawn as plain text below the label text.
*/

/*!
  Contstructs a QContactModelDelegate with parent \a parent.
*/
QContactDelegate::QContactDelegate( QObject * parent )
    : QAbstractItemDelegate(parent)
{
}

/*!
  Destroys a QContactModelDelegate.
*/
QContactDelegate::~QContactDelegate() {}

/*!
  \internal
  Provides an alternate font based of the \a start font.  Reduces the size of the returned font
  by at least step point sizes.  Will attempt a total of six point sizes beyond the request
  point size until a valid font size that differs from the starting font size is found.
*/
QFont QContactDelegate::differentFont(const QFont& start, int step) const
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
        int newSize = f.pointSize() + step;
        if ( newSize > 0 )
            f.setPointSize(f.pointSize()+step);
        else
            return start; // we cannot find a font -> return old one
        step += step < 0 ? -1 : +1;
        QFontMetrics fm(f);
        if ( fm.lineSpacing() != osize )
            break;
    }
    return f;
}

/*!
  Returns the font to use for painting the main label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QContactModel::LabelRole.

  By default returns the font of the style option \a o.
*/
QFont QContactDelegate::mainFont(const QStyleOptionViewItem &o) const
{
    return o.font;
}

/*!
  Returns the font to use for painting the sub label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QContactModel::SubLabelRole.

  By default returns a font at least two point sizes smaller of the font of the
  style option \a o.
*/
QFont QContactDelegate::secondaryFont(const QStyleOptionViewItem &o) const
{
    return differentFont(o.font, -2);
}

/*!
  \overload

  Paints the element at \a index using \a painter with style options \a option.
*/
void QContactDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
        const QModelIndex & index ) const
{
    // could these be roles of the model?  would have to add the 'secondary' roles
    // although preferable as a Qtopia or Qt role as they don't really
    // apply to contacts.
    QString text = index.model()->data(index, QContactModel::LabelRole).toString();
    QString secondaryText = index.model()->data(index, QContactModel::SubLabelRole).toString();

    QPixmap decoration = qvariant_cast<QPixmap>(index.model()->data(index, QContactModel::PortraitRole));
    QPixmap secondaryDecoration = qvariant_cast<QPixmap>(index.model()->data(index, QContactModel::StatusIconRole));
    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft ;

    painter->save();

    // fill rect based on row background
    // or assume can be left to list class
    bool selected = (option.state & QStyle::State_Selected) == QStyle::State_Selected;
    QBrush baseBrush = selected ? option.palette.highlight() : option.palette.base();
    QBrush textBrush = selected ? option.palette.highlightedText() : option.palette.text();
    QPalette modpalette(option.palette);

    modpalette.setBrush(QPalette::Text, textBrush);
    modpalette.setBrush(QPalette::Base, baseBrush);

    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());

    if (selected)
        painter->fillRect(option.rect, baseBrush);

    // set up fonts, fbold, fsmall, fsmallbold
    QFont fbold = mainFont(option);
    QFont fsmall = secondaryFont(option);

    int x = option.rect.x();
    int y = option.rect.y();
    int width = option.rect.width();
    int height = option.rect.height()-1;

    int offset = (height-decoration.height())/2;
    if ( rtl )
        painter->drawPixmap(width-decoration.width(), y+offset, decoration );
    else
        painter->drawPixmap(x, y+offset, decoration);

    x += QContact::thumbnailSize().width() + 2;
    width -= QContact::thumbnailSize().width() + 2;

    int bcardwidth = 0;
    int bcardbase = y;
    if (!secondaryDecoration.isNull()) {
        bcardwidth = secondaryDecoration.width();
        bcardbase = y+secondaryDecoration.height();
        offset = (height-secondaryDecoration.height())/2;
        if ( rtl )
            painter->drawPixmap( option.rect.x(), y+offset, secondaryDecoration );
        else
            painter->drawPixmap(x+width-bcardwidth, y+offset,secondaryDecoration);
    }

    // draw label bold
    painter->setFont(fbold);

    // fit inside available width;
    QFontMetrics fboldM(fbold);

    // somehow underline appropriate characters.
    // clm->markSearchedText(text);, if richtext, would be <u>...</u> but only first
    // QContactModel maybe could do a better job of this.

    QRect space;
    if ( rtl )
        space = QRect(option.rect.x() + bcardwidth, y, width-bcardwidth, height);
    else
        space = QRect(x, y, width - bcardwidth, height);

    // draw label/fileas
    QTextOption to;
    to.setWrapMode(QTextOption::WordWrap);
    to.setAlignment(QStyle::visualAlignment(qApp->layoutDirection(),
                Qt::AlignLeft));

    QString drawText;
    /* elidedText drops formatting from rich text */
#if 0
    if(fboldM.width(text) > space.width())  {
        drawText = elidedText(fboldM, space.width(), Qt::ElideRight, text);
    } else
#endif
        drawText = text;


    // Painting simple rich text, although all I need to do is deal with a bit of
    // an underline.
    QTextDocument document;
    document.setDefaultFont(fbold);
    document.setHtml(drawText); // or setPlainText or create using QTextCursor, etc.

    /* seemingly the only way to stop wrapping in a QTextDocument... */
    document.setPageSize(space.size().expandedTo(QSize(1000, 0)));

    QTextCursor cursor = document.rootFrame()->firstCursorPosition();
    QTextBlockFormat frmt = cursor.blockFormat();
    frmt.setAlignment(QStyle::visualAlignment( qApp->layoutDirection(), Qt::AlignLeft ));
    cursor.setBlockFormat( frmt );
    QTextCharFormat charFmt = cursor.charFormat();

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = modpalette;
    ctx.clip = QRect(0, 0, space.width(), space.height());
    painter->save();
    if (rtl) {
        painter->translate(space.x()-1000+space.width(), space.y());
        painter->setClipRect(1000-space.width(), 0, space.width(), space.height());
    } else {
        painter->translate(space.x(), space.y());
        painter->setClipRect(0, 0, space.width(), space.height());
    }
    document.documentLayout()->draw(painter, ctx);
    painter->restore();
    // was
    //painter->drawText(space, drawText, to);

    y+=fboldM.height();

    painter->setFont(fsmall);
    // could do this in loop similar to abtable paintCell.
    // however would need to keep a list of printed labels, and ensure
    // not printing duplicates.
    QFontMetrics fsmallM(fsmall);
    if (y + fsmallM.height() <= option.rect.bottom()) {
        // go for more text.

        // this could just be a list of 'try this field then that.
        if (y > bcardbase)
            bcardwidth = 0;

        if (!secondaryText.isEmpty()) {
            if (fsmallM.width(secondaryText) > space.width())
                drawText = elidedText(fsmallM, width-bcardwidth, Qt::ElideRight, secondaryText);
            else
                drawText = secondaryText;
            if ( rtl )
                painter->drawText(
                    QRect(option.rect.x()+bcardwidth, y, width-bcardwidth, height),
                    drawText, to );
            else
                painter->drawText(QRect(x, y, width-bcardwidth, height), drawText, to);
        }
    }
    painter->restore();
}

/*!
   \overload

   Returns the size hint for objects drawn with the delgate with style options \a option for item at \a index.
*/
QSize QContactDelegate::sizeHint(const QStyleOptionViewItem & option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);

    QFontMetrics fm(mainFont(option));
    QFontMetrics sfm(secondaryFont(option));

    return QSize(QContact::thumbnailSize().width() + fm.width("M")*10,
            qMax(QContact::thumbnailSize().height(), fm.height()+sfm.height())+1);
}
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
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(slotCurrentChanged(const QModelIndex &)));
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
void QContactListView::slotCurrentChanged(const QModelIndex& newIndex)
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
