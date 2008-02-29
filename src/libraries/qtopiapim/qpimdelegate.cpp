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

#include "qpimdelegate.h"
#include <QPainter>
#include <QAbstractItemModel>
#include <QDebug>
#include "qtopiaapplication.h"


class QPimDelegateData
{
public:
    int ignored; // Padding.
};


/*!
  \class QPimDelegate
  \module qpepim
  \ingroup pim
  \brief The QPimDelegate class provides an abstract delegate for
         rendering multiple lines of text.

  It is useful for drawing lists of PIM information in a somewhat consistent
  way.  It is subclassed for the specific types of PIM information (Contacts,
  Tasks, Alarms etc).

  QPimDelegate draws an item with the following features:
  \list
  \o a background
  \o an optional number of decorations
  \o a main line of text, displayed in bold at the top
  \o a variable number of subsidiary lines of text, each consisting of a
    'header' string rendered in bold, and a 'value' string rendered in a
    normal weight font.  The subsidiary lines of text are rendered slightly
    smaller than the main line of text.  Each sub line can be rendered
    independently, or all subsidiary lines in a given item can have the header
    and value strings lined up consistently.
  \o an optional foreground
  \endlist

  All text lines will be elided if they are too wide.

  This class is not intended to be used directly.  Subclasses are expected
  to override some or all of the customization functions providing the above
  pieces of information or styling settings.
*/

/*!
  Constructs a QPimDelegate, with parent object \a parent.
*/
QPimDelegate::QPimDelegate(QObject *parent)
    : QAbstractItemDelegate(parent), d(new QPimDelegateData)
{

}

/*!
 Destroys a QPimDelegate.
*/
QPimDelegate::~QPimDelegate()
{

}

/*!
  Returns the string to be displayed as the main text element of the
  delegate.  The default implementation returns the DisplayRole of the
  supplied \a index, and ignores \a option.
*/
QString QPimDelegate::mainText(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option);

    return index.model()->data(index, Qt::DisplayRole).toString();
}

/*!
  Given the size hint for the rendered text, return the size hint
  for the entire delegate, based on how any decorations are rendered.
  For example, if decorations are rendered on the left or right of the
  text, the given \a textSizeHint should be expanded horizontally by the
  width of any decorations, and the height should be expanded (if
  necessary) to the height of any decorations.

  The default implementation returns the supplied \a textSizeHint, and
  ignores the supplied \a index and \a option parameters.
 */
QSize QPimDelegate::decorationsSizeHint(
        const QStyleOptionViewItem& option, const QModelIndex& index,
        const QSize& textSizeHint) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return textSizeHint;
}


/*!
  Returns a hint for the number of subsidiary lines of text
  to render for an item, which is used to calculate the sizeHint
  of this delegate.

  The default implementation obtains the actual list of subsidiary
  lines of text to render with the supplied \a option and \a index,
  (\l {QPimDelegate::}{subTexts()}) and returns the size of this list.

  Override this if it can be slow to retrieve the actual list of
  subsidiary lines, but fast to estimate the number (for example,
  if all items are rendered with two subsidiary lines of text, but
  each subsidiary line of text requires a database lookup).

  \sa subTexts()
 */
int QPimDelegate::subTextsCountHint(
        const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return subTexts(option, index).count();
}

/*!
  Returns the list of subsidiary lines to render.  This is
  stored in a list of pairs of strings, with the first member
  of the pair being the "header" string, and the second member
  of the pair being the "value" string.  The "header" string
  will be rendered with a bold font, while the "value" string will
  be rendered with a normal weight font.  If either member of
  the pair is a null QString, then the subsidiary line is
  rendered with the full width and the corresponding font.

  The default implementation returns an empty list, and
  ignores the supplied \a index and \a option parameters.
 */
QList<StringPair> QPimDelegate::subTexts(
        const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QList<StringPair>();
}

/*!
  \enum QPimDelegate::BackgroundStyle

  Enumerates the ways that the background of an item can be rendered by the
  \l {QPimDelegate::}{drawBackground()} function.

  \value None
    No background will be drawn by this delegate.  The container view may
    still have applied a background.
  \value SelectedOnly
    Only the selected item will have its background drawn by the delegate.
    This uses the selected palette brush.
  \value Gradient
    The delegate draws a linear gradient from top to
    bottom in slightly different shades of either the style's
    highlight brush color (if the item is selected) or the base
    brush color.

  \sa backgroundStyle()
 */

/*!
  Returns the way that the background of this item should be rendered.

  The default implementation returns QPimDelegate::SelectedOnly, and
  ignores the supplied \a index and \a option parameters.
 */
