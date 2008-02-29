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

#include <private/qpainter_p.h>
#include <private/qpaintengine_p.h>
#include "qfile.h"
#include "qimage.h"
#include "qlist.h"
#include "qmap.h"
#include "q3paintengine_svg_p.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qregexp.h"
#include "qtextstream.h"

#include <math.h>

static const double deg2rad = 0.017453292519943295769;        // pi/180
static const char piData[] = "version=\"1.0\" standalone=\"no\"";
static const char publicId[] = "-//W3C//DTD SVG 20001102//EN";
static const char systemId[] = "http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd";

static QString qt_svg_compose_path(const QPainterPath &path);

struct QImgElement {
    QDomElement element;
    QImage image;
    Q_DUMMY_COMPARISON_OPERATOR(QImgElement)
};

struct QPixElement {
    QDomElement element;
    QPixmap pixmap;
    Q_DUMMY_COMPARISON_OPERATOR(QPixElement)
};

struct Q3SVGPaintEngineState {
    double textx, texty; // current text position
    int textalign; // text alignment
    Q_DUMMY_COMPARISON_OPERATOR(Q3SVGPaintEngineState)
};

typedef QList<QImgElement> ImageList;
typedef QList<QPixElement> PixmapList;
typedef QList<Q3SVGPaintEngineState> StateList;

enum ElementType {
    InvalidElement = 0,
    AnchorElement,
    CircleElement,
    ClipElement,
    CommentElement,
    DescElement,
    EllipseElement,
    GroupElement,
    ImageElement,
    LineElement,
    PolylineElement,
    PolygonElement,
    PathElement,
    RectElement,
    SvgElement,
    TextElement,
    TitleElement,
    TSpanElement
};

typedef QMap<QString,ElementType> QSvgTypeMap;
static QSvgTypeMap *qSvgTypeMap=0; // element types
static QMap<QString,QString> *qSvgColMap=0; // recognized color keyword names

class Q3SVGPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(Q3SVGPaintEngine)

public:
    Q3SVGPaintEnginePrivate()
        : dirtyTransform(false), dirtyStyle(false), currentClip(0),
          dev(0), wwidth(0), wheight(0) {}
    void appendChild(QDomElement &e, QPicturePrivate::PaintCommand c);
    void applyStyle(QDomElement *e, QPicturePrivate::PaintCommand c) const;
    void applyTransform(QDomElement *e) const;
    double parseLen(const QString &str, bool *ok=0, bool horiz=true) const;
    int lenToInt(const QDomNamedNodeMap &map, const QString &attr, int def = 0) const;
    double lenToDouble(const QDomNamedNodeMap &map, const QString &attr, int def = 0) const;
    bool play(const QDomNode &node, QPainter *p);
    void setTransform(const QString &tr, QPainter *p);
    void restoreAttributes(QPainter *p);
    void saveAttributes(QPainter *p);
    void setStyle(const QString &s, QPainter *p);
    void setStyleProperty(const QString &prop, const QString &val, QPen *pen, QFont *font,
                          int *talign, QPainter *p);
    void drawPath(const QString &data, QPainter *p);
    QColor parseColor(const QString &col);
    void init() {
        QDomImplementation domImpl;
        QDomDocumentType docType = domImpl.createDocumentType("svg", publicId, systemId);
        doc = domImpl.createDocument("http://www.w3.org/2000/svg", "svg", docType);
        doc.insertBefore(doc.createProcessingInstruction("xml", piData), doc.firstChild());
        current = doc.documentElement();
        images.clear();
        pixmaps.clear();

        doc.documentElement().setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    }


    bool dirtyTransform;
    bool dirtyStyle;
    QRect brect; // bounding rectangle
    QDomDocument doc; // document tree
    QDomNode current;

    ImageList images; // old private
    PixmapList pixmaps;
    StateList stack;
    int currentClip;

//     QPoint curPt;
    Q3SVGPaintEngineState *curr;
//     QPainter *pt; // only used by recursive play() et al
    QPen cpen;
    QBrush cbrush;
    QFont cfont;
    QMatrix worldMatrix;
    const QPaintDevice *dev;
    int wwidth;
    int wheight;
};

Q3SVGPaintEngine::Q3SVGPaintEngine()
    : QPaintEngine(*(new Q3SVGPaintEnginePrivate), AllFeatures)
{
    Q_D(Q3SVGPaintEngine);
    d->init();
}

Q3SVGPaintEngine::Q3SVGPaintEngine(Q3SVGPaintEnginePrivate &dptr)
    : QPaintEngine(dptr, AllFeatures)
{
    Q_D(Q3SVGPaintEngine);
    d->init();
}

Q3SVGPaintEngine::~Q3SVGPaintEngine()
{
    delete qSvgTypeMap; qSvgTypeMap = 0;        // static
    delete qSvgColMap; qSvgColMap = 0;
}

bool Q3SVGPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(Q3SVGPaintEngine);
    d->dirtyTransform = d->dirtyStyle = false;
    d->dev = pdev;
    setActive(true);
    return true;
}

bool Q3SVGPaintEngine::end()
{
    Q_D(Q3SVGPaintEngine);
    d->dev = 0;
    setActive(false);
    return true;
}

void Q3SVGPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();
    if (flags & DirtyPen) updatePen(state.pen());
    if ((flags & DirtyBrush) || (flags & DirtyBrushOrigin))
        updateBrush(state.brush(), state.brushOrigin());
    if (flags & DirtyBackground) updateBackground(state.backgroundMode(), state.backgroundBrush());
    if (flags & DirtyFont) updateFont(state.font());
    if (flags & DirtyTransform) updateMatrix(state.matrix());
    if (flags & DirtyClipRegion) updateClipRegion(state.clipRegion(), state.clipOperation());
    if (flags & DirtyClipPath) updateClipPath(state.clipPath(), state.clipOperation());
}

void Q3SVGPaintEngine::updatePen(const QPen &pen)
{
    Q_D(Q3SVGPaintEngine);
    d->cpen = pen;
    d->dirtyStyle = true;
}

void Q3SVGPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    Q_D(Q3SVGPaintEngine);
    d->cbrush = brush;
    d->dirtyStyle = true;
}

void Q3SVGPaintEngine::updateFont(const QFont &font)
{
    Q_D(Q3SVGPaintEngine);
    d->cfont = font;
    d->dirtyStyle = true;
}

