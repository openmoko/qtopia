/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef CHARLIST_H
#define CHARLIST_H

#include <qwidget.h>
#include <qbitmap.h>

class QFontMetrics;

class CharList : public QWidget
{
    Q_OBJECT
public:
    CharList(QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
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