QPimDelegate::BackgroundStyle QPimDelegate::backgroundStyle(
        const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QPimDelegate::SelectedOnly;
}

/*!
  \enum QPimDelegate::SubTextAlignment

  Enumerates the ways that the header and value pairs of the
  subsidiary lines can be rendered.

  \value Independent
    Each header and value pair is rendered independently, leading aligned, with
    the value string being rendered immediately after the header.
  \value CuddledPerItem
    All of the subsidiary lines of an item will be rendered with the same width
    for the header strings, and the header strings will be rendered trailing
    aligned within this space.  The value strings will be rendered leading
    aligned, so that the header and value strings for a subsidiary line will be
    cuddled together.

  \sa subTextAlignment()
*/

/*!
  Returns the alignment of the header and value pairs of the
  subsidiary lines of text.

  The default implementation returns QPimDelegate::Independent, and
  ignores the supplied \a index and \a option parameters.
 */
QPimDelegate::SubTextAlignment QPimDelegate::subTextAlignment(
        const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QPimDelegate::Independent;
}

/*!
  Paints any decorations, and return (by reference) two lists of rectangles
  used to align the painted text.  The \a rtl argument is a convenience
  parameter that is true if the decorations should be painted in a
  right-to-left manner, or false if they should be painted in a left-to right
  manner.  This may affect which side of the rectangle a decoration should
  be painted on.

  The rectangle to paint in (using \a p) should be obtained from
  \a option (\a {option}.rect), and the \a index of the item to paint.

  This function should return (in the \a leadingFloats and \a trailingFloats
  parameters) two lists of rectangles that rendered text should be excluded
  from.  The lists are used in the following way:  If a line of text
  would intersect the rectangles returned, the line of text's leading
  edge will be adjusted to the most trailing edge of any intersected
  \a leadingFloats rectangle, and to the most leading edge of any intersected
  \a trailingFloats rectangle.

  The default implementation does not draw anything, and returns two empty
  lists.

  \sa textRectangle()
 */
void QPimDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option,
                                   const QModelIndex& index,
                                   QList<QRect>& leadingFloats,
                                   QList<QRect>& trailingFloats) const
{
    Q_UNUSED(p);
    Q_UNUSED(rtl);
    Q_UNUSED(option);
    Q_UNUSED(index);
    Q_UNUSED(leadingFloats);
    Q_UNUSED(trailingFloats);
}

/*!
  Given the list of left (\a leftFloats) and right (\a rightFloats) decoration
  rectangles, and the \a entireRect rectangle describing the entire area, and
  the \a top pixel coordinate of a line of text, and the \a height of a line of
  text, return the rectangle that that line of text should be rendered in.

  Note that the \l {QPimDelegate::}{drawDecorations()} function returns lists of
  rectangles that are RTL independent (e.g. leading and trailing instead of
  left and right).  The lists of rectangles passed to this function are in
  absolute terms (left and right) - for an LTR language, they are equivalent, but
  for an RTL language the two lists will be exchanged.

  This function is used while rendering each line of text in the item (both
  the main line and any subsidiary lines).

  \sa drawDecorations()
 */
QRect QPimDelegate::textRectangle(const QRect& entireRect,
                                         const QList<QRect>& leftFloats,
                                         const QList<QRect>& rightFloats,
                                         int top, int height) const
{
    QRect r = QRect(entireRect.x(), top, entireRect.width(), height);
    foreach(QRect leftFloat, leftFloats) {
        if (leftFloat.right() > r.left() && r.intersects(leftFloat))
            r.setLeft(leftFloat.right());
    }
    foreach(QRect rightFloat, rightFloats) {
        if (rightFloat.left() < r.right() && r.intersects(rightFloat))
            r.setRight(rightFloat.left());
    }
    return r;
}


/*!
  Paints the background of the item.

  The rectangle to paint in (using \a p) should be obtained from
  \a option (\a {option}.rect), and the \a index of the item to paint.

  The default implementation fetches the background style to paint
  by calling \l {QPimDelegate::}{backgroundStyle()} for the given
  \a option and \a index.

 */
void QPimDelegate::drawBackground(QPainter *p,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex& index) const
{
    BackgroundStyle style = backgroundStyle(option, index);

    if (style == None)
        return;

    bool selected = (option.state & QStyle::State_Selected) == QStyle::State_Selected;
    QBrush baseBrush = selected ? option.palette.highlight() : option.palette.base();

    if (style == Gradient) {

        QLinearGradient bgg(0, option.rect.top(), 0, option.rect.bottom());
#if QT_VERSION >= 0x040300
        bgg.setColorAt(0.05f, baseBrush.color().lighter(120));  /* light still works, but is obsolete */
        bgg.setColorAt(0.8f, baseBrush.color().darker(200));
#else
        bgg.setColorAt(0.05f, baseBrush.color().light(120));
        bgg.setColorAt(0.8f, baseBrush.color().dark(200));
#endif
        p->setPen(Qt::NoPen);

        p->fillRect(option.rect, bgg);
    } else {
        if (selected)
            p->fillRect(option.rect, baseBrush);
    }
}

