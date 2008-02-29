/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qwidget.h>
#include <qlist.h>
#include <qtopia/mstroke/char.h>

class QIMPenWidget : public QWidget
{
    Q_OBJECT
public:
    QIMPenWidget( QWidget *parent );

    void clear();
    void greyStroke();
    void setReadOnly( bool r ) { readOnly = r; }

    void insertCharSet( QIMPenCharSet *cs, int stretch=1, int pos=-1 );
    void removeCharSet( int );
    void changeCharSet( QIMPenCharSet *cs, int pos );
    void clearCharSets();
    void showCharacter( QIMPenChar *, int speed = 10 );
    virtual QSize sizeHint();

public slots:
    void removeStroke();

signals:
    void changeCharSet( QIMPenCharSet *cs );
    void changeCharSet( int );
    void beginStroke();
    void stroke( QIMPenStroke *ch );

protected slots:
    void timeout();

protected:
    enum Mode { Waiting, Input, Output };
    bool selectSet( QPoint );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void paintEvent( QPaintEvent *e );
    virtual void resizeEvent( QResizeEvent *e );

    struct CharSetEntry {
	QIMPenCharSet *cs;
	int stretch;
    };
    typedef QList<CharSetEntry> CharSetEntryList;
    typedef QListIterator<CharSetEntry> CharSetEntryIterator;

protected:
    Mode mode;
    bool autoHide;
    bool readOnly;
    QPoint lastPoint;
    unsigned pointIndex;
    int strokeIndex;
    int currCharSet;
    QTimer *timer;
    QColor strokeColor;
    QRect dirtyRect;
    QIMPenChar *outputChar;
    QIMPenStroke *outputStroke;
    QIMPenStroke *inputStroke;
    QIMPenStrokeList strokes;
    CharSetEntryList charSets;
    int totalStretch;
};

