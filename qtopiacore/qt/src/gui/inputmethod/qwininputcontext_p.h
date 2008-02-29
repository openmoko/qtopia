/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
