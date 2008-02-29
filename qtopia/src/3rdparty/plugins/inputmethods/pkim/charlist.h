/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CHARLIST_H
#define CHARLIST_H

#include <qwidget.h>
#include <qbitmap.h>

class QFontMetrics;

class CharList : public QWidget
{
    Q_OBJECT
public:
    CharList(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~CharList();

    void setMicroFocus( int x, int y );
    void setAppFont(const QFont &f) { appFont = f; }
    void setChars(const QStringList &ch);
    void setCurrent(const QString &ch);

protected:
    void paintEvent(QPaintEvent*);

private:
    int cellHeight;
    int cellWidth;
    QFont appFont;
    QFontMetrics *fm;
    QStringList chars;
    QString current;
};

#endif