void Q3SVGPaintEngine::updateBackground(Qt::BGMode, const QBrush &)
{
    Q_D(Q3SVGPaintEngine);
    d->dirtyStyle = true;
}

void Q3SVGPaintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_D(Q3SVGPaintEngine);
    d->dirtyTransform = true;
    d->worldMatrix = matrix;
//     d->wwidth = ps->ww;
//     d->wheight = ps->wh;
}

void Q3SVGPaintEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
    Q_D(Q3SVGPaintEngine);
    if (op == Qt::NoClip)
        return;

    QDomElement e;
    d->currentClip++;
    e = d->doc.createElement("clipPath");
    e.setAttribute("id", QString("clip%1").arg(d->currentClip));

    QDomElement path_element = d->doc.createElement("path");
    path_element.setAttribute("d", qt_svg_compose_path(path));
    e.appendChild(path_element);

    d->appendChild(e, QPicturePrivate::PdcSetClipPath);
}

void Q3SVGPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    QPainterPath clipPath;
    clipPath.addRegion(clipRegion);
    updateClipPath(clipPath, op);
}

void Q3SVGPaintEngine::updateRenderHints(QPainter::RenderHints)
{
}

void Q3SVGPaintEngine::drawRect(const QRectF &r)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e;
    e = d->doc.createElement("rect");

    e.setAttribute("x", r.x());
    e.setAttribute("y", r.y());
    e.setAttribute("width", r.width());
    e.setAttribute("height", r.height());
    d->appendChild(e, QPicturePrivate::PdcDrawRect);
}

void Q3SVGPaintEngine::drawPoint(const QPointF &p)
{
    QLineF l(p, p);
    drawLines(&l, 1);
}

void Q3SVGPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    for (int i = 0; i < pointCount; ++i) {
        QLineF l(points[i], points[i]);
        drawLines(&l, 1);
    }
}

void Q3SVGPaintEngine::drawEllipse(const QRect &r)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e;

    if (r.width() == r.height()) {
        e = d->doc.createElement("circle");
        double cx = r.x() + (r.width() / 2.0);
        double cy = r.y() + (r.height() / 2.0);
        e.setAttribute("cx", cx);
        e.setAttribute("cy", cy);
        e.setAttribute("r", cx - r.x());
    } else {
        e = d->doc.createElement("ellipse");
        double cx = r.x() + (r.width() / 2.0);
        double cy = r.y() + (r.height() / 2.0);
        e.setAttribute("cx", cx);
        e.setAttribute("cy", cy);
        e.setAttribute("rx", cx - r.x());
        e.setAttribute("ry", cy - r.y());
    }
    d->appendChild(e, QPicturePrivate::PdcDrawEllipse);
}

void Q3SVGPaintEngine::drawLine(const QLineF &line)
{
    drawLines(&line, 1);
}

void Q3SVGPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e;

    for (int i = 0; i < lineCount; ++i) {
        e = d->doc.createElement("line");
        e.setAttribute("x1", lines[i].x1());
        e.setAttribute("y1", lines[i].y1());
        e.setAttribute("x2", lines[i].x2());
        e.setAttribute("y2", lines[i].y2());
        d->appendChild(e, QPicturePrivate::PdcDrawLineSegments);
    }
}

void Q3SVGPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e = d->doc.createElement("path");
    e.setAttribute("d", qt_svg_compose_path(path));
    d->appendChild(e, QPicturePrivate::PdcDrawPath);
}

void Q3SVGPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(Q3SVGPaintEngine);
    QString str;
    if (mode == PolylineMode) {
        QDomElement e = d->doc.createElement("polyline");
        for (int i = 0; i < pointCount; ++i) {
            QString tmp;
            tmp.sprintf("%f %f ", points[i].x(), points[i].y());
            str += tmp;
        }
        e.setAttribute("points", str.trimmed());
        d->appendChild(e, QPicturePrivate::PdcDrawPolyline);
    } else {
        QDomElement e = d->doc.createElement("polygon");
        for (int i = 0; i < pointCount; ++i) {
            QString tmp;
            tmp.sprintf("%f %f ", points[i].x(), points[i].y());
            str += tmp;
        }
        e.setAttribute("points", str.trimmed());
        d->appendChild(e, QPicturePrivate::PdcDrawPolygon);
    }
}

void Q3SVGPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    QPolygonF poly;
    for (int i = 0; i < pointCount; ++i)
        poly << points[i];
    drawPolygon(poly.constData(), pointCount, mode);
}

void Q3SVGPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF & /* sr */)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e = d->doc.createElement("image");
    e.setAttribute("x", r.x());
    e.setAttribute("y", r.y());
    e.setAttribute("width", r.width());
    e.setAttribute("height", r.height());

    QPixElement pe;
    pe.element = e;
    pe.pixmap = pm;
    d->pixmaps.append(pe);

    // saving to disk and setting the xlink:href attribute will be
    // done later in save() once we now the svg document name.
    d->appendChild(e, QPicturePrivate::PdcDrawPixmap);
}

void Q3SVGPaintEngine::drawTiledPixmap(const QRectF & /* r */, const QPixmap & /* pixmap */,
                                      const QPointF & /* s */)
{
}

void Q3SVGPaintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e = d->doc.createElement("text");
//     int x, y;
//     const QRect r(p.x(), p.y(), ti.width, ti.ascent + ti.descent);
    // horizontal text alignment
 //    if ((ti.flags & Qt::AlignHCenter) != 0) {
//         x = r.x() + r.width() / 2;
//         e.setAttribute("text-anchor", "middle");
//     } else if ((textflags & Qt::AlignRight) != 0) {
//         x = r.right();
//         e.setAttribute("text-anchor", "end");
//     } else {
//         x = r.x();
//     }
//     // vertical text alignment
//     if ((textflags & Qt::AlignVCenter) != 0)
//         y = r.y() + (r.height() + ti.ascent) / 2;
//     else if ((textflags & Qt::AlignBottom) != 0)
//         y = r.bottom();
//     else
//         y = r.y() + ti.ascent;
//     if (x)
//         e.setAttribute("x", x);
//     if (y)
//         e.setAttribute("y", y);
    e.setAttribute("x", p.x());
    e.setAttribute("y", p.y());
    e.appendChild(d->doc.createTextNode(ti.text()));
}

