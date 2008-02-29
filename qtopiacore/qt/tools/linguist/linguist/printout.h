/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PRINTOUT_H
#define PRINTOUT_H

#include <QFont>
#include <QPainter>
#include <QRect>
#include <QList>
#include <QDateTime>

class QPrinter;
class QFontMetrics;

class PrintOut
{
public:
    enum Rule { NoRule, ThinRule, ThickRule };
    enum Style { Normal, Strong, Emphasis };

    PrintOut( QPrinter *printer );
    ~PrintOut();

    void setRule( Rule rule );
    void setGuide( const QString& guide );
    void vskip();
    void flushLine( bool mayBreak = false );
    void addBox( int percent, const QString& text = QString(),
                 Style style = Normal,
                 int halign = Qt::AlignLeft | Qt::TextWordWrap ); //NEW WordBread -> TextWordWrap

    int pageNum() const { return page; }

    struct Box
    {
        QRect rect;
        QString text;
        QFont font;
        int align;

        Box() : align( 0 ) { }
        Box( const QRect& r, const QString& t, const QFont& f, int a )
            : rect( r ), text( t ), font( f ), align( a ) { }
        Box( const Box& b )
            : rect( b.rect ), text( b.text ), font( b.font ),
              align( b.align ) { }

        Box& operator=( const Box& b ) {
            rect = b.rect;
            text = b.text;
            font = b.font;
            align = b.align;
            return *this;
        }

        bool operator==( const Box& b ) const {
            return rect == b.rect && text == b.text && font == b.font &&
                   align == b.align;
        }
    };

private:
    void breakPage(bool init = false);
    void drawRule( Rule rule );

    struct Paragraph {
        QRect rect;
        QList<Box> boxes;

        Paragraph() { }
        Paragraph( QPoint p ) : rect( p, QSize(0, 0) ) { }
    };

    QPrinter *pr;
    QPainter p;
    QFont f8;
    QFont f10;
    QFontMetrics *fmetrics;
    Rule nextRule;
    Paragraph cp;
    int page;
    bool firstParagraph;
    QString g;
    QDateTime dateTime;

    int hmargin;
    int vmargin;
    int voffset;
    int hsize;
    int vsize;
};

#endif
