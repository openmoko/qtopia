/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qtextoption.h"
#include "qapplication.h"
#include "qlist.h"

QT_BEGIN_NAMESPACE

struct QTextOptionPrivate
{
    QList<QTextOption::Tab> tabStops;
};

/*!
    Constructs a text option with default properties for text.
*/
QTextOption::QTextOption()
    : align(Qt::AlignLeft),
      wordWrap(QTextOption::WordWrap),
      design(false),
      unused(0),
      f(0),
      tab(-1),
      d(0)
{
    direction = QApplication::layoutDirection();
}

/*!
    Constructs a text option with the given \a alignment for text.
*/
QTextOption::QTextOption(Qt::Alignment alignment)
    : align(alignment),
      wordWrap(QTextOption::WordWrap),
      design(false),
      unused(0),
      f(0),
      tab(-1),
      d(0)
{
    direction = QApplication::layoutDirection();
}

/*!
    Destroys the text option.
*/
QTextOption::~QTextOption()
{
    delete d;
}

/*!
    \fn QTextOption::QTextOption(const QTextOption &other)

    Construct a copy of the \a other text option.
*/
QTextOption::QTextOption(const QTextOption &o)
    : align(o.align),
      wordWrap(o.wordWrap),
      design(o.design),
      direction(o.direction),
      unused(o.unused),
      f(o.f),
      tab(o.tab),
      d(0)
{
    if (o.d)
        d = new QTextOptionPrivate(*o.d);
}

/*!
    \fn QTextOption &QTextOption::operator=(const QTextOption &other)

    Returns true if the text option is the same as the \a other text option;
    otherwise returns false.
*/
QTextOption &QTextOption::operator=(const QTextOption &o)
{
    if (this == &o)
        return *this;
    delete d; d = 0;
    align = o.align;
    wordWrap = o.wordWrap;
    design = o.design;
    direction = o.direction;
    unused = o.unused;
    f = o.f;
    tab = o.tab;
    if (o.d)
        d = new QTextOptionPrivate(*o.d);
    return *this;
}

/*!
    Sets the tab positions for the text layout to those specified by
    \a tabStops.

    \sa tabArray(), setTabStop(), setTabs()
*/
void QTextOption::setTabArray(QList<qreal> tabStops)
{
    if (!d)
        d = new QTextOptionPrivate;
    QList<QTextOption::Tab> tabs;
    QTextOption::Tab tab;
    foreach (qreal pos, tabStops) {
        tab.position = pos;
        tabs.append(tab);
    }
    d->tabStops = tabs;
}

/*!
    \since 4.4
    Sets the tab positions for the text layout to those specified by
    \a tabStops.

    \sa tabStops()
*/
void QTextOption::setTabs(QList<QTextOption::Tab> tabStops)
{
    if (!d)
        d = new QTextOptionPrivate;
    d->tabStops = tabStops;
}

/*!
    Returns a list of tab positions defined for the text layout.

    \sa setTabArray(), tabStop()
*/
QList<qreal> QTextOption::tabArray() const
{
    if (!d)
        return QList<qreal>();

    QList<qreal> answer;
    QList<QTextOption::Tab>::ConstIterator iter = d->tabStops.constBegin();
    while(iter != d->tabStops.constEnd()) {
        answer.append( (*iter).position);
        ++iter;
    }
    return answer;
}


QList<QTextOption::Tab> QTextOption::tabs() const
{
    if (!d)
        return QList<QTextOption::Tab>();
    return d->tabStops;
}

/*!
    \class QTextOption
    \reentrant

    \brief The QTextOption class provides a description of general rich text
    properties.

    \ingroup text

    QTextOption is used to encapsulate common rich text properties in a single
    object. It contains information about text alignment, layout direction,
    word wrapping, and other standard properties associated with text rendering
    and layout.

    \sa QTextEdit, QTextDocument, QTextCursor
*/

/*!
    \enum QTextOption::WrapMode

    This enum describes how text is wrapped in a document.

    \value NoWrap       Text is not wrapped at all.
    \value WordWrap     Text is wrapped at word boundaries.
    \value ManualWrap   Wrapping occurs at a manually specified length from
                        the start of the line.
    \value WrapAnywhere Text can be wrapped at any point on a line, even if
                        it occurs in the middle of a word.
    \value WrapAtWordBoundaryOrAnywhere If possible, wrapping occurs at a word
                        boundary; otherwise it will occur at the appropriate
                        point on the line, even in the middle of a word.
*/