void Q3SVGPaintEngine::drawImage(const QRectF &r, const QImage &im,
                                 const QRectF &, Qt::ImageConversionFlags)
{
    Q_D(Q3SVGPaintEngine);
    QDomElement e = d->doc.createElement("image");
    e.setAttribute("x", r.x());
    e.setAttribute("y", r.y());
    e.setAttribute("width", r.width());
    e.setAttribute("height", r.height());
    QImgElement ie;
    ie.element = e;
    ie.image = im;
    d->images.append(ie);
    // saving to disk and setting the xlink:href attribute will be
    // done later in save() once we now the svg document name.
    d->appendChild(e, QPicturePrivate::PdcDrawImage);
}


/*!
    Returns the SVG as a single string of XML.
*/

QString Q3SVGPaintEngine::toString() const
{
    Q_D(const Q3SVGPaintEngine);
    if (d->doc.isNull())
        return QString();

    return d->doc.toString();
}

/*!
    Saves the SVG to \a fileName.
*/

bool Q3SVGPaintEngine::save(const QString &fileName)
{
    Q_D(Q3SVGPaintEngine);
    // guess svg id from fileName
    QString svgName = fileName.endsWith(".svg") ?
                      fileName.left(fileName.length()-4) : fileName;

    // now we have the info about name and dimensions available
    QDomElement root = d->doc.documentElement();
    root.setAttribute("id", svgName);
    // the standard doesn't take respect x and y. But we want a
    // proper bounding rect. We make width and height bigger when
    // writing out and subtract x and y when reading in.
    root.setAttribute("x", d->brect.x());
    root.setAttribute("y", d->brect.y());
    root.setAttribute("width", d->brect.width() + d->brect.x());
    root.setAttribute("height", d->brect.height() + d->brect.y());

    // ... and know how to name any image files to be written out
    int icount = 0;
    ImageList::Iterator iit = d->images.begin();
    for (; iit != d->images.end(); ++iit) {
        QString href = QString("%1_%2.png").arg(svgName).arg(icount);
        (*iit).image.save(href, "PNG");
        (*iit).element.setAttribute("xlink:href", href);
        icount++;
    }
    PixmapList::Iterator pit = d->pixmaps.begin();
    for (; pit != d->pixmaps.end(); ++pit) {
        QString href = QString("%1_%2.png").arg(svgName).arg(icount);
        (*pit).pixmap.save(href, "PNG");
        (*pit).element.setAttribute("xlink:href", href);
        icount++;
    }

    QFile f(fileName);
    if (!f.open (QIODevice::WriteOnly))
        return false;
    QTextStream s(&f);
    s.setEncoding(QTextStream::UnicodeUTF8);
    s << d->doc;

    return true;
}

/*!
    \overload

    \a dev is the device to use for saving.
*/

bool Q3SVGPaintEngine::save(QIODevice *dev)
{
    Q_D(Q3SVGPaintEngine);
#if defined(CHECK_RANGE)
    if (!d->images.isEmpty() || !d->pixmaps.isEmpty())
        qWarning("Q3SVGPaintEngine::save: skipping external images");
#endif

    QTextStream s(dev);
    s.setEncoding(QTextStream::UnicodeUTF8);
    s << d->doc;

    return true;
}

/*!
    Sets the bounding rectangle of the SVG to rectangle \a r.
*/

void Q3SVGPaintEngine::setBoundingRect(const QRect &r)
{
    d_func()->brect = r;
}

/*!
    Returns the SVG's bounding rectangle.
*/

QRect Q3SVGPaintEngine::boundingRect() const
{
    return d_func()->brect;
}

/*!
    Loads and parses a SVG from \a dev into the device. Returns true
    on success (i.e. loaded and parsed without error); otherwise
    returns false.
*/

bool Q3SVGPaintEngine::load(QIODevice *dev)
{
    return d_func()->doc.setContent(dev);
}

void Q3SVGPaintEnginePrivate::appendChild(QDomElement &e, QPicturePrivate::PaintCommand c)
{
    if (!e.isNull()) {
        current.appendChild(e);
        if (c == QPicturePrivate::PdcSave)
            current = e;
        // ### optimize application of attributes utilizing <g>
        if (c == QPicturePrivate::PdcSetClipRegion || c == QPicturePrivate::PdcSetClipPath) {
            QDomElement ne;
            ne = doc.createElement("g");
            ne.setAttribute("style", QString("clip-path:url(#clip%1)").arg(currentClip));
            if (dirtyTransform) {
                applyTransform(&ne);
                dirtyTransform = false;
            }
            current.appendChild(ne);
            current = ne;
        } else {
            if (dirtyStyle)                // only reset when entering
                applyStyle(&e, c);        // or leaving a <g> tag
            if (dirtyTransform && e.tagName() != "g") {
                // same as above but not for <g> tags
                applyTransform(&e);
                if (c == QPicturePrivate::PdcSave)
                    dirtyTransform = false;
            }
        }
    }
}

