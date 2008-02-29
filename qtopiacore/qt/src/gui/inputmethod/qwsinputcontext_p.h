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

#ifndef QWSINPUTCONTEXT_P_H
#define QWSINPUTCONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "QtGui/qinputcontext.h"

#ifndef QT_NO_QWS_INPUTMETHODS

class QWSIMEvent;
class QWSIMQueryEvent;
class QWSIMInitEvent;

class QWSInputContext : public QInputContext
{
    Q_OBJECT
public:
    explicit QWSInputContext(QObject* parent = 0);
    ~QWSInputContext() {}


    QString identifierName() { return QString(); }
    QString language() { return QString(); }

    void reset();
    void update();
    void mouseHandler( int x, QMouseEvent *event);

    void setFocusWidget( QWidget *w );
    void widgetDestroyed(QWidget *w);

    bool isComposing() const;

    static QWidget *activeWidget();
    static bool translateIMEvent(QWidget *w, const QWSIMEvent *e);
    static bool translateIMQueryEvent(QWidget *w, const QWSIMQueryEvent *e);
    static bool translateIMInitEvent(const QWSIMInitEvent *e);
};

#endif // QT_NO_QWS_INPUTMETHODS

#endif // QWSINPUTCONTEXT_P_H