/*!
  Paints the foreground of the item.

  The rectangle to paint in (using \a p) should be obtained from
  \a option (\a {option}.rect), and the \a index of the item to paint.

  This function is called after painting all other visual elements
  (background, decorations, text etc) and could be used to apply a
  transparency effect to the rendered items.

  The default implementation does not paint anything.
 */
void QPimDelegate::drawForeground(QPainter *p,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex& index) const
{
    Q_UNUSED(p);
    Q_UNUSED(option);
    Q_UNUSED(index);
}


/*!
  Paints the item specified by \a index, using the supplied \a painter and
  style option \a option.

  The default implementation will first draw the background and decorations
  (\l {QPimDelegate::}{drawBackground()},
  \l {QPimDelegate::}{drawDecorations()}).  Then, the main text string
  is painted (using the font from \l {QPimDelegate::}{mainFont()}), and any
  subsidiary lines are painted (using the fonts from
  \l {QPimDelegate::}{secondaryFont()},
  and \l {QPimDelegate::}{secondaryHeaderFont()})
  according the the alignment given by \l {QPimDelegate::}{subTextAlignment()}.
  Finally, any foreground items will be drawn by
  \l {QPimDelegate::}{drawForeground()}.

  The bounding rectangle used for drawing each line of text is returned by
  \l {QPimDelegate::}{textRectangle()}.

  The entire text region will be drawn vertically centered if there is only a
  small amount of left over space (less than the line spacing of the main
  font).  Otherwise, the text region will be drawn vertically aligned to the
  top of the item.

  \sa drawBackground(), drawDecorations(), mainFont(),
      secondaryFont(), secondaryHeaderFont(),
      subTextAlignment(), textRectangle(), drawForeground()
 */
void QPimDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft ;

    QList<StringPair> subTexts = this->subTexts(option, index);
    SubTextAlignment subAlign = subTextAlignment(option, index);

    painter->save();

    bool sel = (option.state & QStyle::State_Selected)==QStyle::State_Selected;
    QBrush baseBrush = sel ? option.palette.highlight() : option.palette.base();
    QBrush textBrush = sel ? option.palette.highlightedText() : option.palette.text();

    // Draw the background and icons first
    QList<QRect> leftFloats;
    QList<QRect> rightFloats;
    drawBackground(painter, option, index);
    drawDecorations(painter, rtl, option, index, leftFloats, rightFloats);

    // Swap here so we don't need to for textRectangle
    if (rtl)
        qSwap(leftFloats, rightFloats);

    // Make sure we have a valid pen/brush
    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());

    int y = option.rect.y() + 1;
    int height = option.rect.height() - 2;
    QRect space;

    // select the header font
    QFont fmain = mainFont(option, index);
    QFontMetrics fmainM(fmain);
    painter->setFont(fmain);

    if (subTexts.count() < 1) {
        // We vertically center align this text if there CuddledPerListis less
        // than one line of extra space
        if (height < 2 * fmainM.lineSpacing())
            y += (height - fmainM.lineSpacing()) / 2;

        space = textRectangle(option.rect, leftFloats, rightFloats, y, fmainM.lineSpacing());
        painter->drawText(space, Qt::AlignLeading, fmainM.elidedText(mainText(option, index), option.textElideMode, space.width()));
    } else {
        /* Sub labels are present */
        QFont fsubheader = secondaryHeaderFont(option, index);
        QFont fsubvalue = secondaryFont(option, index);

        QFontMetrics fsubvalueM(fsubvalue);
        QFontMetrics fsubheaderM(fsubheader);
        StringPair subLine;

        // We vertically center align all the text if there is less than one line of extra space [as measured by the big font]
        if (height < (2 * fmainM.lineSpacing()) + (subTexts.count() * (fsubheaderM.lineSpacing() + 1)))
            y += (height - (fmainM.lineSpacing() + (subTexts.count() * (fsubheaderM.lineSpacing() + 1)))) / 2;

        space = textRectangle(option.rect, leftFloats, rightFloats, y, fmainM.lineSpacing());
        painter->drawText(space, Qt::AlignLeading, fmainM.elidedText(mainText(option, index), option.textElideMode, space.width()));
        y += fmainM.lineSpacing();

        int headerWidth = 0;
        /* First, calculate the width of all the header sections, if the style is cuddly */
        if (subAlign == CuddledPerItem) {
            foreach(subLine, subTexts) {
                if (!subLine.first.isEmpty()) {
                    int w = fsubheaderM.boundingRect(subLine.first).width();
                    if (w > headerWidth)
                        headerWidth = w;
                }
            }
        }

        /* Now draw! */
        QList<StringPair>::const_iterator it = subTexts.begin();
        while (y + fsubheaderM.lineSpacing() <= option.rect.bottom() && it != subTexts.end()) {
            space = textRectangle(option.rect, leftFloats, rightFloats, y, fsubheaderM.lineSpacing() + 1);
            subLine = *it++;
            if (!subLine.first.isNull()) {
                if (!subLine.second.isEmpty()) {
                    QRect headerRect = space;
                    QRect valueRect = space;

                    QString subText = fsubheaderM.elidedText(subLine.first, Qt::ElideRight, headerRect.width());
                    if (subAlign == Independent)
                        headerWidth = fsubheaderM.width(subText);

                    if (rtl) {
                        headerRect.setLeft(headerRect.right() - headerWidth);
                        valueRect.setRight(headerRect.left());
                    } else {
                        headerRect.setWidth(headerWidth);
                        valueRect.setLeft(headerRect.right());
                    }
                    painter->setFont(fsubheader);
                    painter->drawText(headerRect, Qt::AlignTrailing, subText);
                    painter->setFont(fsubvalue);
                    painter->drawText(valueRect, Qt::AlignLeading, fsubvalueM.elidedText(subLine.second, option.textElideMode, valueRect.width()));
                } else {
                    painter->setFont(fsubheader);
                    painter->drawText(space, Qt::AlignLeading, fsubheaderM.elidedText(subLine.first, option.textElideMode, space.width()));
                }
            } else {
                if (!subLine.second.isEmpty()) {
                    painter->setFont(fsubvalue);
                    painter->drawText(space, Qt::AlignLeading, fsubvalueM.elidedText(subLine.second, option.textElideMode, space.width()));
                }
            }

            y += fsubheaderM.lineSpacing() + 1;
        }
    }

    drawForeground(painter, option, index);
    painter->restore();
}

