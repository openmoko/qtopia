/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef PENSETTINGSWIDGET_H
#define PENSETTINGSWIDGET_H

#include <qwidget.h>
#include <qlist.h>
#include <qtopia/mstroke/char.h>

class QIMPenSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    QIMPenSettingsWidget( QWidget *parent, const char *name = 0 );
    ~QIMPenSettingsWidget();

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
    typedef QList<CharSetEntry *> CharSetEntryList;
    typedef QList<CharSetEntry *>::iterator CharSetEntryIterator;
    typedef QList<CharSetEntry *>::const_iterator CharSetEntryConstIterator;

protected:
    Mode mode;
    bool autoHide;
    bool readOnly;
    QPoint lastPoint;
    int pointIndex;
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
    QList<QRect> penMoves;
};

#endif
