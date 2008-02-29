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

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QGraphicsRectItem>

class QGraphicsTextItem;


class MessageBox : public QGraphicsRectItem
{
public:

    // For a DPI of 100, the text size is 12pt. The resolution is calculated
    // from that ratio.
    static const int DEFAULT_TEXT_SIZE = 12;
    static const int DEFAULT_DPI = 100;

    MessageBox();

    void setMessage(const QString &msg,int width);

private:

    static const int HBORDER = 5;

    void setTextSize();

    //!!!!!!!!! COULD PUT THIS SOMEWHERE MORE COMMON??
    int getAdjustedForResolution(int defaultValue,int defaultResolution);

    QGraphicsTextItem *textItem;

};

#endif