/*!
  Returns the font to use for painting the main label text of the item,
  for the given \a index and style option \a option.

  By default returns a bold version of the font of the style option
  \a option and ignores \a index.
 */
QFont QPimDelegate::mainFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QFont fmain = option.font;
    fmain.setWeight(QFont::Bold);

    return fmain;
}

/*!
  Returns the font to use for painting the subsidiary header texts of the item,
  for the given \a index and style option \a option.

  By default returns an at least two point size smaller and bold version
  of the font of the style option \a option and ignores the supplied \a index.
 */
QFont QPimDelegate::secondaryHeaderFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QFont fsubheader = differentFont(option.font, -2);
    fsubheader.setWeight(QFont::Bold);

    return fsubheader;
}

/*!
  Returns the font to use for painting the subsidiary value texts of the item,
  for the given \a index and style option \a option.

  By default returns an at least two point sizes smaller version of the
  font of the style option \a option and ignores the supplied \a index.
 */
QFont QPimDelegate::secondaryFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    return differentFont(option.font, -2);
}

/*!
  Given a starting font \a start, and a point size difference \a step,
  attempt to return a font that is similar to \a start except different
  in size by approximately \a step.  If no matching font for a given
  \a step value is found, it will try increasingly larger/smaller fonts
  (depending on whether \a step was originally positive or negative,
  and up to 6 in total).
 */
QFont QPimDelegate::differentFont(const QFont& start, int step) const
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
  Calculate the size hint for a specific index \a index and item style \a option.

  The text size hint is calculated from several pieces of information:
  \list
  \o the line spacing of the main font and the sub header fonts
  \o the number of subsidiary lines of text
  \endlist

  The text size hint is then passed to \l {QPimDelegate::}{decorationsSizeHint()}, which
  can modify the text hint as appropriate for the decorations drawn (taking into account
  whether the decoration will be beside or on top of the text, etc).

  \sa decorationsSizeHint()
 */
QSize QPimDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int subTextsCount = subTextsCountHint(option, index);

    QFont main = mainFont(option, index);
    QFont shfont = secondaryHeaderFont(option, index);

    QFontMetrics fm(main);
    QFontMetrics sfm(shfont);

    // This is the text size
    QSize sh(fm.width("M") * 10,1 + fm.lineSpacing() + 1 + ( subTextsCount * (sfm.lineSpacing() + 1)) + 1);

    // allow the decoration size hint to be included
    return decorationsSizeHint(option, index, sh);
}
