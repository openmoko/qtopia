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

#ifndef Q3SIMPLERICHTEXT_H
#define Q3SIMPLERICHTEXT_H

#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>
#include <QtGui/qregion.h>
#include <QtGui/qcolor.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_RICHTEXT

class QPainter;
class QWidget;
class Q3StyleSheet;
class QBrush;
class Q3MimeSourceFactory;
class Q3SimpleRichTextData;

class Q_COMPAT_EXPORT Q3SimpleRichText
{
public:
    Q3SimpleRichText(const QString& text, const QFont& fnt,
                     const QString& context = QString(), const Q3StyleSheet* sheet = 0);
    Q3SimpleRichText(const QString& text, const QFont& fnt,
                     const QString& context, const Q3StyleSheet *sheet,
                     const Q3MimeSourceFactory* factory, int pageBreak = -1,
                     const QColor& linkColor = Qt::blue, bool linkUnderline = true);
    ~Q3SimpleRichText();

    void setWidth(int);
    void setWidth(QPainter*, int);
    void setDefaultFont(const QFont &f);
    int width() const;
    int widthUsed() const;
    int height() const;
    void adjustSize();

    void draw(QPainter* p, int x, int y, const QRect& clipRect,
               const QColorGroup& cg, const QBrush* paper = 0) const;

    void draw(QPainter* p, int x, int y, const QRegion& clipRegion,
               const QColorGroup& cg, const QBrush* paper = 0) const {
        draw(p, x, y, clipRegion.boundingRect(), cg, paper);
    }

    QString context() const;
    QString anchorAt(const QPoint& pos) const;

    bool inText(const QPoint& pos) const;

private:
    Q_DISABLE_COPY(Q3SimpleRichText)

    Q3SimpleRichTextData* d;
};

#endif // QT_NO_RICHTEXT

QT_END_HEADER

#endif // Q3SIMPLERICHTEXT_H
