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

#ifndef Q3SYNTAXHIGHLIGHTER_P_H
#define Q3SYNTAXHIGHLIGHTER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_SYNTAXHIGHLIGHTER
#include "q3syntaxhighlighter.h"
#include "private/q3richtext_p.h"

class Q3SyntaxHighlighterPrivate
{
public:
    Q3SyntaxHighlighterPrivate() :
        currentParagraph(-1)
        {}

    int currentParagraph;
};

class Q3SyntaxHighlighterInternal : public Q3TextPreProcessor
{
public:
    Q3SyntaxHighlighterInternal(Q3SyntaxHighlighter *h) : highlighter(h) {}
    void process(Q3TextDocument *doc, Q3TextParagraph *p, int, bool invalidate) {
        if (p->prev() && p->prev()->endState() == -1)
            process(doc, p->prev(), 0, false);

        highlighter->para = p;
        QString text = p->string()->toString();
        int endState = p->prev() ? p->prev()->endState() : -2;
        int oldEndState = p->endState();
        highlighter->d->currentParagraph = p->paragId();
        p->setEndState(highlighter->highlightParagraph(text, endState));
        highlighter->d->currentParagraph = -1;
        highlighter->para = 0;

        p->setFirstPreProcess(false);
        Q3TextParagraph *op = p;
        p = p->next();
        if ((!!oldEndState || !!op->endState()) && oldEndState != op->endState() &&
             invalidate && p && !p->firstPreProcess() && p->endState() != -1) {
            while (p) {
                if (p->endState() == -1)
                    return;
                p->setEndState(-1);
                p = p->next();
            }
        }
    }
    Q3TextFormat *format(int) { return 0; }

private:
    Q3SyntaxHighlighter *highlighter;

    friend class Q3TextEdit;
};

#endif // QT_NO_SYNTAXHIGHLIGHTER

#endif // Q3SYNTAXHIGHLIGHTER_P_H
