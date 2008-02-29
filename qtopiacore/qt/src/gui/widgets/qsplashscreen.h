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

#ifndef QSPLASHSCREEN_H
#define QSPLASHSCREEN_H

#include <QtGui/qpixmap.h>
#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SPLASHSCREEN
class QSplashScreenPrivate;

class Q_GUI_EXPORT QSplashScreen : public QWidget
{
    Q_OBJECT
public:
    explicit QSplashScreen(const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0);
    QSplashScreen(QWidget *parent, const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0);
    virtual ~QSplashScreen();

    void setPixmap(const QPixmap &pixmap);
    const QPixmap pixmap() const;
    void finish(QWidget *w);
    void repaint();

public Q_SLOTS:
    void showMessage(const QString &message, int alignment = Qt::AlignLeft,
                  const QColor &color = Qt::black);
    void clearMessage();
#ifdef QT3_SUPPORT
    inline QT_MOC_COMPAT void message(const QString &str, int alignment = Qt::AlignLeft,
        const QColor &color = Qt::black) { showMessage(str, alignment, color); }
    inline QT_MOC_COMPAT void clear() { clearMessage(); }
#endif

Q_SIGNALS:
    void messageChanged(const QString &message);

protected:
    bool event(QEvent *e);
    virtual void drawContents(QPainter *painter);
    void mousePressEvent(QMouseEvent *);

private:
    Q_DISABLE_COPY(QSplashScreen)
    Q_DECLARE_PRIVATE(QSplashScreen)
};

#endif // QT_NO_SPLASHSCREEN

QT_END_HEADER

#endif // QSPLASHSCREEN_H
