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

#ifndef QWININPUTCONTEXT_P_H
#define QWININPUTCONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qinputcontext.cpp.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qinputcontext.h"
#include "QtCore/qt_windows.h"

class QWinInputContext : public QInputContext
{
    Q_OBJECT
public:
    explicit QWinInputContext(QObject* parent = 0);
    virtual ~QWinInputContext();

    virtual QString identifierName() { return QLatin1String("win"); }
    virtual QString language();

    virtual void reset();
    virtual void update();

    virtual void mouseHandler(int x, QMouseEvent *event);
    virtual bool isComposing() const;

    virtual void setFocusWidget(QWidget *w);

    bool startComposition();
    bool endComposition();
    bool composition(LPARAM lparam);

    static void enable(QWidget *w, bool e);
    static void TranslateMessage(const MSG *msg);
    static LRESULT DefWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
    void init();
};

#endif // QWININPUTCONTEXT_P_H