/*!
  \fn void QTextOption::setUseDesignMetrics(bool enable)

    If \a enable is true then the layout will use design metrics;
    otherwise it will use the metrics of the paint device (which is
    the default behavior).

    \sa useDesignMetrics()
*/

/*!
  \fn bool QTextOption::useDesignMetrics() const

    Returns true if the layout uses design rather than device metrics;
    otherwise returns false.

    \sa setUseDesignMetrics()
*/

/*!
  \fn Qt::Alignment QTextOption::alignment() const

  Returns the text alignment defined by the option.

  \sa setAlignment()
*/

/*!
  \fn void QTextOption::setAlignment(Qt::Alignment alignment);

  Sets the option's text alignment to the specified \a alignment.

  \sa alignment()
*/

/*!
  \fn Qt::LayoutDirection QTextOption::textDirection() const

  Returns the direction of the text layout defined by the option.

  \sa setTextDirection()
*/

/*!
  \fn void QTextOption::setTextDirection(Qt::LayoutDirection direction)

  Sets the direction of the text layout defined by the option to the
  given \a direction.

  \sa textDirection()
*/

/*!
  \fn WrapMode QTextOption::wrapMode() const

  Returns the text wrap mode defined by the option.

  \sa setWrapMode()
*/

/*!
  \fn void QTextOption::setWrapMode(WrapMode mode)

  Sets the option's text wrap mode to the given \a mode.
*/

/*!
  \enum QTextOption::Flag

  \value IncludeTrailingSpaces
*/

/*!
  \fn Flags QTextOption::flags() const

  Returns the flags associated with the option.

  \sa setFlags()
*/

/*!
  \fn void QTextOption::setFlags(Flags flags)

  Sets the flags associated with the option to the given \a flags.

  \sa flags()
*/

/*!
  \fn qreal QTextOption::tabStop() const

  Returns the distance in device units between tab stops.
  Convenient function for the above method

  \sa setTabStop(), tabArray(), setTabs(), tabs()
*/

/*!
  \fn void QTextOption::setTabStop(qreal tabStop)

  Sets the default distance in device units between tab stops to the value specified
  by \a tabStop.

  \sa tabStop(), setTabArray(), setTabs(), tabs()
*/

/*!
    \enum QTextOption::TabType
    \since 4.4

    This enum holds the different types of tabulator

    \value LeftTab,     A left-tab
    \value RightTab,    A right-tab
    \value CenterTab,   A centered-tab
    \value DelimiterTab A tab stopping at a certain delimiter-character
*/

/*!
    \class QTextOption::Tab
    \since 4.4
    Each tab definition is represented by this struct.
*/

/*!
    \variable Tab::position
    Distance from the start of the paragraph.
    The position of a tab is from the start of the paragraph which implies that when
    the alignment of the paragraph is set to centered, the tab is interpreted to be
    moved the same distance as the left ege of the paragraph does.
    In case the paragraph is set to have a layoutDirection() RightToLeft the position
    is interpreted to be from the right side of the paragraph with higher numbers moving
    the tab to the left.
*/

/*!
    \variable Tab::type
    Determine which type is used.
    In a paragraph that has layoutDirection() RightToLeft the type LeftTab will
    be interpreted to be a RightTab and vice versa.
*/

/*!
    \variable Tab::delimiter
    If type is DelimitorTab; tab until this char is found in the text.
*/

/*!
    \fn Tab::Tab()
    Creates a default left tab with position 80.
*/

/*!
    \fn bool Tab::operator==(const Tab &other) const

    Returns true if tab \a other is equal to this tab;
    otherwise returns false.
*/

/*!
    \fn bool Tab::operator!=(const Tab &other) const

    Returns true if tab \a other is not equal to this tab;
    otherwise returns false.
*/

/*!
  \fn void setTabs(QList<Tab> tabStops)
  Set the Tab properties to \a tabStops.

  \sa tabStop(), tabs()
*/

/*!
  \since 4.4
  \fn QList<QTextOption::Tab> QTextOption::tabs() const
  Returns a list of tab positions defined for the text layout.

  \sa tabStop(), setTabs(), setTabStop()
*/


QT_END_NAMESPACE