void Q3SVGPaintEnginePrivate::applyStyle(QDomElement *e, QPicturePrivate::PaintCommand c) const
{
    // ### do not write every attribute each time
    QColor pcol = cpen.color();
    QColor bcol = cbrush.color();
    QString s;
    if (c == QPicturePrivate::PdcDrawText2 || c == QPicturePrivate::PdcDrawText2Formatted) {
        // QPainter has a reversed understanding of pen/stroke vs.
        // brush/fill for text
        s += QString("fill:rgb(%1,%2,%3);").arg(pcol.red()).arg(pcol.green()).arg(pcol.blue());
        s += QString("stroke-width:0;");
        QFont f = cfont;
        QFontInfo fi(f);
        s += QString("font-size:%1;").arg(fi.pointSize());
        s += QString("font-style:%1;").arg(f.italic() ? "italic" : "normal");
        // not a very scientific distribution
        QString fw;
        if (f.weight() <= QFont::Light)
            fw = "100";
        else if (f.weight() <= QFont::Normal)
            fw = "400";
        else if (f.weight() <= QFont::DemiBold)
            fw = "600";
        else if (f.weight() <= QFont::Bold)
            fw = "700";
        else if (f.weight() <= QFont::Black)
            fw = "800";
        else
            fw = "900";
        s += QString("font-weight:%1;").arg(fw);
        s += QString("font-family:%1;").arg(f.family());
    } else {
        s += QString("stroke:rgb(%1,%2,%3);").arg(pcol.red()).arg(pcol.green()).arg(pcol.blue());
        if (pcol.alpha() != 255)
            s += QString("stroke-opacity:%1;").arg(pcol.alpha()/255.0);
        if (bcol.alpha() != 255)
            s += QString("fill-opacity:%1;").arg(bcol.alpha()/255.0);
        double pw = cpen.width();
        if (pw == 0 && cpen.style() != Qt::NoPen)
            pw = 0.9;
        if (c == QPicturePrivate::PdcDrawLine)
            pw /= (qAbs(worldMatrix.m11()) + qAbs(worldMatrix.m22())) / 2.0;
        s += QString("stroke-width:%1;").arg(pw);
        if (cpen.style() == Qt::DashLine)
            s+= QString("stroke-dasharray:18,6;");
        else if (cpen.style() == Qt::DotLine)
            s+= QString("stroke-dasharray:3;");
        else if (cpen.style() == Qt::DashDotLine)
            s+= QString("stroke-dasharray:9,6,3,6;");
        else if (cpen.style() == Qt::DashDotDotLine)
            s+= QString("stroke-dasharray:9,3,3;");
        if (cbrush.style() == Qt::NoBrush || c == QPicturePrivate::PdcDrawPolyline || c == QPicturePrivate::PdcDrawCubicBezier)
            s += "fill:none;"; // Qt polylines use no brush, neither do Beziers
        else
            s += QString("fill:rgb(%1,%2,%3);").arg(bcol.red()).arg(bcol.green()).arg(bcol.blue());
    }
    e->setAttribute("style", s);
}

void Q3SVGPaintEnginePrivate::applyTransform(QDomElement *e) const
{
    QMatrix m = worldMatrix;

    QString s;
    bool rot = (m.m11() != 1.0 || m.m12() != 0.0 ||
                 m.m21() != 0.0 || m.m22() != 1.0);
    if (!rot && (m.dx() != 0.0 || m.dy() != 0.0)) {
        s = QString("translate(%1,%2)").arg(m.dx()).arg(m.dy());
    } else if (rot) {
        if (m.m12() == 0.0 && m.m21() == 0.0 &&
             m.dx() == 0.0 && m.dy() == 0.0)
            s = QString("scale(%1,%2)").arg(m.m11()).arg(m.m22());
        else
            s = QString("matrix(%1,%2,%3,%4,%5,%6)")
                .arg(m.m11()).arg(m.m12())
                .arg(m.m21()).arg(m.m22())
                .arg(m.dx()).arg(m.dy());
    } else {
        return;
    }
    e->setAttribute("transform", s);
}

bool Q3SVGPaintEngine::play(QPainter *pt)
{
    Q_D(Q3SVGPaintEngine);
    if (!pt) {
        Q_ASSERT(pt);
        return false;
    }
    if (d->dev == 0)
        d->dev = pt->device();
    d->wwidth = pt->window().width();
    d->wheight = pt->window().height();

    pt->setPen(Qt::NoPen); // SVG default pen and brush
    pt->setBrush(Qt::black);
    if (d->doc.isNull()) {
        qWarning("Q3SVGPaintEngine::play: No SVG data set.");
        return false;
    }

    QDomNode svg = d->doc.namedItem("svg");
    if (svg.isNull() || !svg.isElement()) {
        qWarning("Q3SVGPaintEngine::play: Couldn't find any svg element.");
        return false;
    }

    // force transform to be activated in case our sequences
    // are replayed later with a transformed painter
    pt->setWorldXForm(true);

    QDomNamedNodeMap attr = svg.attributes();
    int x = d->lenToInt(attr, "x");
    int y = d->lenToInt(attr, "y");
    d->brect.setX(x);
    d->brect.setY(y);
    QString wstr = attr.contains("width")
                   ? attr.namedItem("width").nodeValue() : QString("100%");
    QString hstr = attr.contains("height")
                   ? attr.namedItem("height").nodeValue() : QString("100%");
    double width = d->parseLen(wstr, 0, true);
    double height = d->parseLen(hstr, 0, false);
    // SVG doesn't respect x and y. But we want a proper bounding rect.
    d->brect.setWidth(int(width) - x);
    d->brect.setHeight(int(height) - y);
    pt->setClipRect(d->brect);

    if (attr.contains("viewBox")) {
        QRegExp re(QString::fromLatin1("\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*,?"
                                       "\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*"));
        if (re.indexIn(attr.namedItem("viewBox").nodeValue()) < 0) {
            qWarning("Q3SVGPaintEngine::play: Invalid viewBox attribute.");
            return false;
        } else {
            double x = re.cap(1).toDouble();
            double y = re.cap(2).toDouble();
            double w = re.cap(3).toDouble();
            double h = re.cap(4).toDouble();
            if (w < 0 || h < 0) {
                qWarning("Q3SVGPaintEngine::play: Invalid viewBox dimension.");
                return false;
            } else if (w == 0 || h == 0) {
                return true;
            }
            pt->scale(width/w, height/h);
            pt->translate(-x, -y);
        }
    }

    const struct ElementTable {
        const char *name;
        ElementType type;
    } etab[] = {
        {"a",        AnchorElement  },
        {"#comment", CommentElement },
        {"circle",   CircleElement  },
        {"clipPath", ClipElement    },
        {"desc",     DescElement    },
        {"ellipse",  EllipseElement },
        {"g",        GroupElement   },
        {"image",    ImageElement   },
        {"line",     LineElement    },
        {"polyline", PolylineElement},
        {"polygon",  PolygonElement },
        {"path",     PathElement    },
        {"rect",     RectElement    },
        {"svg",      SvgElement     },
        {"text",     TextElement    },
        {"tspan",    TSpanElement   },
        {"title",    TitleElement   },
        {0,          InvalidElement }
    };
    // initialize only once
    if (!qSvgTypeMap) {
        qSvgTypeMap = new QSvgTypeMap;
        const ElementTable *t = etab;
        while (t->name) {
            qSvgTypeMap->insert(t->name, t->type);
            t++;
        }
    }

    // initial state
    Q3SVGPaintEngineState st;
    st.textx = st.texty = 0;
    st.textalign = Qt::AlignLeft;
    d->stack.append(st);
    d->curr = &d->stack.last();
    // 'play' all elements recursively starting with 'svg' as root
    bool b = d->play(svg, pt);
    d->stack.removeFirst();
    return b;
}

