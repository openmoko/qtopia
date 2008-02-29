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

#ifndef QMOTIFSTYLE_P_H
#define QMOTIFSTYLE_P_H
#include <QList>
#include <QTime>
#include <QProgressBar>
#include "qmotifstyle.h"
#include "qcommonstyle_p.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

// Private class
class QMotifStylePrivate : public QCommonStylePrivate
{
    Q_DECLARE_PUBLIC(QMotifStyle)
public:
    QMotifStylePrivate();

public:
#ifndef QT_NO_PROGRESSBAR
    QList<QProgressBar *> bars;
    int animationFps;
    int animateTimer;
    QTime startTime;
    int animateStep;
#endif // QT_NO_PROGRESSBAR
};

#endif //QMOTIFSTYLE_P_H
