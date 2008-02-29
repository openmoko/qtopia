/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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


#include "emaillistitem.h"
#include "maillistview.h"

#include <qtimestring.h>

#include <qstring.h>
#include <qfont.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qicon.h>

#include <stdlib.h>


static QPixmap* pm_normal = 0;
static QPixmap* pm_toget = 0;
static QPixmap* pm_tosend = 0;
static QPixmap* pm_unread = 0;
static QPixmap* pm_unfinished = 0;

static QPixmap* pm_mms = 0;
static QPixmap* pm_ems = 0;
static QPixmap* pm_sms = 0;
static QPixmap* pm_email = 0;

static QFont* nameFont = 0;
static QFont* subjectFont = 0;

static void ensurePixmaps()
{
    if ( !pm_normal ) {
        pm_normal = new QPixmap(":image/flag_normal");
        pm_unread = new QPixmap(":image/flag_unread");
        pm_toget = new QPixmap(":image/flag_toget");
        pm_tosend = new QPixmap(":image/flag_tosend");
        pm_unfinished = new QPixmap(":image/flag_unfinished");

        int extent = qApp->style()->pixelMetric(QStyle::PM_SmallIconSize);
        pm_mms = new QPixmap(QIcon(":icon/multimedia").pixmap(extent));
        pm_ems = new QPixmap(*pm_mms);   // Same as MMS icon
        pm_sms = new QPixmap(QIcon(":icon/txt").pixmap(extent));
        pm_email = new QPixmap(QIcon(":icon/email").pixmap(extent));
    }
}

/*  This method must only be called from the EmailClient destructor (otherwise
    nasty things will happen    */
void EmailListItem::deletePixmaps()
{
    delete pm_normal;
    delete pm_unread;
    delete pm_toget;
    delete pm_tosend;
    delete pm_unfinished;
    delete pm_mms;
    delete pm_ems;
    delete pm_sms;
    delete pm_email;
}

EmailListItem::EmailListItem(MailListView *parent, Email *mailIn, int col)
    : QTableWidgetItem(), typePm(0), alt(0), mCol( col )
{
    ensurePixmaps();
    _mail = mailIn;
    parentView = (MailListView *) parent;  //typecasting parent() fails, so we must store it
    columnsSet = false;
    setFlags( flags() & ~Qt::ItemIsEditable );

    setText( _mail->subject() + " " + _mail->from() ); // DEBUGGING
}

EmailListItem::~EmailListItem()
{
}

bool EmailListItem::operator<(const QTableWidgetItem &other) const
{
    EmailListItem *otherItem = (EmailListItem *)&other;

    //Qtablewidget work around. Fails sort on invisible columns
    //always sort visible column

//     if ( mCol == 0 ) {
//         return key(mCol).compare( otherItem->key(mCol) );
//     } else if (mCol == 3) {  //date
    if(mCol == 0){
        //sort by arrival time, (date might not always yield the logical result)
        if ( parentView->arrivalDate() ) {
            uint myId = _mail->arrival();
            uint theirId = otherItem->mail()->arrival();

        return myId < theirId;
        }

        QDateTime myDate( _mail->dateTime() );
        QDateTime theirDate( otherItem->mail()->dateTime() );

        if ( myDate == theirDate )
            return 0;

        if ( myDate < theirDate )
            return -1;

        return 1;
    }

    return text().toLower().localeAwareCompare( other.text().toLower() );
}

QString EmailListItem::key(int c) const
{
    QString orderFix = QString::number(_mail->uuid().data1);
    if ( c == 0 ) {
        if ( _mail->status(EFlag_Incoming) ) {
            // order: new top, not downloaded, read
            if ( _mail->status(EFlag_Downloaded) ) {
                if (_mail->status(EFlag_Read) ) {
                    return ("1" + orderFix);
                } else {
                    return ("3" + orderFix);
                }
            } else {
                return ("2" + orderFix);
            }
        } else {
            if ( _mail->status(EFlag_Sent) ) {
                return ("1" + orderFix);
            } else {
                return ("2" + orderFix);
            }
        }
    } else if ( c == 1 ) {
        if ( _mail->status(EFlag_Incoming) )
            return _mail->fromName().toLower() + orderFix;
        else
            return _mail->to().first() + orderFix;
    } else if ( c == 2 ) {
        return _mail->subject().toLower() + orderFix;
    } else if (c == 3) {        //date
        QString str;

        //sort by arrival time, (date might not always yield the logical result)
        if ( parentView->arrivalDate() ) {
            str.sprintf("%08d", _mail->arrival() );
        } else {
            QDateTime epoch( QDate( 1980, 1, 1 ) );
            str.sprintf( "%08d", epoch.secsTo( _mail->dateTime() ) );
        }

        return str + orderFix;
    }

    // should never get here
    return text().toLower() + orderFix;
}