bool Q3SVGPaintEnginePrivate::play(const QDomNode &node, QPainter *pt)
{
    saveAttributes(pt);

    ElementType t = (*qSvgTypeMap)[node.nodeName()];

    if (t == LineElement && pt->pen().style() == Qt::NoPen) {
        QPen p = pt->pen();
        p.setStyle(Qt::SolidLine);
        pt->setPen(p);
    }
    QDomNamedNodeMap attr = node.attributes();
    if (attr.contains("style"))
        setStyle(attr.namedItem("style").nodeValue(), pt);
    // ### might have to exclude more elements from transform
    if (t != SvgElement && attr.contains("transform"))
        setTransform(attr.namedItem("transform").nodeValue(), pt);
    uint i = attr.length();
    if (i > 0) {
        QPen pen = pt->pen();
        QFont font = pt->font();
        while (i--) {
            QDomNode n = attr.item(i);
            QString a = n.nodeName();
            QString val = n.nodeValue().toLower().trimmed();
            setStyleProperty(a, val, &pen, &font, &curr->textalign, pt);
        }
        pt->setPen(pen);
        pt->setFont(font);
    }

    double x1, y1, x2, y2, rx, ry, w, h;
    double cx1, cy1, crx, cry;
    switch (t) {
    case CommentElement:
        // ignore
        break;
    case RectElement:
        rx = ry = 0;
        x1 = lenToDouble(attr, "x");
        y1 = lenToDouble(attr, "y");
        w = lenToDouble(attr, "width");
        h = lenToDouble(attr, "height");
        if (w == 0 || h == 0) // prevent div by zero below
            break;
        x2 = attr.contains("rx"); // tiny abuse of x2 and y2
        y2 = attr.contains("ry");
        if (x2)
            rx = lenToDouble(attr, "rx");
        if (y2)
            ry = lenToDouble(attr, "ry");
        if (x2 && !y2)
            ry = rx;
        else if (!x2 && y2)
            rx = ry;
        rx = 200.0*rx / w;
        ry = 200.0*ry / h;
        pt->drawRoundRect(QRectF(x1, y1, w, h), int(rx), int(ry));
        break;
    case CircleElement:
        cx1 = lenToDouble(attr, "cx") + 0.5;
        cy1 = lenToDouble(attr, "cy") + 0.5;
        crx = lenToDouble(attr, "r");
        pt->drawEllipse(QRectF(cx1-crx, cy1-crx, 2*crx, 2*crx));
        break;
    case EllipseElement:
        cx1 = lenToDouble(attr, "cx") + 0.5;
        cy1 = lenToDouble(attr, "cy") + 0.5;
        crx = lenToDouble(attr, "rx");
        cry = lenToDouble(attr, "ry");
        pt->drawEllipse(QRectF(cx1-crx, cy1-cry, 2*crx, 2*cry));
        break;
    case LineElement:
        {
            x1 = lenToDouble(attr, "x1");
            x2 = lenToDouble(attr, "x2");
            y1 = lenToDouble(attr, "y1");
            y2 = lenToDouble(attr, "y2");
            QPen p = pt->pen();
            w = p.width();
            p.setWidth((unsigned int)(w * (qAbs(pt->worldMatrix().m11()) + qAbs(pt->worldMatrix().m22())) / 2));
            pt->setPen(p);
            pt->drawLine(QLineF(x1, y1, x2, y2));
            p.setWidthF(w);
            pt->setPen(p);
        }
        break;
    case PolylineElement:
    case PolygonElement:
        {
            QString pts = attr.namedItem("points").nodeValue();
            pts = pts.simplified();
            QStringList sl = pts.split(QRegExp(QString::fromLatin1("[,\\s]")),
                                               QString::SkipEmptyParts);
            QPolygonF ptarr((uint) sl.count() / 2);
            for (int i = 0; i < (int) sl.count() / 2; i++) {
                double dx = sl[2*i].toDouble();
                double dy = sl[2*i+1].toDouble();
                ptarr[i] = QPointF(dx, dy);
            }
            if (t == PolylineElement) {
                if (pt->brush().style() != Qt::NoBrush) {
                    QPen pn = pt->pen();
                    pt->setPen(Qt::NoPen);
                    pt->drawPolygon(ptarr);
                    pt->setPen(pn);
                }
                pt->drawPolyline(ptarr); // ### closes when filled. bug ?
            } else {
                pt->drawPolygon(ptarr);
            }
        }
        break;
    case SvgElement:
    case GroupElement:
    case AnchorElement:
        {
            QDomNode child = node.firstChild();
            while (!child.isNull()) {
                play(child, pt);
                child = child.nextSibling();
            }
        }
        break;
    case PathElement:
        drawPath(attr.namedItem("d").nodeValue(), pt);
        break;
    case TSpanElement:
    case TextElement:
        {
            if (attr.contains("x"))
                 curr->textx = lenToDouble(attr, "x");
            if (attr.contains("y"))
                 curr->texty = lenToDouble(attr, "y");
            if (t == TSpanElement) {
                curr->textx += lenToDouble(attr, "dx");
                curr->texty += lenToDouble(attr, "dy");
            }
            // backup old colors
            QPen pn = pt->pen();
            QColor pcolor = pn.color();
            QColor bcolor = pt->brush().color();
            QDomNode c = node.firstChild();
            while (!c.isNull()) {
                if (c.isText()) {
                    // we have pen and brush reversed for text drawing
                    pn.setColor(bcolor);
                    pt->setPen(pn);
                    QString text = c.toText().nodeValue();
                    text = text.simplified(); // ### 'preserve'
                    w = pt->fontMetrics().width(text);
                    if (curr->textalign == Qt::AlignHCenter)
                        curr->textx -= w / 2;
                    else if (curr->textalign == Qt::AlignRight)
                        curr->textx -= w;
                    pt->drawText(QPointF(curr->textx, curr->texty), text);
                    // restore pen
                    pn.setColor(pcolor);
                    pt->setPen(pn);
                    curr->textx += w;
                } else if (c.isElement() && c.toElement().tagName() == "tspan") {
                    play(c, pt);

                }
                c = c.nextSibling();
            }
            if (t == TSpanElement) {
                // move current text position in parent text element
                StateList::Iterator it = --(--stack.end());
                (*it).textx = curr->textx;
                (*it).texty = curr->texty;
            }
        }
        break;
    case ImageElement:
        {
            x1 = lenToDouble(attr, "x");
            y1 = lenToDouble(attr, "y");
            w = lenToDouble(attr, "width");
            h = lenToDouble(attr, "height");
            QString href = attr.namedItem("xlink:href").nodeValue();
            // ### catch references to embedded .svg files
            QPixmap pix;
            if (!pix.load(href)){
                qWarning("Q3SVGPaintEngine::play: Couldn't load image %s",href.latin1());
                break;
            }
            pt->drawPixmap(QRectF(x1, y1, w, h), pix, QRectF());
        }
        break;
    case DescElement:
    case TitleElement:
        // ignored for now
        break;
    case ClipElement:
        {
            restoreAttributes(pt); // To ensure the clip rect is saved, we need to restore now
            QDomNode child = node.firstChild();
            QDomNamedNodeMap childAttr = child.attributes();
            if (child.nodeName() == "rect") {
                QRect r;
                r.setX(lenToInt(childAttr, "x"));
                r.setY(lenToInt(childAttr, "y"));
                r.setWidth(lenToInt(childAttr, "width"));
                r.setHeight(lenToInt(childAttr, "height"));
                pt->setClipRect(r);
            } else if (child.nodeName() == "ellipse") {
                QRect r;
                int x = lenToInt(childAttr, "cx");
                int y = lenToInt(childAttr, "cy");
                int width = lenToInt(childAttr, "rx");
                int height = lenToInt(childAttr, "ry");
                r.setX(x - width);
                r.setY(y - height);
                r.setWidth(width * 2);
                r.setHeight(height * 2);
                QRegion rgn(r, QRegion::Ellipse);
                pt->setClipRegion(rgn);
            }
            break;
        }
    case InvalidElement:
        qWarning("Q3SVGPaintEngine::play: unknown element type %s", node.nodeName().latin1());
        break;
    }

    if (t != ClipElement)
        restoreAttributes(pt);

    return true;
}

