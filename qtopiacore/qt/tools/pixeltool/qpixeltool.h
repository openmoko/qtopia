/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef QPIXELTOOL_H
#define QPIXELTOOL_H

#include <qwidget.h>
#include <qpixmap.h>

class QAssistantClient;

class QPixelTool : public QWidget
{
    Q_OBJECT
public:
    QPixelTool(QWidget *parent = 0);
    ~QPixelTool();

    void timerEvent(QTimerEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

    QSize sizeHint() const;

public slots:
    void setZoom(int zoom);
    void setGridSize(int gridSize);
    void toggleGrid();
    void toggleFreeze();
    void setZoomVisible(bool visible);
    void copyToClipboard();
    void saveToFile();
    void increaseGridSize() { setGridSize(m_gridSize + 1); }
    void decreaseGridSize() { setGridSize(m_gridSize - 1); }
    void increaseZoom() { setZoom(m_zoom + 1); }
    void decreaseZoom() { setZoom(m_zoom - 1); }
    void showHelp();

private:
    void grabScreen();
    void startZoomVisibleTimer();
    void startGridSizeVisibleTimer();

    bool m_freeze;
    bool m_displayZoom;
    bool m_displayGridSize;
    bool m_mouseDown;
    bool m_autoUpdate;

    int m_gridActive;
    int m_zoom;
    int m_gridSize;

    int m_updateId;
    int m_displayZoomId;
    int m_displayGridSizeId;

    int m_currentColor;

    QPoint m_lastMousePos;
    QPoint m_dragStart;
    QPoint m_dragCurrent;
    QPixmap m_buffer;

    QSize m_initialSize;

    QAssistantClient *m_assistantClient;
};

#endif // QPIXELTOOL_H
