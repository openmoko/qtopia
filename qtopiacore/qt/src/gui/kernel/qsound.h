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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSOUND_H
#define QSOUND_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SOUND

class QSoundPrivate;

class Q_GUI_EXPORT QSound : public QObject
{
    Q_OBJECT

public:
    static bool isAvailable();
    static void play(const QString& filename);

    explicit QSound(const QString& filename, QObject* parent = 0);
    ~QSound();

    int loops() const;
    int loopsRemaining() const;
    void setLoops(int);
    QString fileName() const;

    bool isFinished() const;

public Q_SLOTS:
    void play();
    void stop();

public:
#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QSound(const QString& filename, QObject* parent, const char* name);
    static inline QT3_SUPPORT bool available() { return isAvailable(); }
#endif
private:
    Q_DECLARE_PRIVATE(QSound)
    friend class QAuServer;
};

#endif // QT_NO_SOUND

QT_END_HEADER

#endif // QSOUND_H