/*!
    \internal

    Parse a <length> datatype consisting of a number followed by an
    optional unit specifier. Can be used for type <coordinate> as
    well. For relative units the value of \a horiz will determine
    whether the horizontal or vertical dimension will be used.
*/
double Q3SVGPaintEnginePrivate::parseLen(const QString &str, bool *ok, bool horiz) const
{
    QRegExp reg(QString::fromLatin1("([+-]?\\d*\\.*\\d*[Ee]?[+-]?\\d*)(em|ex|px|%|pt|pc|cm|mm|in|)$"));
    if (reg.indexIn(str) == -1) {
        qWarning("Q3SVGPaintEngine::parseLen: couldn't parse %s", str.latin1());
        if (ok)
            *ok = false;
        return 0.0;
    }

    double dbl = reg.cap(1).toDouble();
    QString u = reg.cap(2);
    if (!u.isEmpty() && u != "px") {
        if (u == "em") {
            QFontInfo fi(cfont);
            dbl *= fi.pixelSize();
        } else if (u == "ex") {
            QFontInfo fi(cfont);
            dbl *= 0.5 * fi.pixelSize();
        } else if (u == "%")
            dbl *= (horiz ? wwidth : wheight)/100.0;
        else if (u == "cm")
            dbl *= dev->logicalDpiX() / 2.54;
        else if (u == "mm")
            dbl *= dev->logicalDpiX() / 25.4;
        else if (u == "in")
            dbl *= dev->logicalDpiX();
        else if (u == "pt")
            dbl *= dev->logicalDpiX() / 72.0;
        else if (u == "pc")
            dbl *= dev->logicalDpiX() / 6.0;
        else
            qWarning("Q3SVGPaintEngine::parseLen: Unknown unit %s", u.latin1());
    }
    if (ok)
        *ok = true;
    return dbl;
}

/*!
    \internal

    Returns the length specified in attribute \a attr in \a map. If
    the specified attribute doesn't exist or can't be parsed \a def is
    returned.
*/

int Q3SVGPaintEnginePrivate::lenToInt(const QDomNamedNodeMap &map, const QString &attr, int def) const
{
    if (map.contains(attr)) {
        bool ok;
        double dbl = parseLen(map.namedItem(attr).nodeValue(), &ok);
        if (ok)
            return qRound(dbl);
    }
    return def;
}

double Q3SVGPaintEnginePrivate::lenToDouble(const QDomNamedNodeMap &map, const QString &attr,
                                    int def) const
{
    if (map.contains(attr)) {
        bool ok;
        double x = parseLen(map.namedItem(attr).nodeValue(), &ok);
        if (ok) return x;
    }
    return static_cast<double>(def);
}

void Q3SVGPaintEnginePrivate::setTransform(const QString &tr, QPainter *pt)
{
    QString t = tr.simplified();

    QRegExp reg(QString::fromLatin1("\\s*([\\w]+)\\s*\\(([^\\(]*)\\)"));
    int index = 0;
    while ((index = reg.indexIn(t, index)) >= 0) {
        QString command = reg.cap(1);
        QString params = reg.cap(2);
        QStringList plist = params.split(QRegExp(QString::fromLatin1("[,\\s]")),
                                         QString::SkipEmptyParts);
        if (command == "translate") {
            double tx = 0, ty = 0;
            tx = plist[0].toDouble();
            if (plist.count() >= 2)
                ty = plist[1].toDouble();
            pt->translate(tx, ty);
        } else if (command == "rotate") {
            pt->rotate(plist[0].toDouble());
        } else if (command == "scale") {
            double sx, sy;
            sx = sy = plist[0].toDouble();
            if (plist.count() >= 2)
                sy = plist[1].toDouble();
            pt->scale(sx, sy);
        } else if (command == "matrix" && plist.count() >= 6) {
            double m[6];
            for (int i = 0; i < 6; i++)
                m[i] = plist[i].toDouble();
            QMatrix wm(m[0], m[1], m[2], m[3], m[4], m[5]);
            pt->setWorldMatrix(wm, true);
        } else if (command == "skewX") {
            pt->shear(0.0, tan(plist[0].toDouble() * deg2rad));
        } else if (command == "skewY") {
            pt->shear(tan(plist[0].toDouble() * deg2rad), 0.0);
        }

        // move on to next command
        index += reg.matchedLength();
    }
}
/*!
    \internal

    Push the current drawing attributes on a stack.

    \sa restoreAttributes()
*/

