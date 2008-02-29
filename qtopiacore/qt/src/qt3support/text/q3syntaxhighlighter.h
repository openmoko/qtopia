/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SYNTAXHIGHLIGHTER_H
#define Q3SYNTAXHIGHLIGHTER_H

#include <QtGui/qfont.h>
#include <QtGui/qcolor.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3TextEdit;
class Q3SyntaxHighlighterInternal;
class Q3SyntaxHighlighterPrivate;
class Q3TextParagraph;

class Q_COMPAT_EXPORT Q3SyntaxHighlighter
{
    friend class Q3SyntaxHighlighterInternal;

public:
    Q3SyntaxHighlighter(Q3TextEdit *textEdit);
    virtual ~Q3SyntaxHighlighter();

    virtual int highlightParagraph(const QString &text, int endStateOfLastPara) = 0;

    void setFormat(int start, int count, const QFont &font, const QColor &color);
    void setFormat(int start, int count, const QColor &color);
    void setFormat(int start, int count, const QFont &font);
    Q3TextEdit *textEdit() const { return edit; }

    void rehighlight();

    int currentParagraph() const;

private:
    Q3TextParagraph *para;
    Q3TextEdit *edit;
    Q3SyntaxHighlighterPrivate *d;
};

QT_END_HEADER

#endif // Q3SYNTAXHIGHLIGHTER_H
