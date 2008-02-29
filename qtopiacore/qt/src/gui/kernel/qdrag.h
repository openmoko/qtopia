/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDRAG_H
#define QDRAG_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_DRAGANDDROP
class QMimeData;
class QDragPrivate;
class QWidget;
class QPixmap;
class QPoint;
class QDragManager;

class Q_GUI_EXPORT QDrag : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDrag)
public:
    explicit QDrag(QWidget *dragSource);
    ~QDrag();

    void setMimeData(QMimeData *data);
    QMimeData *mimeData() const;

    void setPixmap(const QPixmap &);
    QPixmap pixmap() const;

    void setHotSpot(const QPoint &hotspot);
    QPoint hotSpot() const;

    QWidget *source() const;
    QWidget *target() const;

    Qt::DropAction start(Qt::DropActions supportedActions = Qt::CopyAction);
    Qt::DropAction exec(Qt::DropActions supportedActions = Qt::MoveAction);
    Qt::DropAction exec(Qt::DropActions supportedActions, Qt::DropAction defaultAction);

    void setDragCursor(const QPixmap &cursor, Qt::DropAction action);

Q_SIGNALS:
    void actionChanged(Qt::DropAction action);
    void targetChanged(QWidget *newTarget);

private:
#ifdef Q_WS_MAC
    friend class QWidgetPrivate;
#endif
    friend class QDragManager;
    Q_DISABLE_COPY(QDrag)
};

#endif // QT_NO_DRAGANDDROP

QT_END_HEADER

#endif // QDRAG_H
