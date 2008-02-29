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

#ifndef QPAINTENGINE_OPENGL_P_H
#define QPAINTENGINE_OPENGL_P_H

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

#include "QtGui/qpaintengine.h"

class QOpenGLPaintEnginePrivate;

class QOpenGLPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QOpenGLPaintEngine)
public:
    QOpenGLPaintEngine();
    ~QOpenGLPaintEngine();

    bool begin(QPaintDevice *pdev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);
    void updateCompositionMode(QPainter::CompositionMode composition_mode);

    void drawRects(const QRectF *r, int rectCount);
    void drawLines(const QLineF *lines, int lineCount);
    void drawPoints(const QPointF *p, int pointCount);

#ifdef Q_NO_USING_KEYWORD
    inline void drawPoints(const QPoint *points, int pointCount) { QPaintEngine::drawPoints(points, pointCount); }
    inline void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
        { QPaintEngine::drawPolygon(points, pointCount, mode); }
    inline void drawLines(const QLine *lines, int lineCount) { QPaintEngine::drawLines(lines, lineCount); }
    inline void drawRects(const QRect *r, int rectCount) { QPaintEngine::drawRects(r, rectCount); }
#else
    using QPaintEngine::drawRects;
    using QPaintEngine::drawPolygon;
    using QPaintEngine::drawPoints;
    using QPaintEngine::drawLines;
#endif

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);

    void drawPath(const QPainterPath &path);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
    void drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                   Qt::ImageConversionFlags conversionFlags);
    void drawTextItem(const QPointF &p, const QTextItem &ti);

#ifdef Q_WS_WIN
    HDC handle() const;
#else
    Qt::HANDLE handle() const;
#endif
    inline Type type() const { return QPaintEngine::OpenGL; }

private:
    void drawPolyInternal(const QPolygonF &pa, bool close = true);
    void drawTextureRect(int tx_width, int tx_height, const QRectF &r, const QRectF &sr, GLenum target);
    Q_DISABLE_COPY(QOpenGLPaintEngine)
};

#endif // QPAINTENGINE_OPENGL_P_H