void EmailListItem::updateState()
{
    setColumns();
}

QString EmailListItem::dateToString( QDateTime dateTime )
{
    QDate endWeek = QDate::currentDate();
    endWeek.addDays( 7 - endWeek.dayOfWeek() );

    int daysTo = abs( dateTime.date().daysTo(endWeek) );
    if ( daysTo < 7 ) {
        if ( QDate::currentDate() == dateTime.date() ) {
            QString day = MailListView::tr("Today %1").arg( QTimeString::localHM( dateTime.time() ) );
            return day;
        } else if ( dateTime.daysTo(QDateTime::currentDateTime()) == 1 ) {
            return MailListView::tr("Yesterday");
        } else {
            return QTimeString::localDayOfWeek(dateTime.date());
        }
    } else {
        return QTimeString::localYMD( dateTime.date() );
    }
}

void EmailListItem::setColumns()
{
    QString temp;

    setText( _mail->subject() + " " + _mail->from() ); // DEBUGGING

    if ( _mail->status(EFlag_Incoming) ) {
#ifdef QTOPIA_PHONE
        if (true) {
#else
        if (mCol == 0) {
#endif
            if ( _mail->status(EFlag_Downloaded) ) {
                if ( _mail->status(EFlag_Read) ) {
                    setIcon(*pm_normal);
                } else {
                    setIcon(*pm_unread);
                }
            } else {
                setIcon(*pm_toget);
            }
        } else if (mCol == 1) {
            QString from = _mail->fromName();
            setText(from);
        }
    } else {
#ifdef QTOPIA_PHONE
        if (true) {
#else
        if (mCol == 0) {
#endif
            if ( _mail->status(EFlag_Sent) ) {
                    setIcon(*pm_normal);
            } else if ( _mail->unfinished() ) {
                setIcon(*pm_unfinished);
            } else {
                setIcon(*pm_tosend);
            }
        } else if (mCol == 1 ) {

            int recipientCount = _mail->to().count() + _mail->cc().count() + _mail->bcc().count();
            if (_mail->to().count() > 0) {
                temp = _mail->to().first();
                if (recipientCount > 1)
                    temp += "...";
            }

            setText(temp);
        }
    }

    if (mCol == 2) {
        setText(_mail->subject() );
#if QTOPIA4_TODO
        parentView->ensureWidthSufficient( text( 2 ) );
#endif
    }

    if (mCol == 3) {
        QDate date = _mail->dateTime().date();
        if ( !date.isNull() ) {
            setText( dateToString( _mail->dateTime() ) );
        } else {
            setText( _mail->dateString() );
        }
    }

    if (_mail->status(EFlag_TypeMms))
        typePm = pm_mms;
    else if (_mail->status(EFlag_TypeEms))
        typePm = pm_ems;
    else if (_mail->status(EFlag_TypeSms))
        typePm = pm_sms;
    else if (_mail->status(EFlag_TypeEmail))
        typePm = pm_email;

    columnsSet = true;
}

Email* EmailListItem::mail()
{
    return _mail;
}

void EmailListItem::setMail(Email *newMail)
{
    _mail = newMail;

    setColumns();
    parentView->update( parentView->visualItemRect( this ) );
}

QUuid EmailListItem::id()
{
    return _mail->uuid();
}

EmailListItemDelegate::EmailListItemDelegate(MailListView *parent)
    : QItemDelegate(parent),
      mParent(parent)
{
}

void EmailListItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    EmailListItem *item = mParent->emailItemFromIndex( index );
    if (item && item->mail()) {
        Q_ASSERT(index.isValid());
        const QAbstractItemModel *model = index.model();
        Q_ASSERT(model);

        QStyleOptionViewItem opt = option;

        // Set color group
        opt.palette.setCurrentColorGroup(option.state & QStyle::State_Enabled
                                         ? QPalette::Active : QPalette::Disabled);

        // set font
        QVariant value = model->data(index, Qt::FontRole);
        if (value.isValid())
            opt.font = qvariant_cast<QFont>(value);

        // set text alignment
        value = model->data(index, Qt::TextAlignmentRole);
        if (value.isValid())
            opt.displayAlignment = QFlag(value.toInt());

        // set text color
        value = model->data(index, Qt::TextColorRole);
        if (value.isValid() && qvariant_cast<QColor>(value).isValid())
            opt.palette.setColor(QPalette::Text, qvariant_cast<QColor>(value));

        // do layout

        // decoration
        if ( !item->stateUpdated() )
            item->updateState();

        value = model->data(index, Qt::DecorationRole);
        QPixmap pixmap1 = decoration(opt, value); // flag (un)read/toget/tosend
        QPixmap pixmap2; // type email/sms/mm/ems
        if ( item->pixmap() ) {
            pixmap2 = *item->pixmap();
        }

        QRect pixmapRect1 = (pixmap1.isNull() ? QRect(0, 0, 0, 0)
                            : QRect(QPoint(0, 0), option.decorationSize));
        QRect pixmapRect2 = (pixmap2.isNull() ? QRect(0, 0, 0, 0)
                            : QRect(QPoint( 0, 0 ),
                                    QPoint( pixmap2.size().width(),
                                            pixmap2.size().height() ) ) );

        // display
        if (!nameFont) {
            nameFont = new QFont( opt.font );
            subjectFont = new QFont( *nameFont );
            nameFont->setBold( true );
            subjectFont->setPointSize( nameFont->pointSize() * 3 / 4 );
        }

        QRect nameRect;
        QString name;
        if ( item->mail()->status(EFlag_Incoming) ) {
            QString from = item->mail()->fromName();
            QContactModel *model = mParent->contactModel();
            name = item->mail()->displayName( model, from.simplified() );
        } else {
            int recipientCount = item->mail()->to().count() +
                                 item->mail()->cc().count() +
                                 item->mail()->bcc().count();
            QString temp;
            if ( item->mail()->to().count() > 0) {
                temp = item->mail()->to().first();
                QContactModel *model = mParent->contactModel();
                temp = item->mail()->displayName( model, temp.simplified() );
                if (recipientCount > 1)
                    temp += "...";
            }
            name = temp.simplified();
        }

        if (!name.isEmpty())
        {
            QRectF result;
            painter->setFont(*nameFont);
            painter->drawText(option.rect,
                              Qt::TextDontPrint|Qt::TextDontClip,
                              name, &result);
            nameRect = result.toRect();
            nameRect.setWidth( nameRect.width() + 6 );//anti-elide fudge
        }

        QRect subjectRect;
        QString subject = item->mail()->subject().simplified();

        if (!subject.isEmpty())
        {
            QRectF result;
            painter->setFont(*subjectFont);
            painter->drawText(option.rect,
                              Qt::TextDontPrint|Qt::TextDontClip,
                              subject, &result);
            subjectRect = result.toRect();
        }

        int margin = 2;
        pixmapRect1.translate( margin, margin + opt.rect.top()  );
        pixmapRect2.translate( pixmapRect1.right() + margin,
                               margin + opt.rect.top() );
        nameRect.translate( pixmapRect2.right() + margin, margin );
        subjectRect.translate( pixmapRect2.right() + margin, nameRect.height() );
        subjectRect.setWidth( subjectRect.width() + 6 );//anti-elide fudge

        // handle rtl drawing
        if (opt.direction == Qt::RightToLeft) {
            pixmapRect1.setRect( opt.rect.left() + opt.rect.width() - pixmapRect1.width(), pixmapRect1.top(),
                                 pixmapRect1.width(), pixmapRect1.height() );
            pixmapRect2.setRect( pixmapRect1.left() - pixmapRect2.width(), opt.rect.top(),
                                 pixmapRect2.width(), pixmapRect2.height() );
            nameRect.setRect( pixmapRect2.left() - nameRect.width(), nameRect.top(),
                              nameRect.width(), nameRect.height() );
            subjectRect.setRect( pixmapRect2.left() - subjectRect.width(), subjectRect.top(),
                                 subjectRect.width(), subjectRect.height() );
        }

        // draw the background color
        if (option.showDecorationSelected &&
            (option.state & QStyle::State_Selected)) {
            QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                      ? QPalette::Normal : QPalette::Disabled;
            painter->fillRect(option.rect,
                              option.palette.brush(cg, QPalette::Highlight));
        } else {
            value = model->data(index, Qt::BackgroundColorRole);
            if (value.isValid() && qvariant_cast<QColor>(value).isValid())
                painter->fillRect(option.rect, qvariant_cast<QColor>(value));
        }

        // draw the item
        if (pixmapRect1.isValid())
            drawDecoration(painter, opt, pixmapRect1, pixmap1);
        if (pixmapRect2.isValid())
            drawDecoration(painter, opt, pixmapRect2, pixmap2);

        // avoid drawing highlight color twice.
        opt.palette.setColor(QPalette::Highlight, QColor(0,0,0,0));

        if (!name.isEmpty()) {
            drawDisplay(painter, opt, nameRect, name);
            drawFocus(painter, opt, nameRect);
        }
        if (!subject.isEmpty()) {
            QStyleOptionViewItem subjectOpt = opt;
            subjectOpt.font = *subjectFont;
            drawDisplay(painter, subjectOpt, subjectRect, subject);
            drawFocus(painter, subjectOpt, subjectRect);
        }
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}