void Q3SVGPaintEnginePrivate::saveAttributes(QPainter *pt)
{
    pt->save();
    // copy old state
    Q3SVGPaintEngineState st(*curr);
    stack.append(st);
    curr = &stack.last();
}

/*!
    \internal

    Pop the current drawing attributes off the stack.

    \sa saveAttributes()
*/

void Q3SVGPaintEnginePrivate::restoreAttributes(QPainter *pt)
{
    pt->restore();
    Q_ASSERT(stack.count() > 1);
    stack.removeLast();
    curr = &stack.last();
}

void Q3SVGPaintEnginePrivate::setStyle(const QString &s, QPainter *pt)
{
    QStringList rules = s.split(QChar(';'), QString::SkipEmptyParts);

    QPen pen = pt->pen();
    QFont font = pt->font();

    QStringList::ConstIterator it = rules.constBegin();
    for (; it != rules.constEnd(); it++) {
        int col = (*it).indexOf(':');
        if (col > 0) {
            QString prop = (*it).left(col).simplified();
            QString val = (*it).right((*it).length() - col - 1);
            val = val.toLower().trimmed();
            setStyleProperty(prop, val, &pen, &font, &curr->textalign, pt);
        }
    }
    pt->setPen(pen);
    pt->setFont(font);
}

void Q3SVGPaintEnginePrivate::setStyleProperty(const QString &prop, const QString &val, QPen *pen,
                                       QFont *font, int *talign, QPainter *pt)
{
    if (prop == "stroke") {
        if (val == "none") {
            pen->setStyle(Qt::NoPen);
        } else {
            pen->setColor(parseColor(val));
            if (pen->style() == Qt::NoPen)
                pen->setStyle(Qt::SolidLine);
            if (pen->width() == 0)
                pen->setWidth(1);
        }
    } else if (prop == "stroke-opacity") {
        double opacity = parseLen(val);
        QColor c = pen->color();
        c.setAlpha((int)(opacity*255));
        pen->setColor(c);
    } else if (prop == "fill-opacity") {
        double opacity = parseLen(val);
        QColor c = pt->brush().color();
        c.setAlpha((int)(opacity*255));
        pt->setBrush(c);
    } else if (prop == "stroke-width") {
        double w = parseLen(val);
        if (w > 0.0001)
            pen->setWidth(int(w));
        else
            pen->setStyle(Qt::NoPen);
    } else if (prop == "stroke-linecap") {
        if (val == "butt")
            pen->setCapStyle(Qt::FlatCap);
        else if (val == "round")
            pen->setCapStyle(Qt::RoundCap);
        else if (val == "square")
            pen->setCapStyle(Qt::SquareCap);
    } else if (prop == "stroke-linejoin") {
        if (val == "miter")
            pen->setJoinStyle(Qt::MiterJoin);
        else if (val == "round")
            pen->setJoinStyle(Qt::RoundJoin);
        else if (val == "bevel")
            pen->setJoinStyle(Qt::BevelJoin);
    } else if (prop == "stroke-dasharray") {
        if (val == "18,6")
            pen->setStyle(Qt::DashLine);
        else if (val == "3")
            pen->setStyle(Qt::DotLine);
        else if (val == "9,6,3,6")
            pen->setStyle(Qt::DashDotLine);
        else if (val == "9,3,3")
            pen->setStyle(Qt::DashDotDotLine);
    } else if (prop == "fill") {
        if (val == "none")
            pt->setBrush(Qt::NoBrush);
        else
            pt->setBrush(parseColor(val));
    } else if (prop == "font-size") {
        font->setPixelSize(qRound(parseLen(val)));
    } else if (prop == "font-family") {
        font->setFamily(val);
    } else if (prop == "font-style") {
        if (val == "normal")
            font->setItalic(false);
        else if (val == "italic")
            font->setItalic(true);
        else
            qWarning("QSvgDevice::setStyleProperty: unhandled font-style: %s", val.latin1());
    } else if (prop == "font-weight") {
        int w = font->weight();
        // no exact equivalents so we have to "round" a little bit
        if (val == "100" || val == "200")
            w = QFont::Light;
        if (val == "300" || val == "400" || val == "normal")
            w = QFont::Normal;
        else if (val == "500" || val == "600")
            w = QFont::DemiBold;
        else if (val == "700" || val == "bold" || val == "800")
            w = QFont::Bold;
        else if (val == "900")
            w = QFont::Black;
        font->setWeight(w);
    } else if (prop == "text-anchor") {
        if (val == "middle")
            *talign = Qt::AlignHCenter;
        else if (val == "end")
            *talign = Qt::AlignRight;
        else
            *talign = Qt::AlignLeft;
    }
}

