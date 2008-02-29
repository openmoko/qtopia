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

#include "messagebox_p.h"
#include <QApplication>
#include <QPalette>
#include <QPaintDevice>
#include <QDesktopWidget>


/*!
  \internal
  \class MessageBox

  \brief A transparent box of text, designed to give users temporary information during
  the game.
*/


/*!
  \internal
  \fn MessageBox::MessageBox()
*/
MessageBox::MessageBox()
    : textItem(0)
{
    // Create the text item that will sit inside the box.
    // According to the ~QGraphicsItem documentation: "Destroys the QGraphicsItem and all its children."
    // Therefore, there should be no memory leak here.
    textItem = new QGraphicsTextItem;
    textItem->setParentItem(this);
    textItem->setPos(HBORDER,0);

    QColor bgColour = QApplication::palette().color(QPalette::Normal,QPalette::Highlight);
    bgColour.setAlphaF(0.5);
    setBrush(bgColour);
    textItem->setDefaultTextColor(QApplication::palette().color(QPalette::Normal, QPalette::HighlightedText));

    setTextSize();
}

// If the defaultValue is set for the defaultResolution, returns a new value based
// on the current resolution.
int MessageBox::getAdjustedForResolution(int defaultValue,int defaultResolution)
{
    int logicalDpiX = QApplication::desktop()->logicalDpiX();
    if ( logicalDpiX != defaultResolution ) {
        return qRound(defaultValue * logicalDpiX/defaultResolution);
    }

    return defaultValue;
}

// Called by ctor. Sets the text size of the message box, according to the current resolution.
void MessageBox::setTextSize()
{
    QFont font(QApplication::font()); // = textItem->font();
    font.setBold(true);
    textItem->setFont(font);
}

/*!
  \internal
  \fn void MessageBox::setMessage(const QString &msg,int width)
  \a msg: THe new message which is to be displayed.
  \a width: The maximum width of the MessagBox.
*/
void MessageBox::setMessage(const QString &msg,int w)
{
    int border = getAdjustedForResolution(HBORDER,DEFAULT_DPI);

    textItem->setPlainText(msg);

    w -= (border * 2);

    textItem->adjustSize();
    if ( textItem->boundingRect().width() >= w ) {
        textItem->setTextWidth(w - 1);
    }

    setRect(0,0,textItem->boundingRect().width(),textItem->boundingRect().height());
}

