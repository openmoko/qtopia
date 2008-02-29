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

#ifndef QSCROLLAREA_H
#define QSCROLLAREA_H

#include <QtGui/qabstractscrollarea.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_SCROLLAREA

class QScrollAreaPrivate;

class Q_GUI_EXPORT QScrollArea : public QAbstractScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool widgetResizable READ widgetResizable WRITE setWidgetResizable)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

public:
    explicit QScrollArea(QWidget* parent=0);
    ~QScrollArea();

    QWidget *widget() const;
    void setWidget(QWidget *widget);
    QWidget *takeWidget();

    bool widgetResizable() const;
    void setWidgetResizable(bool resizable);

    QSize sizeHint() const;
    bool focusNextPrevChild(bool next);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment);

    void ensureVisible(int x, int y, int xmargin = 50, int ymargin = 50);
    void ensureWidgetVisible(QWidget *childWidget, int xmargin = 50, int ymargin = 50);

protected:
    QScrollArea(QScrollAreaPrivate &dd, QWidget *parent = 0);
    bool event(QEvent *);
    bool eventFilter(QObject *, QEvent *);
    void resizeEvent(QResizeEvent *);
    void scrollContentsBy(int dx, int dy);

private:
    Q_DECLARE_PRIVATE(QScrollArea)
    Q_DISABLE_COPY(QScrollArea)
};

#endif // QT_NO_SCROLLAREA

QT_END_HEADER

#endif // QSCROLLAREA_H