void Q3SVGPaintEnginePrivate::drawPath(const QString &data, QPainter *pt)
{
    double x0 = 0, y0 = 0;              // starting point
    double x = 0, y = 0;                // current point
    QPointF ctrlPt;
    QPainterPath path;                  // resulting path
    int idx = 0;                        // current data position
    int mode = 0, lastMode = 0;         // parser state
    bool relative = false;              // e.g. 'h' vs. 'H'
    QString commands("MZLHVCSQTA");     // recognized commands
    int cmdArgs[] = { 2, 0, 2, 1, 1, 6, 4, 4, 2, 7 };                   // no of arguments
    QRegExp reg(QString::fromLatin1("\\s*,?\\s*([+-]?\\d*\\.?\\d*)"));  // floating point

    // detect next command
    while (idx < data.length()) {
        QChar ch = data[(int)idx++];
        if (ch.isSpace())
            continue;
        QChar chUp = ch.toUpper();
        int cmd = commands.indexOf(chUp);
        if (cmd >= 0) {
            // switch to new command mode
            mode = cmd;
            relative = (ch != chUp);                // e.g. 'm' instead of 'M'
        } else {
            if (mode && !ch.isLetter()) {
                cmd = mode;                        // continue in previous mode
                idx--;
            } else {
                qWarning("Q3SVGPaintEngine::drawPath: Unknown command");
                return;
            }
        }

        // read in the required number of arguments
        const int maxArgs = 7;
        double arg[maxArgs];
        int numArgs = cmdArgs[cmd];
        for (int i = 0; i < numArgs; i++) {
            int pos = reg.indexIn(data, idx);
            if (pos == -1) {
                qWarning("Q3SVGPaintEngine::drawPath: Error parsing arguments");
                return;
            }
            arg[i] = reg.cap(1).toDouble();
            idx = pos + reg.matchedLength();
        };

        // process command
        double offsetX = relative ? x : 0;        // correction offsets
        double offsetY = relative ? y : 0;        // for relative commands
        switch (mode) {
        case 0:                                        // 'M' move to
            x = x0 = arg[0] + offsetX;
            y = y0 = arg[1] + offsetY;
            path.moveTo(x0, y0);
            mode = 2;                                  // -> 'L'
            break;
        case 1:                                        // 'Z' close path
            x = x0;
            y = y0;
            path.closeSubpath();
            mode = 0;
            break;
        case 2:                                        // 'L' line to
            x = arg[0] + offsetX;
            y = arg[1] + offsetY;
            path.lineTo(x, y);
            break;
        case 3:                                        // 'H' horizontal line
            x = arg[0] + offsetX;
            path.lineTo(x, y);
            break;
        case 4:                                        // 'V' vertical line
            y = arg[0] + offsetY;
            path.lineTo(x, y);
            break;
        case 5: {                                        // 'C' cubic bezier curveto
            QPointF c1(arg[0]+offsetX, arg[1]+offsetY);
            QPointF c2(arg[2]+offsetX, arg[3]+offsetY);
            QPointF e(arg[4]+offsetX, arg[5]+offsetY);
            path.cubicTo(c1, c2, e);
            ctrlPt = c2;
            x = e.x();
            y = e.y();
            break;
        }
        case 6: {                                        // 'S' smooth shorthand
            QPointF c1;
            if (lastMode == 5 || lastMode == 6)
                c1 = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
            else
                c1 = QPointF(x, y);
            QPointF c2(arg[0]+offsetX, arg[1]+offsetY);
            QPointF e(arg[2]+offsetX, arg[3]+offsetY);
            path.cubicTo(c1, c2, e);
            ctrlPt = c2;
            x = e.x();
            y = e.y();
            break;
        }
        case 7: {                                        // 'Q' quadratic bezier curves
            QPointF c(arg[0]+offsetX, arg[1]+offsetY);
            QPointF e(arg[2]+offsetX, arg[3]+offsetY);
            path.quadTo(c, e);
            ctrlPt = c;
            x = e.x();
            y = e.y();
            break;
        }
        case 8: {                                      // 'T' smooth shorthand
            QPointF e(arg[0]+offsetX, arg[1]+offsetY);
            QPointF c;
            if (lastMode == 7 || lastMode == 8)
                c = QPointF(2*x-ctrlPt.x(), 2*y-ctrlPt.y());
            else
                c = QPointF(x, y);
            path.quadTo(c, e);
            ctrlPt = c;
            x = e.x();
            y = e.y();
            break;
        }
        case 9:                                        // 'A' elliptical arc curve
            // ### just a straight line
            x = arg[5] + offsetX;
            y = arg[6] + offsetY;
            path.lineTo(x, y);
            break;
        };
        lastMode = mode;
    }
    pt->drawPath(path);
}

/*!
    \internal

    Parses a CSS2-compatible color specification. Either a keyword or
    a numerical RGB specification like #ff00ff or rgb(255,0,50%).
*/

QColor Q3SVGPaintEnginePrivate::parseColor(const QString &col)
{
    static const struct ColorTable {
        const char *name;
        const char *rgb;
    } coltab[] = {
        { "black",   "#000000" },
        { "silver",  "#c0c0c0" },
        { "gray",    "#808080" },
        { "white",   "#ffffff" },
        { "maroon",  "#800000" },
        { "red",     "#ff0000" },
        { "purple",  "#800080" },
        { "fuchsia", "#ff00ff" },
        { "green",   "#008000" },
        { "lime",    "#00ff00" },
        { "olive",   "#808000" },
        { "yellow",  "#ffff00" },
        { "navy",    "#000080" },
        { "blue",    "#0000ff" },
        { "teal",    "#008080" },
        { "aqua",    "#00ffff" },
        // ### the latest spec has more
        { 0,         0         }
    };

    // initialize color map on first use
    if (!qSvgColMap) {
        qSvgColMap = new QMap<QString,QString>;
        const struct ColorTable *t = coltab;
        while (t->name) {
            qSvgColMap->insert(t->name, t->rgb);
            ++t;
        }
    }

    // a keyword?
    if (qSvgColMap->contains(col))
        return QColor((*qSvgColMap)[col]);
    // in rgb(r,g,b) form ?
    QString c = col;
    c.replace(QRegExp(QString::fromLatin1("\\s*")), "");
    QRegExp reg(QString::fromLatin1("^rgb\\((\\d+)(%?),(\\d+)(%?),(\\d+)(%?)\\)$"));
    if (reg.indexIn(c) >= 0) {
        int comp[3];
        for (int i = 0; i < 3; i++) {
            comp[i] = reg.cap(2*i+1).toInt();
            if (!reg.cap(2*i+2).isEmpty())                // percentage ?
                comp[i] = int((double(255*comp[i])/100.0));
        }
        return QColor(comp[0], comp[1], comp[2]);
    }

    // check for predefined Qt color objects, #RRGGBB and #RGB
    return QColor(col);
}

static QString qt_svg_compose_path(const QPainterPath &path)
{
    QString str, tmp;
    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &elm = path.elementAt(i);
        switch (elm.type) {
        case QPainterPath::LineToElement:
            tmp.sprintf("L %f %f ", elm.x, elm.y);
            str += tmp;
            break;
        case QPainterPath::MoveToElement:
            tmp.sprintf("M %f %f ", elm.x, elm.y);
            str += tmp;
            break;
        case QPainterPath::CurveToElement:
        {
            Q_ASSERT(path.elementCount() > i+2);
            const QPainterPath::Element cd1 = path.elementAt(i+1);
            const QPainterPath::Element cd2 = path.elementAt(i+2);
            Q_ASSERT(cd1.type == QPainterPath::CurveToDataElement
                     && cd2.type == QPainterPath::CurveToDataElement);
            tmp.sprintf("C %f %f %f %f %f %f ", elm.x, elm.y, cd1.x, cd1.y, cd2.x, cd2.y);
            str += tmp;
            i += 2;
            break;
        }
        default:
            break;
        }
    }
    return str;
}

