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
#include <qtopia/pim/qappointmentmodel.h>
#include <QPainter>
#include <QMap>
#include <QList>
#include <QSet>
#include <QPixmap>
#include <QTextDocument>
#include <QTimer>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QStyle>
#include <qappointmentview.h>
#include <QDebug>

/*!
  Constructs a QAppointmentModelDelegate with parent \a parent.
*/
QAppointmentDelegate::QAppointmentDelegate( QObject * parent )
    : QAbstractItemDelegate(parent)
{
    iconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
}

/*!
  Destroys a QAppointmentModelDelegate.
*/
QAppointmentDelegate::~QAppointmentDelegate() {}

/*!
  \internal
  Provides an alternate font based of the \a start font.  Reduces the size of the returned font
  by at least step point sizes.  Will attempt a total of six point sizes beyond the request
  point size until a valid font size that differs from the starting font size is found.
*/
QFont QAppointmentDelegate::differentFont(const QFont& start, int step) const
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
  QAbstractItemModel::data() where role is QAppointmentModel::LabelRole.

  By default returns the font of the style option \a o.
*/
QFont QAppointmentDelegate::mainFont(const QStyleOptionViewItem &o) const
{
    return o.font;
}

/*!
  Returns the font to use for painting the sub label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QAppointmentModel::SubLabelRole.

  By default returns a font at least two point sizes smaller of the font of the
  style option \a o.
*/
QFont QAppointmentDelegate::secondaryFont(const QStyleOptionViewItem &o) const
{
    return differentFont(o.font, -2);
}

/*!
  \overload

  Paints the element at \a index using \a painter with style options \a option.
*/
void QAppointmentDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
        const QModelIndex & index) const
{
    //  Prepare brush + pen and draw in background rectangle

    QRect border;
    QPen pen(Qt::black);
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.highlight());
        border = option.rect.adjusted(1, 1, -1, -1);
        pen.setWidth(2);
    } else {
        painter->setBrush(qvariant_cast<QColor>(index.model()->data(index, Qt::BackgroundColorRole)).light(170));
        border = option.rect.adjusted(0, 0, -1, -1);
    }

    painter->setPen(pen);
    painter->drawRect(border);

    QRect contentRect = option.rect.adjusted(2, 2, -2, -2);
    QRect textRect = contentRect;

    painter->save();
    painter->setClipRect(contentRect);

    // We try to align the text and the icons so that they are vertically
    // centered if that does not result in too much whitespace (being
    // defined as more than one font linespacing high)

    // In addition, we try to wrap the text to the left boundary of the icons

    QString appText = index.model()->data(index, Qt::DisplayRole).toString();
    int fontHeight = QFontMetrics(option.font).lineSpacing();
    int rightTextEdge = contentRect.right();
    int vertPos = 0;
    QRect textMetrics = painter->boundingRect(textRect, Qt::AlignLeft | Qt::TextWrapAnywhere, appText);

    if (textMetrics.height() < contentRect.height() && textMetrics.height() > (contentRect.height() - fontHeight))
        vertPos = (contentRect.height() - textMetrics.height() + 1) / 2;

    //  Draw in the relevant event icons (vertically centered), with a 2px margin
    // If the text is not vertically centered, we don't center the icons either.
    QList<QVariant> icons = index.model()->data( index, Qt::DecorationRole ).toList();
    if (icons.count() > 0) {
        // We use the height of the text as the desired icon size (-2 for the padding)
        int drawnIconSize = fontHeight - 2;
        if (drawnIconSize <= 0)
            drawnIconSize = 1;

        // number of rows of icons drawn
        int numRows = (contentRect.height() + 2) / (drawnIconSize + 2);
        if (numRows > icons.count())
            numRows = icons.count();

        // vertical offset, if any (if more than one line spare, we top align)
        int iconVOffset = (contentRect.height() + 2 - (numRows * (drawnIconSize + 2)) + 1) / 2;
        if (vertPos == 0 || iconVOffset < 0 || iconVOffset >= contentRect.height() || iconVOffset > (drawnIconSize / 2))
            iconVOffset = 0;

        // Icon drawing position
        int iconY = iconVOffset;
        int iconX = contentRect.right() - drawnIconSize;

        // Now draw the icons, starting from top right, moving downwards and then leftwards
        for (QList<QVariant>::Iterator it = icons.begin(); it != icons.end(); ++it) {
            QIcon icon = qvariant_cast<QIcon>(*it);
            icon.paint(painter, iconX, contentRect.top() + iconY, drawnIconSize, drawnIconSize);

            // Save this
            rightTextEdge = iconX - 2;

            // Calculate where the next icon should go
            if(contentRect.height() >= iconY + drawnIconSize + 2 + drawnIconSize)
                iconY += drawnIconSize + 2;
            else {
                iconY = iconVOffset;
                iconX -= drawnIconSize + 2;
            }
        }
    }

    //  Prepare pen and draw in text
    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    // Now see if we can fit our text in.
    textRect.setRight(rightTextEdge);
    textRect.adjust(0, vertPos, 0, 0);

    painter->drawText(textRect, Qt::AlignLeft | Qt::TextWrapAnywhere, appText);
    painter->restore();
}

/*!
   \overload

   Returns the size hint for objects drawn with the delegate with style options \a option for item at \a index.
*/
QSize QAppointmentDelegate::sizeHint(const QStyleOptionViewItem & option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);

    QFontMetrics fm(mainFont(option));

#ifndef QTOPIA_PHONE
    QFontMetrics sfm(secondaryFont(option));
    return QSize(fm.width("M") * 10, fm.height() + sfm.height() + 4);
#else
    return QSize(fm.width("M") * 10, fm.height() + 4);  //  Make Qtopia phone more compact
#endif
}
