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

#ifndef QWINDOWSSTYLE_P_H
#define QWINDOWSSTYLE_P_H

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

#include "qwindowsstyle.h"
#include "qcommonstyle_p.h"

#ifndef QT_NO_STYLE_WINDOWS
#include <QList>
#include <QTime>
#include <QHash>

class QStringList;
class QTime;
class QProgressBar;

class IconTheme
{

public:
    IconTheme(QHash <int, QString> dirList, QStringList parents) :
          _dirList(dirList), _parents(parents), _valid(true){ }
    IconTheme() : _valid(false){ }

    QHash <int, QString> dirList() {return _dirList;}
    QStringList parents() {return _parents;}
    bool isValid() {return _valid;}

private:
    QHash <int, QString> _dirList;
    QStringList _parents;
    bool _valid;
};

class QWindowsStylePrivate : public QCommonStylePrivate
{
    Q_DECLARE_PUBLIC(QWindowsStyle)
public:
    QWindowsStylePrivate();
    bool hasSeenAlt(const QWidget *widget) const;
    bool altDown() const { return alt_down; }
    bool alt_down;
    QList<const QWidget *> seenAlt;
    int menuBarTimer;

    QList<QProgressBar *> bars;
    int animationFps;
    int animateTimer;
    QTime startTime;
    int animateStep;    
    QColor inactiveCaptionText;
    QColor activeCaptionColor;
    QColor activeGradientCaptionColor;
    QColor inactiveCaptionColor;
    QColor inactiveGradientCaptionColor;
    
    //icon detection on X11
    QPixmap findIcon(int size, const QString &) const;
#ifdef Q_WS_X11
    QPixmap findIconHelper(int size, const QString &, const QString &, QStringList &visited) const;
    IconTheme parseIndexFile(const QString &themeName) const;
    mutable QString themeName;
    QStringList iconDirs;
    mutable QHash <QString, IconTheme> themeList;
#endif
};

#endif // QT_NO_STYLE_WINDOWS
#endif //QWINDOWSSTYLE_P_H
