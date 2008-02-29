/****************************************************************************
** $Id: qt/src/kernel/qinputcontext_p.h   2.3.12   edited 2005-10-27 $
**
** Declaration of QInputContext class
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QINPUTCONTEXT_P_H
#define QINPUTCONTEXT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_qws.cpp and qwidget_qws.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

#ifndef QT_NO_QWS_IM

class QKeyEvent;
class QWidget;
class QFont;
class QString;


#ifdef Q_WS_QWS
struct QWSIMEvent;
#endif

class QInputContext
{
public:
#ifdef Q_WS_QWS
    static void translateIMEvent( QWSIMEvent *, QWidget * );
    static void reset( QWidget *focusW = 0 );

    static void setMicroFocusWidget(QWidget *);
    static QWidget *microFocusWidget();

    static void notifyWidgetDeletion( QWidget * );
    static void notifyWidgetForcedIMEnd( QWidget * );
    
private:    
    static void retrieveMarkedText( QWidget * );
    static QWidget* activeWidget;
    static QString* composition;
    static bool activeMarkedDeleted;
#endif //Q_WS_QWS
};

#endif //QT_NO_QWS_IM
#endif // QINPUTCONTEXT_P_H
