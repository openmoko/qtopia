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

#include "qstylesheetstyle_p.h"

#ifndef QT_NO_STYLE_STYLESHEET

#include <qdebug.h>
#include <qapplication.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qstyleoption.h>
#include <qlineedit.h>
#include <qwindowsstyle.h>
#include <qcombobox.h>
#include <qwindowsstyle.h>
#include <qplastiquestyle.h>
#include "private/qcssparser_p.h"
#include "private/qmath_p.h"
#include <qabstractscrollarea.h>
#include <qtooltip.h>
#include <qshareddata.h>
#include <qradiobutton.h>

using namespace QCss;

static QHash<const QWidget *, QVector<StyleRule> > *styleRulesCache = 0;
typedef QHash<QString, QHash<int, QRenderRule> > QRenderRules;
static QHash<const QWidget *, QRenderRules> *renderRulesCache = 0;
static QHash<const QWidget *, int> *customPaletteWidgets = 0; // widgets whose palette we tampered
static QHash<const QWidget *, int> *customFontWidgets = 0; // widgets whose font we tampered
static QHash<const QWidget *, StyleSheet> *styleSheetCache = 0; // parsed style sheets

#define ceil(x) ((int)(x) + ((x) > 0 && (x) != (int)(x)))

static void initStatics()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;
    styleRulesCache = new QHash<const QWidget *, QVector<StyleRule> >;
    renderRulesCache = new QHash<const QWidget *, QRenderRules>;
    customPaletteWidgets = new QHash<const QWidget *, int>;
    customFontWidgets = new QHash<const QWidget *, int>;
    styleSheetCache = new QHash<const QWidget *, StyleSheet>;
}

struct QStyleSheetBorderImageData : public QSharedData
{
    QStyleSheetBorderImageData()
    {
        for (int i = 0; i < 4; i++)
            cuts[i] = -1;
    }
    QPixmap topEdge, bottomEdge, leftEdge, rightEdge, middle;
    QRect topEdgeRect, bottomEdgeRect, leftEdgeRect, rightEdgeRect, middleRect;
    QRect topLeftCorner, topRightCorner, bottomRightCorner, bottomLeftCorner;
    int cuts[4];
    QPixmap pixmap;
    QImage image;
    QCss::TileMode horizStretch, vertStretch;

    void cutBorderImage();
};

struct QStyleSheetBackgroundData : public QSharedData
{
    QStyleSheetBackgroundData(const QBrush& b, const QPixmap& p, QCss::Repeat r,
                              Qt::Alignment a, QCss::Origin o)
        : brush(b), pixmap(p), repeat(r), position(a), origin(o) { }

    bool isTransparent() const {
        if (brush.style() != Qt::NoBrush)
            return !brush.isOpaque();
        return pixmap.isNull() ? false : pixmap.hasAlpha();
    }
    QBrush brush;
    QPixmap pixmap;
    QCss::Repeat repeat;
    Qt::Alignment position;
    QCss::Origin origin;
};

struct QStyleSheetBorderData : public QSharedData
{
    QStyleSheetBorderData() : bi(0)
    {
        for (int i = 0; i < 4; i++) {
            borders[i] = 0;
            styles[i] = QCss::BorderStyle_None;
        }
    }

    QStyleSheetBorderData(int *b, QColor *c, QCss::BorderStyle *s, QSize *r) : bi(0)
    {
        for (int i = 0; i < 4; i++) {
            borders[i] = b[i];
            styles[i] = s[i];
            colors[i] = c[i];
            radii[i] = r[i].expandedTo(QSize(0, 0));
        }
    }

    int borders[4];
    QColor colors[4];
    QCss::BorderStyle styles[4];
    QSize radii[4]; // topleft, topright, bottomleft, bottomright

    const QStyleSheetBorderImageData *borderImage() const
    { return bi; }
    bool hasBorderImage() const { return bi!=0; }

    QStyleSheetBorderImageData *bi;
};

struct QStyleSheetBoxData : public QSharedData
{
    QStyleSheetBoxData(int *m, int *p, int s) : spacing(s)
    {
        for (int i = 0; i < 4; i++) {
            margins[i] = m[i];
            paddings[i] = p[i];
        }
    }

    int margins[4];
    int paddings[4];

    int spacing;
};

struct QStyleSheetPaletteData : public QSharedData
{
    QStyleSheetPaletteData(const QColor &fg, const QColor &sfg, const QBrush &sbg,
                           const QBrush &abg)
        : foreground(fg), selectionForeground(sfg), selectionBackground(sbg),
          alternateBackground(abg) { }

    QColor foreground;
    QColor selectionForeground;
    QBrush selectionBackground;
    QBrush alternateBackground;
};

struct QStyleSheetGeometryData : public QSharedData
{
    QStyleSheetGeometryData(int w, int h, int minw, int minh)
        : minWidth(minw), minHeight(minh), width(w), height(h) { }

    int minWidth, minHeight, width, height;
};

struct QStyleSheetPositionData : public QSharedData
{
    QStyleSheetPositionData(int l, int t, int r, int b, Origin o, Qt::Alignment p, QCss::PositionMode m)
        : left(l), top(t), bottom(b), right(r), origin(o), position(p), mode(m) { }

    int left, top, bottom, right;
    Origin origin;
    Qt::Alignment position;
    QCss::PositionMode mode;
};

class QRenderRule
{
public:
    QRenderRule() : pal(0), b(0), bg(0), bd(0), geo(0), p(0) { }
   QRenderRule(const QVector<QCss::Declaration> &, const QWidget *, const QString&, QStyle *);
    ~QRenderRule() { }

    QRect borderRect(const QRect &r) const;
    QRect paddingRect(const QRect &r) const;
    QRect contentsRect(const QRect &r) const;
    QRect boxRect(const QRect &r) const;
    QSize boxSize(const QSize &s) const;
    QRect originRect(const QRect &rect, Origin origin) const;

    bool paintsOver(Edge e1, Edge e2);
    void drawBorder(QPainter *, const QRect&);
    void drawBorderImage(QPainter *, const QRect&);
    void drawBackground(QPainter *, const QRect&);
    void drawBackgroundImage(QPainter *, const QRect&);
    void drawFrame(QPainter *, const QRect&);
    void drawImage(QPainter *p, const QRect &rect);
    void drawRule(QPainter *, const QRect&);
    void configurePalette(QPalette *, QPalette::ColorGroup, const QWidget *, bool);
    void configurePalette(QPalette *p, QPalette::ColorRole fr, QPalette::ColorRole br);

    void getRadii(const QRect &br, QSize *tlr, QSize *trr, QSize *blr, QSize *brr) const;

    const QStyleSheetPaletteData *palette() const { return pal; }
    const QStyleSheetBoxData *box() const { return b; }
    const QStyleSheetBackgroundData *background() const { return bg; }
    const QStyleSheetBorderData *border() const { return bd; }
    const QStyleSheetGeometryData *geometry() const { return geo; }
    const QStyleSheetPositionData *position() const { return p; }

    bool hasPalette() const { return pal != 0; }
    bool hasBackground() const { return bg != 0; }
    bool hasGradientBackground() const { return bg && bg->brush.style() >= Qt::LinearGradientPattern
                                                   && bg->brush.style() <= Qt::ConicalGradientPattern; }
    bool hasBox() const { return b != 0; }
    bool hasBorder() const { return bd != 0; }
    bool hasPosition() const { return p != 0; }
    bool hasGeometry() const { return geo != 0; }
    bool hasDrawable() const { return hasBorder() || hasBackground() || hasImage(); }

    QSize minimumContentsSize() const
    { return geo ? QSize(geo->minWidth, geo->minHeight) : QSize(0, 0); }

    QSize contentsSize() const
    { return geo ? QSize(geo->width, geo->height) : (imageRect.isValid() ? imageRect.size() : QSize()); }
    bool hasContentsSize() const
    { return (geo && (geo->width != -1 || geo->height != -1)) || imageRect.isValid(); }

    QSize size() const { return boxSize(contentsSize()); }

    bool hasImage() const { return !image.isNull(); }
    QPixmap image;
    QRect imageRect;
    QFont font;

    QHash<QString, int> styleHints;
    bool hasStyleHint(const QString& sh) const { return styleHints.contains(sh); }
    int styleHint(const QString& sh) const { return styleHints.value(sh); }

    void fixupBorder();
#if 0
    void fixupGeometry(const QWidget *w, QString part, QStyle *);
#endif

    QSharedDataPointer<QStyleSheetPaletteData> pal;
    QSharedDataPointer<QStyleSheetBoxData> b;
    QSharedDataPointer<QStyleSheetBackgroundData> bg;
    QSharedDataPointer<QStyleSheetBorderData> bd;
    QSharedDataPointer<QStyleSheetGeometryData> geo;
    QSharedDataPointer<QStyleSheetPositionData> p;
};

///////////////////////////////////////////////////////////////////////////////////////////
static const char *knownStyleHints[] = {
    "activate-on-singleclick",
    "button-layout",
    "combobox-list-mousetracking",
    "combobox-popup",
    "dither-disable-text",
    "etch-disabled-text",
    "gridline-color",
    "lineedit-password-character",
    "menubar-altkey-navigation",
    "menubar-separator",
    "messagebox-text-interaction-flags",
    "mouse-tracking",
    "opacity",
    "show-decoration-selected",
    "spinbox-click-autorepeat-rate",
    "spincontrol-disable-on-bounds"
};

static const char **endKnownStyleHints = knownStyleHints + sizeof(knownStyleHints)/sizeof(char *);

QRenderRule::QRenderRule(const QVector<Declaration> &declarations, const QWidget *,
                         const QString &, QStyle *)
: pal(0), b(0), bg(0), bd(0), geo(0), p(0)
{
    ValueExtractor v(declarations);
    int w = -1, h = -1, minw = -1, minh = -1;
    if (v.extractGeometry(&w, &h, &minw, &minh))
        geo = new QStyleSheetGeometryData(w, h, minw, minh);

    int left = 0, top = 0, right = 0, bottom = 0;
    Origin origin = Origin_Unknown;
    Qt::Alignment position = 0;
    QCss::PositionMode mode = PositionMode_Static;
    if (v.extractPosition(&left, &top, &right, &bottom, &origin, &position, &mode))
        p = new QStyleSheetPositionData(left, top, right, bottom, origin, position, mode);

    int margins[4], paddings[4], spacing = -1;
    for (int i = 0; i < 4; i++)
        margins[i] = paddings[i] = 0;
    if (v.extractBox(margins, paddings, &spacing))
        b = new QStyleSheetBoxData(margins, paddings, spacing);

    int borders[4];
    QColor colors[4];
    QCss::BorderStyle styles[4];
    QSize radii[4];
    for (int i = 0; i < 4; i++) {
        borders[i] = 0;
        styles[i] = BorderStyle_None;
    }
    if (v.extractBorder(borders, colors, styles, radii))
        bd = new QStyleSheetBorderData(borders, colors, styles, radii);

    QBrush brush;
    QString uri;
    Repeat repeat = Repeat_XY;
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;
    origin = Origin_Padding;
    if (v.extractBackground(&brush, &uri, &repeat, &alignment, &origin))
        bg = new QStyleSheetBackgroundData(brush, QPixmap(uri), repeat, alignment, origin);

    QColor sfg, fg;
    QBrush sbg, abg;
    if (v.extractPalette(&fg, &sfg, &sbg, &abg))
        pal = new QStyleSheetPaletteData(fg, sfg, sbg, abg);

    // FIXME: Need to take into account the widget fonts
    int adj = -255;
    v.extractFont(&font, &adj);

    for (int i = 0; i < declarations.count(); i++) {
        const Declaration& decl = declarations.at(i);
        if (decl.propertyId == BorderImage) {
            QString uri;
            QCss::TileMode horizStretch, vertStretch;
            int cuts[4];

            decl.borderImageValue(&uri, cuts, &horizStretch, &vertStretch);
            if (uri.isEmpty() || uri == QLatin1String("none")) {
                if (bd && bd->bi)
                    bd->bi->pixmap = QPixmap();
            } else {
                if (!bd)
                    bd = new QStyleSheetBorderData;
                if (!bd->bi)
                    bd->bi = new QStyleSheetBorderImageData;

                QStyleSheetBorderImageData *bi = bd->bi;
                bi->pixmap = QPixmap(uri);
                for (int i = 0; i < 4; i++)
                    bi->cuts[i] = cuts[i];
                bi->horizStretch = horizStretch;
                bi->vertStretch = vertStretch;
            }
        } else if (decl.propertyId == QtImage) {
            image = QPixmap(decl.uriValue());
            if (!imageRect.isValid())
                imageRect = QRect(0, 0, image.width(), image.height());
        } else if (qBinaryFind(knownStyleHints, endKnownStyleHints, decl.property) != endKnownStyleHints) {
            int hint;
            decl.intValue(&hint);
            styleHints[decl.property] = hint;
        } else if (decl.property.compare(QLatin1String("image-region"), Qt::CaseInsensitive) == 0) {
            imageRect = decl.rectValue();
        } else if (decl.propertyId == UnknownProperty) {
            qWarning("Unknown property %s", qPrintable(decl.property));
        }
    }

    fixupBorder();
#if 0
    fixupGeometry(widget, part, style);
#endif
}

QRect QRenderRule::borderRect(const QRect& r) const
{
    if (!hasBox())
        return r;
    const int* m = box()->margins;
    return r.adjusted(m[LeftEdge], m[TopEdge], -m[RightEdge], -m[BottomEdge]);
}

QRect QRenderRule::paddingRect(const QRect& r) const
{
    QRect br = borderRect(r);
    if (!hasBorder())
        return br;
    const int *b = border()->borders;
    return br.adjusted(b[LeftEdge], b[TopEdge], -b[RightEdge], -b[BottomEdge]);
}

QRect QRenderRule::contentsRect(const QRect& r) const
{
    QRect pr = paddingRect(r);
    if (!hasBox())
        return pr;
    const int *p = box()->paddings;
    return pr.adjusted(p[LeftEdge], p[TopEdge], -p[RightEdge], -p[BottomEdge]);
}

QRect QRenderRule::boxRect(const QRect& cr) const
{
    QRect r = cr;
    if (hasBox()) {
        const int *m = box()->margins;
        const int *p = box()->paddings;
        r.adjust(-p[LeftEdge] - m[LeftEdge], -p[TopEdge] - m[TopEdge],
                 p[RightEdge] + m[RightEdge], p[BottomEdge] + m[BottomEdge]);
    }
    if (hasBorder()) {
        const int *b = border()->borders;
        r.adjust(-b[LeftEdge], -b[TopEdge], b[RightEdge], b[BottomEdge]);
    }
    return r;
}

QSize QRenderRule::boxSize(const QSize &cs) const
{
    QSize bs = boxRect(QRect(QPoint(0, 0), cs)).size();
    if (cs.width() < 0) bs.setWidth(-1);
    if (cs.height() < 0) bs.setHeight(-1);
    return bs;
}

void QRenderRule::fixupBorder()
{
    if (bd == 0)
        return;

    if (!bd->hasBorderImage() || bd->bi->pixmap.isNull()) {
        delete bd->bi;
        bd->bi = 0;
        // ignore the color, border of edges that have none border-style
        QColor color = pal ? pal->foreground : QColor();
        for (int i = 0; i < 4; i++) {
            if (bd->styles[i] != BorderStyle_None) {
                if (!bd->colors[i].isValid()) // auto-acquire 'color'
                    bd->colors[i] = color;
            } else {
                // border-style: none forces width to be 0
                bd->colors[i] = QColor();
                bd->borders[i] = 0;
            }
        }

        return;
    }

    // inspect the border image
    QStyleSheetBorderImageData *bi = bd->bi;
    if (bi->cuts[0] == -1) {
        for (int i = 0; i < 4; i++) // assume, cut = border
            bi->cuts[i] = int(border()->borders[i]);
    }
    bi->cutBorderImage();
}

#if 0
void QRenderRule::fixupGeometry(const QWidget *w, QString part, QStyle *style)
{
    if (part.isEmpty() || hasGeometry())
        return;
    int width = -1, height = -1;
    part = part.toLower();
    if (part.compare(QLatin1String("up-button")) == 0
        || part.compare(QLatin1String("down-button")) == 0)
        width = 16;
    else if (part.compare(QLatin1String("drop-down")) == 0)
        width = 16;
    else if (part.compare(QLatin1String("indicator")) == 0) {
        if (qobject_cast<const QRadioButton *>(w)) {
            width = style->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, 0, w);
            height = style->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight, 0, w);
        } else {
            width = style->pixelMetric(QStyle::PM_IndicatorWidth, 0, w);
            height = style->pixelMetric(QStyle::PM_IndicatorWidth, 0, w);
        }
    } else if (part.compare(QLatin1String("menu-indicator")) == 0) {
        width = height = style->pixelMetric(QStyle::PM_MenuButtonIndicator, 0, w);
    } else if (part.endsWith("-arrow")) {
        width = height = 7;
    }
    geo = new QStyleSheetGeometryData(width, height, width, height);
}
#endif

void QStyleSheetBorderImageData::cutBorderImage()
{
    const int w = pixmap.width();
    const int h = pixmap.height();
    const int &l = cuts[LeftEdge], &r = cuts[RightEdge],
              &t = cuts[TopEdge], &b = cuts[BottomEdge];

    topEdgeRect = QRect(l, 0, w - r - l, t);
    bottomEdgeRect = QRect(l, h - b, w - l - r, b);
    if (horizStretch != TileMode_Stretch) {
        if (topEdgeRect.isValid())
            topEdge = pixmap.copy(topEdgeRect).scaledToHeight(t);
        if (bottomEdgeRect.isValid())
            bottomEdge = pixmap.copy(bottomEdgeRect).scaledToHeight(b);
    }

    leftEdgeRect = QRect(0, t, l, h - b - t);
    rightEdgeRect = QRect(w - r, t, r, h - t- b);
    if (vertStretch != TileMode_Stretch) {
        if (leftEdgeRect.isValid())
            leftEdge = pixmap.copy(leftEdgeRect).scaledToWidth(l);
        if (rightEdgeRect.isValid())
            rightEdge = pixmap.copy(rightEdgeRect).scaledToWidth(r);
    }

    middleRect = QRect(l, t, w - r -l, h - t - b);
    if (middleRect.isValid()
        && !(horizStretch == TileMode_Stretch && vertStretch == TileMode_Stretch)) {
        middle = pixmap.copy(middleRect);
    }
}

///////////////////////////////////////////////////////////////////////////////////
static QPen qPenFromStyle(const QBrush& b, qreal width, BorderStyle s)
{
    Qt::PenStyle ps = Qt::NoPen;

    switch (s) {
    case BorderStyle_Dotted:
        ps  = Qt::DotLine;
        break;
    case BorderStyle_Dashed:
        ps = width == 1 ? Qt::DotLine : Qt::DashLine;
        break;
    case BorderStyle_DotDash:
        ps = Qt::DashDotLine;
        break;
    case BorderStyle_DotDotDash:
        ps = Qt::DashDotDotLine;
        break;
    case BorderStyle_Inset:
    case BorderStyle_Outset:
    case BorderStyle_Solid:
        ps = Qt::SolidLine;
        break;
    default:
        break;
    }

    return QPen(b, width, ps, Qt::FlatCap);
}

static void qDrawRoundedCorners(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2,
                                const QSizeF& r1, const QSizeF& r2,
                                Edge edge, BorderStyle s, QColor c)
{
    const qreal pw = (edge == TopEdge || edge == BottomEdge) ? y2-y1 : x2-x1;
    if (s == BorderStyle_Double) {
        qreal wby3 = pw/3;
        switch (edge) {
        case TopEdge:
        case BottomEdge:
            qDrawRoundedCorners(p, x1, y1, x2, y1+wby3, r1, r2, edge, BorderStyle_Solid, c);
            qDrawRoundedCorners(p, x1, y2-wby3, x2, y2, r1, r2, edge, BorderStyle_Solid, c);
            break;
        case LeftEdge:
            qDrawRoundedCorners(p, x1, y1+1, x1+wby3, y2, r1, r2, LeftEdge, BorderStyle_Solid, c);
            qDrawRoundedCorners(p, x2-wby3, y1+1, x2, y2, r1, r2, LeftEdge, BorderStyle_Solid, c);
            break;
        case RightEdge:
            qDrawRoundedCorners(p, x1, y1+1, x1+wby3, y2, r1, r2, RightEdge, BorderStyle_Solid, c);
            qDrawRoundedCorners(p, x2-wby3, y1+1, x2, y2, r1, r2, RightEdge, BorderStyle_Solid, c);
            break;
        default:
            break;
        }
        return;
    } else if (s == BorderStyle_Ridge || s == BorderStyle_Groove) {
        BorderStyle s1, s2;
        if (s == BorderStyle_Groove) {
            s1 = BorderStyle_Inset;
            s2 = BorderStyle_Outset;
        } else {
            s1 = BorderStyle_Outset;
            s2 = BorderStyle_Inset;
        }
        int pwby2 = qRound(pw/2);
        switch (edge) {
        case TopEdge:
            qDrawRoundedCorners(p, x1, y1, x2, y1 + pwby2, r1, r2, TopEdge, s1, c);
            qDrawRoundedCorners(p, x1, y1 + pwby2, x2, y2, r1, r2, TopEdge, s2, c);
            break;
        case BottomEdge:
            qDrawRoundedCorners(p, x1, y1 + pwby2, x2, y2, r1, r2, BottomEdge, s1, c);
            qDrawRoundedCorners(p, x1, y1, x2, y2-pwby2, r1, r2, BottomEdge, s2, c);
            break;
        case LeftEdge:
            qDrawRoundedCorners(p, x1, y1, x1 + pwby2, y2, r1, r2, LeftEdge, s1, c);
            qDrawRoundedCorners(p, x1 + pwby2, y1, x2, y2, r1, r2, LeftEdge, s2, c);
            break;
        case RightEdge:
            qDrawRoundedCorners(p, x1 + pwby2, y1, x2, y2, r1, r2, RightEdge, s1, c);
            qDrawRoundedCorners(p, x1, y1, x2 - pwby2, y2, r1, r2, RightEdge, s2, c);
            break;
        default:
            break;
        }
    } else if ((s == BorderStyle_Outset && (edge == TopEdge || edge == LeftEdge))
            || (s == BorderStyle_Inset && (edge == BottomEdge || edge == RightEdge)))
            c = c.light();

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    qreal pwby2 = pw/2;
    p->setBrush(Qt::NoBrush);
    QPen pen = qPenFromStyle(c, pw, s);
    pen.setCapStyle(Qt::SquareCap); // this eliminates the offby1 errors that we might hit below
    p->setPen(pen);
    switch (edge) {
    case TopEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 - r1.width() + pwby2, y1 + pwby2,
                              2*r1.width() - pw, 2*r1.height() - pw), 135*16, -45*16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - r2.width() + pwby2, y1 + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 45*16, 45*16);
        break;
    case BottomEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 - r1.width() + pwby2, y2 - 2*r1.height() + pwby2,
                              2*r1.width() - pw, 2*r1.height() - pw), -90 * 16, -45 * 16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - r2.width() + pwby2, y2 - 2*r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), -90 * 16, 45 * 16);
        break;
    case LeftEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x1 + pwby2, y1 - r1.height() + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 135*16, 45*16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x1 + pwby2, y2 - r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 180*16, 45*16);
        break;
    case RightEdge:
        if (!r1.isEmpty())
            p->drawArc(QRectF(x2 - 2*r1.width() + pwby2, y1 - r1.height() + pwby2,
                       2*r1.width() - pw, 2*r1.height() - pw), 45*16, -45*16);
        if (!r2.isEmpty())
            p->drawArc(QRectF(x2 - 2*r2.width() + pwby2, y2 - r2.height() + pwby2,
                       2*r2.width() - pw, 2*r2.height() - pw), 315*16, 45*16);
        break;
    default:
        break;
    }
    p->restore();
}


void qDrawEdge(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2, qreal dw1, qreal dw2,
               Edge edge, BorderStyle style, QColor c)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const qreal width = (edge == TopEdge || edge == BottomEdge) ? (y2-y1) : (x2-x1);

    if (width <= 2 && style == BorderStyle_Double)
        style = BorderStyle_Solid;

    switch (style) {
    case BorderStyle_Inset:
    case BorderStyle_Outset:
        if (style == BorderStyle_Outset && (edge == TopEdge || edge == LeftEdge)
            || (style == BorderStyle_Inset && (edge == BottomEdge || edge == RightEdge)))
            c = c.light();
        // fall through!
    case BorderStyle_Solid: {
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        if (width == 1 || (dw1 == 0 && dw2 == 0)) {
            p->drawRect(QRectF(x1, y1, x2-x1, y2-y1));
        } else { // draw trapezoid
            QPolygonF quad;
            switch (edge) {
            case TopEdge:
                quad << QPointF(x1, y1) << QPointF(x1 + dw1, y2)
                     << QPointF(x2 - dw2, y2) << QPointF(x2, y1);
                break;
            case BottomEdge:
                quad << QPointF(x1 + dw1, y1) << QPointF(x1, y2)
                     << QPointF(x2, y2) << QPointF(x2 - dw2, y1);
                break;
            case LeftEdge:
                quad << QPointF(x1, y1) << QPointF(x1, y2)
                     << QPointF(x2, y2 - dw2) << QPointF(x2, y1 + dw1);
                break;
            case RightEdge:
                quad << QPointF(x1, y1 + dw1) << QPointF(x1, y2 - dw2)
                     << QPointF(x2, y2) << QPointF(x2, y1);
                break;
            default:
                break;
            }
            p->drawConvexPolygon(quad);
        }
        break;
    }
    case BorderStyle_Dotted:
    case BorderStyle_Dashed:
    case BorderStyle_DotDash:
    case BorderStyle_DotDotDash:
        p->setPen(qPenFromStyle(c, width, style));
        if (width == 1)
            p->drawLine(QLineF(x1, y1, x2 - 1, y2 - 1));
        else if (edge == TopEdge || edge == BottomEdge)
            p->drawLine(QLineF(x1 + width/2, (y1 + y2)/2, x2 - width/2, (y1 + y2)/2));
        else
            p->drawLine(QLineF((x1+x2)/2, y1 + width/2, (x1+x2)/2, y2 - width/2));
        break;

    case BorderStyle_Double: {
        int wby3 = qRound(width/3);
        int dw1by3 = qRound(dw1/3);
        int dw2by3 = qRound(dw2/3);
        switch (edge) {
        case TopEdge:
            qDrawEdge(p, x1, y1, x2, y1 + wby3, dw1by3, dw2by3, TopEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x1 + dw1 - dw1by3, y2 - wby3, x2 - dw2 + dw1by3, y2,
                      dw1by3, dw2by3, TopEdge, BorderStyle_Solid, c);
            break;
        case LeftEdge:
            qDrawEdge(p, x1, y1, x1 + wby3, y2, dw1by3, dw2by3, LeftEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x2 - wby3, y1 + dw1 - dw1by3, x2, y2 - dw2 + dw2by3, dw1by3, dw2by3,
                      LeftEdge, BorderStyle_Solid, c);
            break;
        case BottomEdge:
            qDrawEdge(p, x1 + dw1 - dw1by3, y1, x2 - dw2 + dw2by3, y1 + wby3, dw1by3, dw2by3,
                      BottomEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x1, y2 - wby3, x2, y2, dw1by3, dw2by3, BottomEdge, BorderStyle_Solid, c);
            break;
        case RightEdge:
            qDrawEdge(p, x2 - wby3, y1, x2, y2, dw1by3, dw2by3, RightEdge, BorderStyle_Solid, c);
            qDrawEdge(p, x1, y1 + dw1 - dw1by3, x1 + wby3, y2 - dw2 + dw2by3, dw1by3, dw2by3,
                      RightEdge, BorderStyle_Solid, c);
            break;
        default:
            break;
        }
        break;
    }
    case BorderStyle_Ridge:
    case BorderStyle_Groove: {
        BorderStyle s1, s2;
        if (style == BorderStyle_Groove) {
            s1 = BorderStyle_Inset;
            s2 = BorderStyle_Outset;
        } else {
            s1 = BorderStyle_Outset;
            s2 = BorderStyle_Inset;
        }
        int dw1by2 = qFloor(dw1/2), dw2by2 = qFloor(dw2/2);
        int wby2 = qRound(width/2);
        switch (edge) {
        case TopEdge:
            qDrawEdge(p, x1, y1, x2, y1 + wby2, dw1by2, dw2by2, TopEdge, s1, c);
            qDrawEdge(p, x1 + dw1by2, y1 + wby2, x2 - dw2by2, y2, dw1by2, dw2by2, TopEdge, s2, c);
            break;
        case BottomEdge:
            qDrawEdge(p, x1, y1 + wby2, x2, y2, dw1by2, dw2by2, BottomEdge, s1, c);
            qDrawEdge(p, x1 + dw1by2, y1, x2 - dw2by2, y1 + wby2, dw1by2, dw2by2, BottomEdge, s2, c);
            break;
        case LeftEdge:
            qDrawEdge(p, x1, y1, x1 + wby2, y2, dw1by2, dw2by2, LeftEdge, s1, c);
            qDrawEdge(p, x1 + wby2, y1 + dw1by2, x2, y2 - dw2by2, dw1by2, dw2by2, LeftEdge, s2, c);
            break;
        case RightEdge:
            qDrawEdge(p, x1 + wby2, y1, x2, y2, dw1by2, dw2by2, RightEdge, s1, c);
            qDrawEdge(p, x1, y1 + dw1by2, x1 + wby2, y2 - dw2by2, dw1by2, dw2by2, RightEdge, s2, c);
            break;
        default:
            break;
        }
    }
    default:
        break;
    }
    p->restore();
}

// Determines if Edge e1 draws over Edge e2. Depending on this trapezoids or rectanges are drawn
bool QRenderRule::paintsOver(Edge e1, Edge e2)
{
    BorderStyle s1 = bd->styles[e1];
    BorderStyle s2 = bd->styles[e2];

    if (s2 == BorderStyle_None || bd->colors[e2] == Qt::transparent)
        return true;

    if ((s1 == BorderStyle_Solid && s2 == BorderStyle_Solid) && (bd->colors[e1] == bd->colors[e2]))
        return true;

    return false;
}

static void qDrawCenterTiledPixmap(QPainter *p, const QRectF& r, const QPixmap& pix)
{
    p->drawTiledPixmap(r, pix, QPoint(pix.width() - int(r.width())%pix.width(),
                                      pix.height() - int(r.height())%pix.height()));
}

// Note: Round is not supported
void QRenderRule::drawBorderImage(QPainter *p, const QRect& rect)
{
    const QRectF br(rect);
    const int *borders = border()->borders;
    const int &l = borders[LeftEdge], &r = borders[RightEdge],
              &t = borders[TopEdge],  &b = borders[BottomEdge];
    QRectF pr = br.adjusted(l, t, -r, -b);

    QPainter::RenderHints oldHints = p->renderHints();
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    const QStyleSheetBorderImageData *bi = border()->borderImage();
    const QPixmap& pix = bi->pixmap;
    const int *c = bi->cuts;
    QRectF tlc(0, 0, c[LeftEdge], c[TopEdge]);
    if (tlc.isValid())
        p->drawPixmap(QRectF(br.topLeft(), QSizeF(l, t)), pix, tlc);
    QRectF trc(pix.width() - c[RightEdge], 0, c[RightEdge], c[TopEdge]);
    if (trc.isValid())
        p->drawPixmap(QRectF(br.left() + br.width() - r, br.y(), r, t), pix, trc);
    QRectF blc(0, pix.height() - c[BottomEdge], c[LeftEdge], c[BottomEdge]);
    if (blc.isValid())
        p->drawPixmap(QRectF(br.x(), br.y() + br.height() - b, l, b), pix, blc);
    QRectF brc(pix.width() - c[RightEdge], pix.height() - c[BottomEdge],
               c[RightEdge], c[BottomEdge]);
    if (brc.isValid())
        p->drawPixmap(QRectF(br.x() + br.width() - r, br.y() + br.height() - b, r, b),
                      pix, brc);

    QRectF topEdgeRect(br.x() + l, br.y(), pr.width(), t);
    QRectF bottomEdgeRect(br.x() + l, br.y() + br.height() - b, pr.width(), b);

    switch (bi->horizStretch) {
    case TileMode_Stretch:
        if (bi->topEdgeRect.isValid())
            p->drawPixmap(topEdgeRect, pix, bi->topEdgeRect);
        if (bi->bottomEdgeRect.isValid())
            p->drawPixmap(bottomEdgeRect, pix, bi->bottomEdgeRect);
        if (bi->middleRect.isValid()) {
            if (bi->vertStretch == TileMode_Stretch)
                p->drawPixmap(pr, pix, bi->middleRect);
            else if (bi->vertStretch == TileMode_Repeat) {
                QPixmap scaled = bi->middle.scaled(int(pr.width()), bi->middle.height());
                qDrawCenterTiledPixmap(p, pr, scaled);
            }
        }
        break;
    case TileMode_Repeat:
        if (!bi->topEdge.isNull())
            qDrawCenterTiledPixmap(p, topEdgeRect, bi->topEdge);
        if (!bi->bottomEdge.isNull())
            qDrawCenterTiledPixmap(p, bottomEdgeRect, bi->bottomEdge);
        if (bi->middleRect.isValid()) {
            if (bi->vertStretch == TileMode_Repeat) {
                qDrawCenterTiledPixmap(p, pr, bi->middle);
            } else if (bi->vertStretch == TileMode_Stretch) {
                QPixmap scaled = bi->middle.scaled(bi->middle.width(), int(pr.height()));
                qDrawCenterTiledPixmap(p, pr, scaled);
            }
        }
        break;
    case TileMode_Round:
        if (!bi->topEdge.isNull()) {
            int rwh = (int)pr.width()/ceil(pr.width()/bi->topEdge.width());
            QPixmap scaled = bi->topEdge.scaled(rwh, bi->topEdge.height());
            int blank = int(pr.width()) % rwh;
            p->drawTiledPixmap(QRectF(br.x() + l + blank/2, br.y(), pr.width() - blank, t),
                               scaled);
        }
        if (!bi->bottomEdge.isNull()) {
            int rwh = (int) pr.width()/ceil(pr.width()/bi->bottomEdge.width());
            QPixmap scaled = bi->bottomEdge.scaled(rwh, bi->bottomEdge.height());
            int blank = int(pr.width()) % rwh;
            p->drawTiledPixmap(QRectF(br.x() + l+ blank/2, br.y()+br.height()-b,
                                      pr.width() - blank, b), scaled);
        }
        break;
    default:
        break;
    }

    QRectF leftEdgeRect(br.x(), br.y() + t, l, pr.height());
    QRectF rightEdgeRect(br.x() + br.width()- r, br.y() + t, r, pr.height());

    switch (bi->vertStretch) {
    case TileMode_Stretch:
         if (bi->leftEdgeRect.isValid())
              p->drawPixmap(leftEdgeRect, pix, bi->leftEdgeRect);
        if (bi->rightEdgeRect.isValid())
            p->drawPixmap(rightEdgeRect, pix, bi->rightEdgeRect);
        break;
    case TileMode_Repeat:
        if (!bi->leftEdge.isNull())
            qDrawCenterTiledPixmap(p, leftEdgeRect, bi->leftEdge);
        if (!bi->rightEdge.isNull())
            qDrawCenterTiledPixmap(p, rightEdgeRect, bi->rightEdge);
        break;
    case TileMode_Round:
        if (!bi->leftEdge.isNull()) {
            int rwh = (int) pr.height()/ceil(pr.height()/bi->leftEdge.height());
            QPixmap scaled = bi->leftEdge.scaled(bi->leftEdge.width(), rwh);
            int blank = int(pr.height()) % rwh;
            p->drawTiledPixmap(QRectF(br.x(), br.y() + t + blank/2, l, pr.height() - blank),
                               scaled);
        }
        if (!bi->rightEdge.isNull()) {
            int rwh = (int) pr.height()/ceil(pr.height()/bi->rightEdge.height());
            QPixmap scaled = bi->rightEdge.scaled(bi->rightEdge.width(), rwh);
            int blank = int(pr.height()) % rwh;
            p->drawTiledPixmap(QRectF(br.x() + br.width() - r, br.y()+t+blank/2, r,
                                      pr.height() - blank), scaled);
        }
        break;
    default:
        break;
    }

    p->setRenderHints(oldHints);
}

QRect QRenderRule::originRect(const QRect &rect, Origin origin) const
{
    switch (origin) {
    case Origin_Padding:
        return paddingRect(rect);
    case Origin_Border:
        return borderRect(rect);
    case Origin_Content:
        return contentsRect(rect);
    case Origin_Margin:
    default:
        return rect;
    }
}

void QRenderRule::drawBackgroundImage(QPainter *p, const QRect &rect)
{
    Q_ASSERT(hasBackground());
    const QPixmap& bgp = background()->pixmap;
    if (bgp.isNull())
        return;
    QRect r = originRect(rect, background()->origin);
    QRect aligned = QStyle::alignedRect(Qt::LeftToRight, background()->position, bgp.size(), r);
    QRect inter = aligned.intersected(r);

    switch (background()->repeat) {
    case Repeat_Y:
        p->drawTiledPixmap(inter.x(), r.y(), inter.width(), r.height(), bgp,
                           inter.x() - aligned.x(),
                           bgp.height() - int(aligned.y() - r.y()) % bgp.height());
        break;
    case Repeat_X:
        p->drawTiledPixmap(r.x(), inter.y(), r.width(), inter.height(), bgp,
                           bgp.width() - int(aligned.x() - r.x())%bgp.width(),
                           inter.y() - aligned.y());
        break;
    case Repeat_XY:
        p->drawTiledPixmap(r, bgp,
                           QPoint(bgp.width() - int(aligned.x() - r.x())% bgp.width(),
                                  bgp.height() - int(aligned.y() - r.y())%bgp.height()));
        break;
    case Repeat_None:
    default:
        p->drawPixmap(inter.x(), inter.y(), bgp, inter.x() - aligned.x(),
                      inter.y() - aligned.y(), inter.width(), inter.height());
        break;
    }
}

void QRenderRule::getRadii(const QRect &br, QSize *tlr, QSize *trr, QSize *blr, QSize *brr) const
{
    Q_ASSERT(hasBorder());
    const QSize *radii = border()->radii;
    *tlr = radii[0];
    *trr = radii[1];
    *blr = radii[2];
    *brr = radii[3];
    if (tlr->width() + trr->width() > br.width()
        || blr->width() + brr->width() > br.width())
        *tlr = *blr = QSize(0, 0);
    if (tlr->height() + trr->height() > br.height()
        || blr->height() + brr->height() > br.height())
        *trr = *brr = QSize(0, 0);
}

void QRenderRule::drawBorder(QPainter *p, const QRect& rect)
{
    Q_ASSERT(hasBorder());
    const QRectF br(rect);
    if (border()->hasBorderImage()) {
        drawBorderImage(p, rect);
        return;
    }

    const BorderStyle *styles = border()->styles;
    const int *borders = border()->borders;
    const QColor *colors = border()->colors;

    QSize tlr, trr, blr, brr;
    getRadii(rect, &tlr, &trr, &blr, &brr);

    // Drawn in increasing order of precendence
    if (styles[BottomEdge] != BorderStyle_None) {
        qreal dw1 = (blr.width() || paintsOver(BottomEdge, LeftEdge)) ? 0 : borders[LeftEdge];
        qreal dw2 = (brr.width() || paintsOver(BottomEdge, RightEdge)) ? 0 : borders[RightEdge];
        qreal x1 = br.x() + blr.width();
        qreal y1 = br.y() + br.height() - borders[BottomEdge];
        qreal x2 = br.x() + br.width() - brr.width();
        qreal y2 = br.y() + br.height() ;

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, BottomEdge, styles[BottomEdge], colors[BottomEdge]);
        if (blr.width() || brr.width())
            qDrawRoundedCorners(p, x1, y1, x2, y2, blr, brr, BottomEdge, styles[BottomEdge], colors[BottomEdge]);
    }
    if (styles[RightEdge] != BorderStyle_None) {
        qreal dw1 = (trr.height() || paintsOver(RightEdge, TopEdge)) ? 0 : borders[TopEdge];
        qreal dw2 = (brr.height() || paintsOver(RightEdge, BottomEdge)) ? 0 : borders[BottomEdge];
        qreal x1 = br.x() + br.width() - borders[RightEdge];
        qreal y1 = br.y() + trr.height();
        qreal x2 = br.x() + br.width();
        qreal y2 = br.y() + br.height() - brr.height();

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, RightEdge, styles[RightEdge], colors[RightEdge]);
        if (trr.height() || brr.height())
            qDrawRoundedCorners(p, x1, y1, x2, y2, trr, brr, RightEdge, styles[RightEdge], colors[RightEdge]);
    }
    if (styles[LeftEdge] != BorderStyle_None) {
        qreal dw1 = (tlr.height() || paintsOver(LeftEdge, TopEdge)) ? 0 : borders[TopEdge];
        qreal dw2 = (blr.height() || paintsOver(LeftEdge, BottomEdge)) ? 0 : borders[BottomEdge];
        qreal x1 = br.x();
        qreal y1 = br.y() + tlr.height();
        qreal x2 = br.x() + borders[LeftEdge];
        qreal y2 = br.y() + br.height() - blr.height();

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, LeftEdge, styles[LeftEdge], colors[LeftEdge]);
        if (tlr.height() || blr.height())
            qDrawRoundedCorners(p, x1, y1, x2, y2, tlr, blr, LeftEdge, styles[LeftEdge], colors[LeftEdge]);
    }
    if (styles[TopEdge] != BorderStyle_None) {
        qreal dw1 = (tlr.width() || paintsOver(TopEdge, LeftEdge)) ? 0 : borders[LeftEdge];
        qreal dw2 = (trr.width() || paintsOver(TopEdge, RightEdge)) ? 0 : borders[RightEdge];
        qreal x1 = br.x() + tlr.width();
        qreal y1 = br.y();
        qreal x2 = br.left() + br.width() - trr.width();
        qreal y2 = br.y() + borders[TopEdge];

        qDrawEdge(p, x1, y1, x2, y2, dw1, dw2, TopEdge, styles[TopEdge], colors[TopEdge]);
        if (tlr.width() || trr.width())
            qDrawRoundedCorners(p, x1, y1, x2, y2, tlr, trr, TopEdge, styles[TopEdge], colors[TopEdge]);
    }
}

void QRenderRule::drawBackground(QPainter *p, const QRect& rect)
{
    if (!hasBackground())
        return;

    if (background()->brush.style() != Qt::NoBrush) {
        QRect fillRect = borderRect(rect);
        QBrush brush = background()->brush;
        if (brush.style() >= Qt::LinearGradientPattern
            && brush.style() <= Qt::ConicalGradientPattern) {
            QMatrix m;
            m.translate(fillRect.left(), fillRect.top());
            m.scale(fillRect.width(), fillRect.height());
            brush.setMatrix(m);
        }
        p->fillRect(fillRect, brush);
    }
    drawBackgroundImage(p, rect);
}

void QRenderRule::drawFrame(QPainter *p, const QRect& rect)
{
    drawBackground(p, rect);
    if (hasBorder())
        drawBorder(p, borderRect(rect));
}

void QRenderRule::drawImage(QPainter *p, const QRect &rect)
{
    if (!hasImage())
        return;
    // draw center aligned
    QRect aligned = QStyle::alignedRect(Qt::LeftToRight, QFlag(Qt::AlignCenter), imageRect.size(), rect);
    QRect inter = aligned.intersected(rect);
    p->drawPixmap(inter.x(), inter.y(), image, imageRect.x() + inter.x() - aligned.x(),
                  imageRect.y() + inter.y() - aligned.y(), inter.width(), inter.height());
}

void QRenderRule::drawRule(QPainter *p, const QRect& rect)
{
    drawFrame(p, rect);
    drawImage(p, contentsRect(rect));
}

// *shudder* , *horror*, *whoa* <-- what you might feel when you see the functions below
void QRenderRule::configurePalette(QPalette *p, QPalette::ColorRole fr, QPalette::ColorRole br)
{
    if (bg && bg->brush.style() != Qt::NoBrush) {
        p->setBrush(br, bg->brush);
        p->setBrush(QPalette::Window, bg->brush);
    }

    if (!hasPalette())
        return;

    if (pal->foreground.isValid()) {
        p->setBrush(fr, pal->foreground);
        p->setBrush(QPalette::WindowText, pal->foreground);
        p->setBrush(QPalette::Text, pal->foreground);
    }
    if (pal->selectionBackground.style() != Qt::NoBrush)
        p->setBrush(QPalette::Highlight, pal->selectionBackground);
    if (pal->selectionForeground.isValid())
        p->setBrush(QPalette::HighlightedText, pal->selectionForeground);
    if (pal->alternateBackground.style() != Qt::NoBrush)
        p->setBrush(QPalette::AlternateBase, pal->alternateBackground);
}

void QRenderRule::configurePalette(QPalette *p, QPalette::ColorGroup cg, const QWidget *w, bool embedded)
{
#ifdef QT_NO_COMBOBOX
    const bool isReadOnlyCombo = false;
#else
    const bool isReadOnlyCombo = qobject_cast<const QComboBox *>(w) != 0;
#endif

    if (bg && bg->brush.style() == Qt::SolidPattern) {
        if (isReadOnlyCombo) {
            p->setBrush(cg, QPalette::Base, bg->brush); // for windows, windowxp
            p->setBrush(cg, QPalette::Button, bg->brush); // for plastique
        } else {
            p->setBrush(cg, w->backgroundRole(), bg->brush);
            //p->setBrush(cg, QPalette::Window, bg->brush);
        }
    }

    if (embedded) {
        /* For embedded widgets (ComboBox, SpinBox and ScrollArea) we want the embedded widget
         * to be transparent when we have a transparent background or border image */
        if ((hasBackground() && background()->isTransparent())
            || (hasBorder() && border()->hasBorderImage() && border()->borderImage()->middleRect.isValid()))
            p->setBrush(cg, w->backgroundRole(), Qt::transparent);
    }

    if (!hasPalette())
        return;

    if (pal->foreground.isValid()) {
        if (isReadOnlyCombo) {
            p->setBrush(cg, QPalette::ButtonText, pal->foreground);
        } else {
            p->setBrush(cg, w->foregroundRole(), pal->foreground);
            p->setBrush(cg, QPalette::WindowText, pal->foreground);
        }
        p->setBrush(cg, QPalette::Text, pal->foreground);
    }
    if (pal->selectionBackground.style() != Qt::NoBrush)
        p->setBrush(cg, QPalette::Highlight, pal->selectionBackground);
    if (pal->selectionForeground.isValid())
        p->setBrush(cg, QPalette::HighlightedText, pal->selectionForeground);
    if (pal->alternateBackground.style() != Qt::NoBrush)
        p->setBrush(cg, QPalette::AlternateBase, pal->alternateBackground);
}

///////////////////////////////////////////////////////////////////////////////
// Style rules
#define WIDGET(x) (static_cast<QWidget *>(x.ptr))

class QStyleSheetStyleSelector : public StyleSelector
{
public:
    QStyleSheetStyleSelector() { }

    bool nodeNameEquals(NodePtr node, const QString& name) const
    {
        if (WIDGET(node)->inherits(name.toLatin1()))
            return true;
        if (name == QLatin1String("QToolTip")
            && QLatin1String("QTipLabel")== QLatin1String(WIDGET(node)->metaObject()->className()))
            return true;
        return false;
    }
    QString attribute(NodePtr node, const QString& name) const
    {
        return (name == "class" && !WIDGET(node)->property("class").isValid())
            ? WIDGET(node)->metaObject()->className()
            : WIDGET(node)->property(name.toLatin1()).toString();
    }
    bool hasAttribute(NodePtr node, const QString& name) const
    { return name == "class"
             || WIDGET(node)->metaObject()->indexOfProperty(name.toLatin1()) != -1
             || WIDGET(node)->dynamicPropertyNames().contains(name.toLatin1()); }
    bool hasAttributes(NodePtr) const
    { return true; }
    QStringList nodeIds(NodePtr node) const
    { return QStringList(WIDGET(node)->objectName()); }
    bool isNullNode(NodePtr node) const
    { return node.ptr == 0; }
    NodePtr parentNode(NodePtr node)
    { NodePtr n; n.ptr = WIDGET(node)->parentWidget(); return n; }
    NodePtr previousSiblingNode(NodePtr)
    { NodePtr n; n.ptr = 0; return n; }
    NodePtr duplicateNode(NodePtr node)
    { return node; }
    void freeNode(NodePtr)
    { }
};

static QVector<QCss::StyleRule> styleRules(QWidget *w)
{
    QStyleSheetStyleSelector styleSelector;
    if (!qApp->styleSheet().isEmpty()) {
        StyleSheet appSs;
        if (!styleSheetCache->contains(0)) {
            Parser parser1(qApp->styleSheet());
            if (!parser1.parse(&appSs))
                qWarning("Could not parse application stylesheet");
           appSs.origin = StyleSheetOrigin_Author;
            styleSheetCache->insert(0, appSs);
        } else {
            appSs = styleSheetCache->value(0);
        }
        styleSelector.styleSheets += appSs;
    }

    QList<QCss::StyleSheet> widgetSs;
    for (QWidget *wid = w; wid; wid = wid->parentWidget()) {
        if (wid->styleSheet().isEmpty())
            continue;
        StyleSheet ss;
        if (!styleSheetCache->contains(wid)) {
            Parser parser(wid->styleSheet());
            if (!parser.parse(&ss) && wid == w) {
                Parser parser2(QLatin1String("* {") + wid->styleSheet() + QLatin1String("}"));
                if (!parser2.parse(&ss))
                   qWarning("Could not parse stylesheet of widget %p", wid);
            }
            ss.origin = StyleSheetOrigin_Inline;
            styleSheetCache->insert(wid, ss);
        } else {
            ss = styleSheetCache->value(wid);
        }
        widgetSs.prepend(ss);
    }

    for (int i = 0; i < widgetSs.count(); i++)
        widgetSs[i].depth = i + 1;

    styleSelector.styleSheets += widgetSs;

    StyleSelector::NodePtr n;
    n.ptr = w;
    return styleSelector.styleRulesForNode(n);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rendering rules
enum PseudoElement {
    PseudoElement_None,
    PseudoElement_DownArrow,
    PseudoElement_UpArrow,
    PseudoElement_LeftArrow,
    PseudoElement_RightArrow,
    PseudoElement_Indicator,
    PseudoElement_MenuIndicator,
    PseudoElement_DropDown,
    PseudoElement_DropDownArrow,
    PseudoElement_Item,
    PseudoElement_SpinBoxUpButton,
    PseudoElement_SpinBoxUpArrow,
    PseudoElement_SpinBoxDownButton,
    PseudoElement_SpinBoxDownArrow,
    PseudoElement_GroupBoxTitle,
    PseudoElement_GroupBoxIndicator,
    NumPseudoElements
};

static struct PseudoElementInfo {
    QStyle::SubControl subControl;
    const char *name;
} knownPseudoElements[NumPseudoElements] = {
    { QStyle::SC_None, "", },
    { QStyle::SC_None, "down-arrow" },
    { QStyle::SC_None, "up-arrow" },
    { QStyle::SC_None, "left-arrow" },
    { QStyle::SC_None, "right-arrow" },
    { QStyle::SC_None, "indicator" },
    { QStyle::SC_None, "menu-indicator" },
    { QStyle::SC_ComboBoxArrow, "drop-down" },
    { QStyle::SC_ComboBoxArrow, "down-arrow" },
    { QStyle::SC_None, "item" },
    { QStyle::SC_SpinBoxUp, "up-button" },
    { QStyle::SC_SpinBoxUp, "up-arrow" },
    { QStyle::SC_SpinBoxDown, "down-button" },
    { QStyle::SC_SpinBoxDown, "down-arrow" },
    { QStyle::SC_GroupBoxLabel, "title" },
    { QStyle::SC_GroupBoxCheckBox, "indicator" }
};

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QString &part, QStyle::State state) const
{
    Q_ASSERT(w);
    Q_ASSERT(styleRulesCache->contains(w)); // style sheet rules must have been computed!
    const QVector<StyleRule> styleRules = styleRulesCache->value(w);
    QHash<int, QRenderRule> &renderRules = (*renderRulesCache)[w][part];

    int pseudoState = (state & QStyle::State_Enabled)
                                 ? PseudoState_Enabled : PseudoState_Disabled;
    if (state & QStyle::State_Sunken)
        pseudoState |= PseudoState_Pressed;
    if (state & QStyle::State_MouseOver || state & QStyle::State_Selected)
        pseudoState |= PseudoState_Hover;
    if (state & QStyle::State_HasFocus)
        pseudoState |= PseudoState_Focus;
    if (state & QStyle::State_On)
        pseudoState |= PseudoState_On;
    if (state & QStyle::State_Off)
        pseudoState |= PseudoState_Off;
    if (state & QStyle::State_NoChange)
        pseudoState |= PseudoState_Indeterminate;

    if (renderRules.contains(pseudoState))
        return renderRules[pseudoState]; // already computed before

    QVector<Declaration> declarations;
    for (int i = 0; i < styleRules.count(); i++) {
        const Selector& selector = styleRules.at(i).selectors.at(0);
        // Rules with pseudo elements don't cascade. This is an intentional
        // diversion for CSS
        if (part.compare(selector.pseudoElement(), Qt::CaseInsensitive) != 0)
            continue;
        const int cssState = selector.pseudoState();
        if ((cssState == PseudoState_Unspecified) || ((cssState & pseudoState) == cssState))
            declarations += styleRules.at(i).declarations;
    }

    QRenderRule newRule(declarations, w, part, baseStyle());
    renderRules[pseudoState] = newRule;
    return newRule;
}


QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, int pseudoElement, QStyle::State state) const
{
    return renderRule(w, knownPseudoElements[pseudoElement].name, state);
}

QRenderRule QStyleSheetStyle::renderRule(const QWidget *w, const QStyleOption *opt, int pseudoElement) const
{
    Q_ASSERT(w && !styleRulesCache->value(w).isEmpty());
    QStyle::State state = opt ? opt->state : QStyle::State(QStyle::State_None);

    if (const QStyleOptionComplex *complex = qstyleoption_cast<const QStyleOptionComplex *>(opt)) {
        if (pseudoElement != PseudoElement_None) {
            // if not an active subcontrol, just pass enabled/disabled
            QStyle::SubControl subControl = knownPseudoElements[pseudoElement].subControl;

            if (!(complex->activeSubControls & subControl))
                state = QStyle::State(opt->state & QStyle::State_Enabled);
        }

        switch (pseudoElement) {
        case PseudoElement_DropDown:
        case PseudoElement_DropDownArrow:
            state |= (opt->state & QStyle::State_On); // propagate popup state as on/off
            break;
        case PseudoElement_SpinBoxUpButton:
        case PseudoElement_SpinBoxDownButton:
        case PseudoElement_SpinBoxUpArrow:
        case PseudoElement_SpinBoxDownArrow:
#ifndef QT_NO_SPINBOX
            if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
                bool on = false;
                bool up = pseudoElement == PseudoElement_SpinBoxUpButton
                          || pseudoElement == PseudoElement_SpinBoxUpArrow;
                if ((sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) && up)
                    on = true;
                else if ((sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) && !up)
                    on = true;
                state |= (on ? QStyle::State_On : QStyle::State_Off);
            }
#endif // QT_NO_SPINBOX
            break;
        case PseudoElement_GroupBoxTitle:
            state |= (opt->state & (QStyle::State_MouseOver | QStyle::State_Sunken));
            break;
        case PseudoElement_None:
        default:
            // QStyle::State_On is set when the popup is being shown
            // Propagate EditField Pressed state
            if ((complex->activeSubControls & QStyle::SC_ComboBoxEditField)
                && (!(opt->state & QStyle::State_MouseOver)))
                state = opt->state | QStyle::State_Sunken;
        }
    } else {
        // Add hacks for simple controls here
#ifndef QT_NO_LINEEDIT
        // LineEdit sets Sunken flag to indicate Sunken frame (argh)
        if (qobject_cast<const QLineEdit *>(w)) {
            state &= ~QStyle::State_Sunken;
        } else
#endif
#ifndef QT_NO_MENUBAR
        if (qobject_cast<const QMenuBar *>(w)) {
            if (state & QStyle::State_Sunken) {
                state &= ~QStyle::State_Sunken;
                state |= QStyle::State_On;
            }
        } else
#endif
        { } // required for the above ifdef'ery
    }

    return renderRule(w, knownPseudoElements[pseudoElement].name, state);
}

bool QStyleSheetStyle::hasStyleRule(const QWidget *w, int part) const
{
    const QVector<StyleRule> &styleRules = styleRulesCache->value(w);
    if (part == PseudoElement_None)
        return w && !styleRules.isEmpty();

    // ### cache the result
    QString pseudoElement = knownPseudoElements[part].name;
    QVector<Declaration> declarations;
    for (int i = 0; i < styleRules.count(); i++) {
        const Selector& selector = styleRules.at(i).selectors.at(0);
        if (pseudoElement.compare(selector.pseudoElement(), Qt::CaseInsensitive) == 0)
            return true;
    }

    return false;
}

static Qt::Alignment defaultPosition(int pe)
{
    switch (pe) {
    case PseudoElement_SpinBoxUpButton:
    case PseudoElement_SpinBoxDownButton:
        return (pe == PseudoElement_SpinBoxUpButton) ? Qt::AlignTop | Qt::AlignRight
                                                     : Qt::AlignBottom | Qt::AlignRight;
    case PseudoElement_Indicator:
        return Qt::AlignLeft | Qt::AlignVCenter;
    case PseudoElement_DropDown:
        return Qt::AlignTop | Qt::AlignRight;
    case PseudoElement_DropDownArrow:
    case PseudoElement_SpinBoxDownArrow:
    case PseudoElement_SpinBoxUpArrow:
    case PseudoElement_DownArrow:
        return Qt::AlignCenter;
    case PseudoElement_MenuIndicator:
        return Qt::AlignBottom | Qt::AlignRight;
    case PseudoElement_GroupBoxTitle:
    case PseudoElement_GroupBoxIndicator:
        return Qt::AlignTop | Qt::AlignLeft;
    default:
        return 0;
    }
}

static QSize expandedSize(QSize sz, const QRect& rect, int pe)
{
    // some magic for relative positioning since we don't support % metrics yet
    switch (pe) {
    case PseudoElement_SpinBoxUpButton:
    case PseudoElement_SpinBoxDownButton:
        if (sz.height() == -1) 
            sz.setHeight(rect.height()/2);
        break;
    case PseudoElement_DropDown:
        if (sz.height() == -1) 
            sz.setHeight(rect.height());
        break;
    default:
        break;
    }

    // expand to rectangle
    if (sz.height() == -1)
        sz.setHeight(rect.height());
    if (sz.width() == -1)
        sz.setWidth(rect.width());

    return sz;
}

static QRect positionRect(const QRenderRule& rule1, const QRenderRule& rule2, int pe, const QRect& rect,
                          Qt::LayoutDirection dir)
{
    Origin defOrigin = Origin_Padding;
    Qt::Alignment defPosition = defaultPosition(pe);

    const QStyleSheetPositionData *p = rule2.position();
    Origin origin = (p && p->origin != Origin_Unknown) ? p->origin : defOrigin;
    QRect originRect = rule1.originRect(rect, origin);
    PositionMode mode = p ? p->mode : PositionMode_Static;
    Qt::Alignment position = (p && p->position != 0) ? p->position : defPosition;
    QRect r;

    if (mode != PositionMode_Absolute) {
        QSize sz = expandedSize(rule2.size(), originRect, pe);
        sz = sz.expandedTo(rule2.minimumContentsSize());
        r = QStyle::alignedRect(dir, position, sz, originRect);
        if (p) {
            int left = p->left ? p->left : -p->right;
            int top = p->top ? p->top : -p->bottom;
            r.translate(dir == Qt::LeftToRight ? left : -left, top);
        }
    } else {
        if (p)
            r = originRect.adjusted(dir == Qt::LeftToRight ? p->left : p->right, p->top, 
                                    dir == Qt::LeftToRight ? -p->right : -p->left, -p->bottom);
        if (rule2.hasContentsSize()) {
            QSize sz = expandedSize(rule2.size(), r, pe);
            sz = sz.expandedTo(rule2.minimumContentsSize());
            r = QStyle::alignedRect(dir, position, sz, r);
        }
    }
    return r;
}

static QWidget *embeddedWidget(QWidget *w)
{
#ifndef QT_NO_COMBOBOX
    if (QComboBox *cmb = qobject_cast<QComboBox *>(w))
        if (cmb->isEditable())
            return cmb->lineEdit();
        else
            return cmb;
#endif

#ifndef QT_NO_SPINBOX
    if (QAbstractSpinBox *sb = qobject_cast<QAbstractSpinBox *>(w))
        return qFindChild<QLineEdit *>(sb);
#endif

#ifndef QT_NO_SCROLLAREA
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w))
        return sa->viewport();
#endif

    return w;
}

// remember to revert changes in unsetPalette
void QStyleSheetStyle::setPalette(QWidget *w)
{
    const QRenderRule &hoverRule = renderRule(w, PseudoElement_None, QStyle::State_MouseOver);
    if (hoverRule.hasDrawable())
        w->setAttribute(Qt::WA_Hover);

    // put stuff for which palette is not set here
#ifndef QT_NO_MENUBAR
    if (qobject_cast<QMenuBar *>(w)) {
        // MenuBar has autoFillBackground set which does not play well with our stuff
        const QRenderRule& subRule = renderRule(w, PseudoElement_Item, QStyle::State_MouseOver);
        if (subRule.hasDrawable())
            w->setAttribute(Qt::WA_Hover);
        return;
    }
#endif

    struct RuleRoleMap {
        QStyle::StateFlag state;
        QPalette::ColorGroup group;
    } map[3] = {
        { QStyle::State_Enabled, QPalette::Active },
        { QStyle::State_None, QPalette::Disabled },
        { QStyle::State_Enabled, QPalette::Inactive }
    };

    QPalette p = w->palette();
    QWidget *ew = embeddedWidget(w);

    for (int i = 0; i < 3; i++) {
        QRenderRule rule = renderRule(w, PseudoElement_None, map[i].state);
        if (i == 0) {
            customFontWidgets->insert(w, rule.font.resolve() & ~w->font().resolve());
            w->setFont(rule.font);
        }

        rule.configurePalette(&p, map[i].group, ew, ew != w);
    }

    if (w->palette() != p) {
        customPaletteWidgets->insert(w, p.resolve() & ~w->palette().resolve());
        w->setPalette(p);
    }
}

static void unsetPalette(QWidget *w)
{
    if (customPaletteWidgets->contains(w)) {
        QPalette p = QApplication::palette(w);
        p.resolve(customPaletteWidgets->value(w));
        p.resolve(w->palette());
        w->setPalette(p);
        customPaletteWidgets->remove(w);
    }
    if (customFontWidgets->contains(w)) {
        QFont f = QApplication::font();
        f.resolve(customFontWidgets->value(w));
        f.resolve(w->font());
        w->setFont(f);
        customFontWidgets->remove(w);
    }
}

static void updateWidget(QWidget *widget)
{
    QEvent e(QEvent::StyleChange);
    QApplication::sendEvent(widget, &e);
    widget->update();
    widget->updateGeometry();
}

static void updateWidgets(const QList<const QWidget *>& widgets)
{
    for (int i = 0; i < widgets.size(); ++i) {
        QWidget *widget = const_cast<QWidget *>(widgets.at(i));
        Q_ASSERT(widget);
        styleRulesCache->remove(widget);
        renderRulesCache->remove(widget);
        widget->style()->polish(widget);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// The stylesheet style
QStyleSheetStyle::QStyleSheetStyle(QStyle *base)
: base(base), refcount(1)
{
    initStatics();
}

QStyle *QStyleSheetStyle::baseStyle() const
{
    if (base)
        return base;
    if (QStyleSheetStyle *me = qobject_cast<QStyleSheetStyle *>(qApp->style()))
        return me->base;
    return qApp->style();
}

void QStyleSheetStyle::widgetDestroyed(QObject *o)
{
    styleRulesCache->remove((const QWidget *)o);
    renderRulesCache->remove((const QWidget *)o);
    customPaletteWidgets->remove((const QWidget *)o);
}

static bool unstylable(QWidget *w)
{
    if (w->windowType() == Qt::Desktop)
        return true;

#ifndef QT_NO_SCROLLAREA
    if (QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(w->parentWidget())) {
        if (sa->viewport() == w)
            return true;
    } else
#endif
#ifndef QT_NO_LINEEDIT
    if (qobject_cast<QLineEdit *>(w)) {
        if (0
#ifndef QT_NO_COMBOBOX
            || qobject_cast<QComboBox *>(w->parentWidget())
#endif
#ifndef QT_NO_SPINBOX
            || qobject_cast<QAbstractSpinBox *>(w->parentWidget())
#endif
            )
            return true;
    }
#endif
    return false;
}

void QStyleSheetStyle::polish(QWidget *w)
{
    if (styleRulesCache->contains(w))
        return;

    baseStyle()->polish(w);
    if (unstylable(w))
        return;

    QObject::connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));
    renderRulesCache->remove(w);
    QVector<QCss::StyleRule> rules = styleRules(w);
    styleRulesCache->insert(w, rules);
    unsetPalette(w);
    if (!rules.isEmpty()) {
        setPalette(w);
    }
    updateWidget(w);
}

void QStyleSheetStyle::polish(QApplication *app)
{
    styleRulesCache->clear();
    renderRulesCache->clear();
    baseStyle()->polish(app);
}

void QStyleSheetStyle::polish(QPalette &pal)
{
    baseStyle()->polish(pal);
}

void QStyleSheetStyle::repolish(QWidget *w)
{
    QList<const QWidget *> children = qFindChildren<const QWidget *>(w, QString());
    children.append(w);
    styleSheetCache->remove(w);
    updateWidgets(children);
}

void QStyleSheetStyle::repolish(QApplication *)
{
    styleSheetCache->remove(0);
    updateWidgets(styleRulesCache->keys());
}

void QStyleSheetStyle::unpolish(QWidget *w)
{
    styleRulesCache->remove(w);
    renderRulesCache->remove(w);
    styleSheetCache->remove(w);
    baseStyle()->unpolish(w);
    unsetPalette(w);
    QObject::disconnect(w, SIGNAL(destroyed(QObject*)),
                       this, SLOT(widgetDestroyed(QObject*)));
}

void QStyleSheetStyle::unpolish(QApplication *app)
{
    styleRulesCache->clear();
    renderRulesCache->clear();
    styleSheetCache->remove(0);
    baseStyle()->unpolish(app);
}

void QStyleSheetStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                          const QWidget *w) const
{
    if (!hasStyleRule(w)) {
        baseStyle()->drawComplexControl(cc, opt, p, w);
        return;
    }

    QRenderRule rule = renderRule(w, opt);

    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (rule.hasBorder() || rule.hasBox() || hasStyleRule(w, PseudoElement_DropDown)) {
                rule.drawFrame(p, opt->rect);
                QRenderRule subRule = renderRule(w, opt, PseudoElement_DropDown);
                QRect r = positionRect(rule, subRule, PseudoElement_DropDown, opt->rect, opt->direction);
                subRule.drawRule(p, r);

                QRenderRule subRule2 = renderRule(w, opt, PseudoElement_DropDownArrow);
                r = positionRect(subRule, subRule2, PseudoElement_DropDownArrow, r, opt->direction);
                subRule2.drawRule(p, r);
            } else {
                baseStyle()->drawComplexControl(cc, cb, p, w);
            }
            return;
        }
        break;

    case CC_SpinBox:
        if (rule.hasBorder() || rule.hasBox() || hasStyleRule(w, PseudoElement_SpinBoxDownButton)
            || hasStyleRule(w, PseudoElement_SpinBoxUpButton)) {
            rule.drawFrame(p, opt->rect);

            // down button
            QRenderRule subRule1 = renderRule(w, opt, PseudoElement_SpinBoxDownButton);
            QRect r = positionRect(rule, subRule1, PseudoElement_SpinBoxDownButton, opt->rect, opt->direction);
            subRule1.drawRule(p, r);

            // down arrow
            QRenderRule subRule2 = renderRule(w, opt, PseudoElement_SpinBoxDownArrow);
            r = positionRect(subRule1, subRule2, PseudoElement_SpinBoxDownArrow, r, opt->direction);
            subRule2.drawRule(p, r);

            // up button border
            QRenderRule subRule3 = renderRule(w, opt, PseudoElement_SpinBoxUpButton);
            r = positionRect(rule, subRule3, PseudoElement_SpinBoxUpButton, opt->rect, opt->direction);
            subRule3.drawRule(p, r);

            // up arrow
            QRenderRule subRule4 = renderRule(w, opt, PseudoElement_SpinBoxUpArrow);
            r = positionRect(subRule3, subRule4, PseudoElement_SpinBoxUpArrow, r, opt->direction);
            subRule4.drawRule(p, r);

            return;
        }
        break;

    case CC_GroupBox:
        if (rule.hasDrawable() || rule.hasBox() || hasStyleRule(w, PseudoElement_GroupBoxTitle)) {
            if (const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
                QRect labelRect, checkBoxRect, titleRect;
                bool hasTitle = (gb->subControls & QStyle::SC_GroupBoxCheckBox) || !gb->text.isEmpty();
                QRenderRule titleRule = renderRule(w, opt, PseudoElement_GroupBoxTitle);

                bool clipSet = false;

                if (hasTitle) {
                    labelRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxLabel, w);
                    if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
                        checkBoxRect = subControlRect(CC_GroupBox, opt, SC_GroupBoxCheckBox, w);
                        titleRect = titleRule.boxRect(checkBoxRect.united(labelRect));
                    } else {
                        titleRect = titleRule.boxRect(labelRect);
                    }
                    if (titleRule.hasBackground() && !titleRule.background()->isTransparent()) {
                        clipSet = true;
                        p->save();
                        p->setClipRegion(QRegion(opt->rect) - titleRect);
                    }
                }

                rule.drawRule(p, opt->rect);

                if (clipSet)
                    p->restore();

                // draw background and frame of the title
                if (hasTitle)
                    titleRule.drawRule(p, titleRect);

                // draw the indicator
                if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
                    QStyleOptionButton box;
                    box.QStyleOption::operator=(*gb);
                    box.rect = checkBoxRect;
                    drawPrimitive(PE_IndicatorCheckBox, &box, p, w);
                    drawPrimitive(PE_IndicatorCheckBox, &box, p, w);
                }

                // draw the text
                if (!gb->text.isEmpty()) {
                    int alignment = int(Qt::AlignCenter);
                    if (!styleHint(QStyle::SH_UnderlineShortcut, opt, w))
                        alignment |= Qt::TextHideMnemonic;

                    QPalette pal = gb->palette;
                    pal.setColor(QPalette::WindowText, gb->textColor);
                    titleRule.configurePalette(&pal, QPalette::WindowText, QPalette::Window);
                    drawItemText(p, labelRect,  alignment, pal, gb->state & State_Enabled,
                                 gb->text, QPalette::WindowText);
                }
            }
            return;
        }
        break;

    default:
        break;
    }

    baseStyle()->drawComplexControl(cc, opt, p, w);
}

void QStyleSheetStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                          const QWidget *w) const
{
    if (!hasStyleRule(w)) {
        baseStyle()->drawControl(ce, opt, p, w);
        return;
    }

    QRenderRule rule = renderRule(w, opt);

    switch (ce) {
    // Push button
    case CE_PushButton:
        ParentStyle::drawControl(ce, opt, p, w);
        return;

    case CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton btnOpt(*btn);
            btnOpt.rect = rule.borderRect(opt->rect);
            if (rule.hasBox() || rule.hasDrawable()
                || ((btn->features & QStyleOptionButton::HasMenu) && hasStyleRule(w, PseudoElement_MenuIndicator))) {
                if (rule.hasBox() || rule.hasDrawable()) {
                    rule.drawFrame(p, opt->rect);
                } else {
                    btnOpt.features &= ~QStyleOptionButton::HasMenu;
                    baseStyle()->drawControl(ce, &btnOpt, p, w);
                }

                if (btn->features & QStyleOptionButton::HasMenu) {
                    QRenderRule subRule = renderRule(w, opt, PseudoElement_MenuIndicator);
                    QRect ir = positionRect(rule, subRule, PseudoElement_MenuIndicator, opt->rect, opt->direction);
                    if (subRule.hasImage()) {
                        subRule.drawImage(p, ir);
                    } else {
                        btnOpt.rect = ir;
                        baseStyle()->drawPrimitive(PE_IndicatorArrowDown, &btnOpt, p, w);
                    }
                }
            } else {
                baseStyle()->drawControl(ce, &btnOpt, p, w);
            }
        }
        return;

    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            rule.configurePalette(&butOpt.palette, QPalette::ButtonText, QPalette::Button);
            ParentStyle::drawControl(ce, &butOpt, p, w);
        }
        return;

    case CE_RadioButton:
    case CE_CheckBox:
        rule.drawFrame(p, opt->rect);
        ParentStyle::drawControl(ce, opt, p, w);
        return;

    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton butOpt(*btn);
            rule.configurePalette(&butOpt.palette, QPalette::ButtonText, QPalette::Button);
            ParentStyle::drawControl(ce, &butOpt, p, w);
        }
        return;

    case CE_Splitter:
        if (rule.hasDrawable()) {
            rule.drawRule(p, opt->rect);
            return;
        }
        break;

    case CE_ToolBar:
        if (rule.hasBackground()) {
            rule.drawBackground(p, opt->rect);
        }
        if (rule.hasBorder()) {
            rule.drawBorder(p, rule.borderRect(opt->rect));
        } else {
#ifndef QT_NO_TOOLBAR
            if (const QStyleOptionToolBar *tb = qstyleoption_cast<const QStyleOptionToolBar *>(opt)) {
                QStyleOptionToolBar newTb(*tb);
                newTb.rect = rule.borderRect(opt->rect);
                baseStyle()->drawControl(ce, &newTb, p, w);
            }
#endif // QT_NO_TOOLBAR
        }
        return;

    case CE_MenuEmptyArea:
    case CE_MenuBarEmptyArea:
        if (rule.hasBackground()) {
            rule.drawBackground(p, opt->rect);
            return;
        }
        break;

    case CE_MenuBarItem:
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *m = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            QStyleOptionMenuItem mi(*m);
            QRenderRule subRule = renderRule(w, opt, PseudoElement_Item);
            mi.rect = subRule.contentsRect(opt->rect);
            rule.configurePalette(&mi.palette, QPalette::ButtonText, QPalette::Button);
            subRule.configurePalette(&mi.palette, QPalette::ButtonText, QPalette::Button);

            if (rule.hasBackground() || subRule.hasDrawable()) {
                if (rule.hasBackground() && !subRule.hasDrawable()) {
                    QBrush b;
                    if (!rule.background()->pixmap.isNull())
                        mi.palette.setBrush(QPalette::Button, QBrush(rule.background()->pixmap));
                    else
                        mi.palette.setBrush(QPalette::Button, QBrush(rule.background()->brush));
                    ParentStyle::drawControl(ce, &mi, p, w);
                }  else {
                    subRule.drawRule(p, opt->rect);   // item
                    QCommonStyle::drawControl(ce, &mi, p, w);          // text
                }
            } else {
                baseStyle()->drawControl(ce, &mi, p, w);
            }
        }
        return;

#ifndef QT_NO_COMBOBOX
    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (!rule.hasBox() && !rule.hasBorder())
                break;
            QRect editRect = subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, w);
            p->save();
            p->setClipRect(editRect);
            if (!cb->currentIcon.isNull()) {
                QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal
                                                             : QIcon::Disabled;
                QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
                QRect iconRect(editRect);
                iconRect.setWidth(cb->iconSize.width() + 4);
                iconRect = alignedRect(QApplication::layoutDirection(),
                                       Qt::AlignLeft | Qt::AlignVCenter,
                                       iconRect.size(), editRect);
                if (cb->editable)
                    p->fillRect(iconRect, opt->palette.brush(QPalette::Base));
                drawItemPixmap(p, iconRect, Qt::AlignCenter, pixmap);

                if (cb->direction == Qt::RightToLeft)
                    editRect.translate(-4 - cb->iconSize.width(), 0);
                else
                    editRect.translate(cb->iconSize.width() + 4, 0);
            }
            if (!cb->currentText.isEmpty() && !cb->editable) {
                drawItemText(p, editRect.adjusted(0, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, cb->palette,
                             cb->state & State_Enabled, cb->currentText);
            }
            p->restore();
            return;
        }
        break;
#endif // QT_NO_COMBOBOX

    case CE_SizeGrip:
        if (!rule.hasImage())
            break;
        if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(opt)) {
            p->save();
            switch (sgOpt->corner) {
            case Qt::BottomRightCorner: break;
            case Qt::BottomLeftCorner: p->rotate(90); break;
            case Qt::TopLeftCorner: p->rotate(180); break;
            case Qt::TopRightCorner: p->rotate(270); break;
            default: break;
            }
            rule.drawImage(p, opt->rect);
            p->restore();
            return;
        }
        break;

    default:
        break;
    }

    baseStyle()->drawControl(ce, opt, p, w);
}

void QStyleSheetStyle::drawItemPixmap(QPainter *p, const QRect &rect, int alignment, const
                                  QPixmap &pixmap) const
{
    baseStyle()->drawItemPixmap(p, rect, alignment, pixmap);
}

void QStyleSheetStyle::drawItemText(QPainter *painter, const QRect& rect, int alignment, const QPalette &pal,
                                bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    baseStyle()->drawItemText(painter, rect, alignment, pal, enabled, text, textRole);
}

void QStyleSheetStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                     const QWidget *w) const
{
    if (pe == PE_FrameStatusBar) {
        if (hasStyleRule(w->parentWidget())) {
            QRenderRule subRule = renderRule(w->parentWidget(), opt, PseudoElement_Item);
            if (subRule.hasDrawable()) {
                subRule.drawRule(p, opt->rect);
                return;
            }
        }
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    if (!hasStyleRule(w)) {
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;
    }

    int pseudoElement = PseudoElement_None;

    QRenderRule rule = renderRule(w, opt);

    switch (pe) {
    case PE_IndicatorArrowDown:
        pseudoElement = PseudoElement_DownArrow;
        break;

    case PE_IndicatorRadioButton:
    case PE_IndicatorCheckBox:
        pseudoElement = PseudoElement_Indicator;
        break;

    case PE_PanelButtonCommand:
        if (!rule.hasBorder()) {
            baseStyle()->drawPrimitive(pe, opt, p, w);
        } else {
            rule.drawBorder(p, opt->rect);
        }
        return;

    // how do we specify these in the css?
    case PE_FrameDefaultButton:
        return;

    case PE_Frame:
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frm = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            // Only solid background brush is supported on a LineEdit
            if (!rule.hasBorder() && (pe == PE_Frame || !rule.hasGradientBackground())) {
                if (pe == PE_Frame)
                    rule.drawBackground(p, opt->rect);
                QStyleOptionFrame frmOpt(*frm);
                rule.configurePalette(&frmOpt.palette, QPalette::Text, QPalette::Base);
                frmOpt.rect = rule.borderRect(frmOpt.rect); // apply margin
                baseStyle()->drawPrimitive(pe, &frmOpt, p, w);
            } else {
                rule.drawFrame(p, opt->rect);
            }
        }
        return;

    case PE_Widget:
        rule.drawBackground(p, opt->rect);
        return;

    case PE_FrameMenu:
    case PE_PanelMenuBar:
        if (rule.hasBorder()) {
            rule.drawBorder(p, rule.borderRect(opt->rect));
            return;
        }
        break;

    case PE_IndicatorToolBarHandle:
        if (rule.hasDrawable()) {
            rule.drawRule(p, opt->rect);
            return;
        }
        break;

    // Menu stuff that would be nice to customize
    case PE_IndicatorMenuCheckMark:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
        baseStyle()->drawPrimitive(pe, opt, p, w);
        return;

    case PE_PanelTipLabel:
        if (rule.hasDrawable()) {
            rule.drawFrame(p, opt->rect);
            return;
        }
        break;

    case PE_FrameGroupBox:
        if (rule.hasBorder()) {
            QRect cr = opt->rect;
            if (rule.hasBox()) {
                const int *m = rule.box()->margins;
                cr = opt->rect.adjusted(-m[LeftEdge], -m[TopEdge], m[RightEdge], m[BottomEdge]);
            }
            rule.drawRule(p, cr);
            return;
        }
        break;

    default:
        break;
    }

    if (pseudoElement != PseudoElement_None) {
        QRenderRule subRule = renderRule(w, opt, pseudoElement);
        if (subRule.hasDrawable()) {
            subRule.drawRule(p, opt->rect);
        } else {
            baseStyle()->drawPrimitive(pe, opt, p, w);
        }
    } else {
        baseStyle()->drawPrimitive(pe, opt, p, w);
    }
}

QPixmap QStyleSheetStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap,
                                          const QStyleOption *option) const
{
    return baseStyle()->generatedIconPixmap(iconMode, pixmap, option);
}

QStyle::SubControl QStyleSheetStyle::hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                 const QPoint &pt, const QWidget *w) const
{
    switch (cc) {
    case CC_GroupBox:
    case CC_ComboBox:
    case CC_Slider:
        return ParentStyle::hitTestComplexControl(cc, opt, pt, w);
    default:
        break;
    }

    return baseStyle()->hitTestComplexControl(cc, opt, pt, w);
}

QRect QStyleSheetStyle::itemPixmapRect(const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    return baseStyle()->itemPixmapRect(rect, alignment, pixmap);
}

QRect QStyleSheetStyle::itemTextRect(const QFontMetrics &metrics, const QRect& rect, int alignment,
                                 bool enabled, const QString& text) const
{
    return baseStyle()->itemTextRect(metrics, rect, alignment, enabled, text);
}

int QStyleSheetStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->pixelMetric(m, opt, w);

    QRenderRule rule = renderRule(w, opt);
    QRenderRule subRule;

    switch (m) {
    case PM_MenuButtonIndicator:
        subRule = renderRule(w, opt, PseudoElement_MenuIndicator);
        if (subRule.hasContentsSize())
            return subRule.contentsSize().width();
        break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_ButtonMargin:
        // do it with the padding, if you want a shift
        if (rule.hasBox() || rule.hasDrawable())
            return 0;

    case PM_DefaultFrameWidth:
    case PM_ButtonDefaultIndicator:
        break;

    case PM_ExclusiveIndicatorWidth:
    case PM_IndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
    case PM_IndicatorHeight:
        subRule = renderRule(w, PseudoElement_Indicator);
        if (subRule.hasContentsSize()) {
            return (m == PM_ExclusiveIndicatorWidth) || (m == PM_IndicatorWidth)
                        ? subRule.contentsSize().width()
                        : subRule.contentsSize().height();
        }
        break;

    case PM_ToolTipLabelFrameWidth: // border + margin + padding (support only one width)
    case PM_ToolBarFrameWidth:
        if (rule.hasBorder() || rule.hasBox())
            return (rule.border() ? rule.border()->borders[LeftEdge] : 0)
                   + (rule.hasBox() ? rule.box()->margins[LeftEdge] + rule.box()->paddings[LeftEdge]: 0);
        break;

    case PM_MenuPanelWidth:  // border + margin + padding (support only one width)
    case PM_MenuBarPanelWidth:
        if (rule.hasBorder() || rule.hasBox())
            return (rule.border() ? rule.border()->borders[LeftEdge] : 0)
                   + (rule.hasBox() ? rule.box()->margins[LeftEdge]: 0);
        break;


    case PM_MenuHMargin:
    case PM_MenuBarHMargin:
        if (rule.hasBox())
            return rule.box()->paddings[LeftEdge];
        break;

    case PM_ToolBarItemMargin:
    case PM_MenuVMargin:
    case PM_MenuBarVMargin:
        if (rule.hasBox())
            return rule.box()->paddings[TopEdge];
        break;

    case PM_ToolBarItemSpacing:
    case PM_MenuBarItemSpacing:
        if (rule.hasBox() && rule.box()->spacing != -1)
            return rule.box()->spacing;
        break;

    case PM_SmallIconSize:
    case PM_MenuDesktopFrameWidth:
    case PM_MenuTearoffHeight:
    case PM_MenuScrollerHeight:
        break;
    case PM_ToolBarExtensionExtent:
        break;

    case PM_ToolBarHandleExtent:
        if (rule.hasBox() || rule.hasBorder() || rule.hasContentsSize())
            return rule.boxSize(rule.contentsSize()).expandedTo(rule.minimumContentsSize()).width();

    case PM_SplitterWidth:
        if (rule.hasContentsSize())
            return rule.contentsSize().width();
        break;

    case PM_CheckBoxLabelSpacing: // group box
        subRule = renderRule(w, PseudoElement_GroupBoxTitle);
        if (subRule.hasBox() && subRule.box()->spacing != -1)
            return subRule.box()->spacing;
        break;

    default:
        break;
    }

    return baseStyle()->pixelMetric(m, opt, w);
}

QSize QStyleSheetStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                         const QSize &csz, const QWidget *w) const
{
    if (!hasStyleRule(w)) {
#ifndef QT_NO_SPINBOX
        // don't touch the size of a embedded lineedit in a styled spin box
        if (ct == CT_LineEdit && w && qobject_cast<QAbstractSpinBox *>(w->parentWidget())
            && hasStyleRule(w->parentWidget())) {
            QRenderRule rule = renderRule(w->parentWidget(), opt);
            if (rule.hasBox() || rule.hasBorder())
                return csz;
        }
#endif
        return baseStyle()->sizeFromContents(ct, opt, csz, w);
    }

    QRenderRule rule = renderRule(w, opt);
    QSize sz = csz.expandedTo(rule.minimumContentsSize());

    switch (ct) {
    case CT_PushButton:
        if (rule.hasBorder() || rule.hasBox())
            return rule.boxSize(sz);
        break;

    case CT_LineEdit: // does not contains fw
        if (rule.hasBorder() || rule.hasGradientBackground()) {
            return rule.boxSize(sz);
        } else {
            // What follows is a really ugly hack to support padding with native frames
            QSize baseSize = baseStyle()->sizeFromContents(ct, opt, sz, w);
            if (!rule.hasBox())
                return baseSize;
            QSize parentSize = ParentStyle::sizeFromContents(ct, opt, sz, w);
            if (parentSize != baseSize)
                return baseSize;
            return rule.boxSize(baseSize);
        }
        break;

    case CT_CheckBox:
    case CT_RadioButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, PseudoElement_Indicator)) {
                bool isRadio = (ct == CT_RadioButton);
                int iw = pixelMetric(isRadio ? PM_ExclusiveIndicatorWidth
                                             : PM_IndicatorWidth, btn, w);
                int ih = pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight
                                             : PM_IndicatorHeight, btn, w);

                QSize margins(0, 0);
                if (!rule.hasBox() && !rule.hasBorder())
                    margins = QSize((!btn->icon.isNull() && btn->text.isEmpty()) ? 0 : 10, 4);
                int spacing = rule.hasBox() ? rule.box()->spacing : -1;
                if (spacing == -1)
                    spacing = 6;
                sz = sz + QSize(iw + spacing, 0) + margins;
                sz.setHeight(qMax(sz.height(), ih));
                return rule.boxSize(sz);
            }
        }
        break;

    case CT_GroupBox:
        if (rule.hasDrawable() || rule.hasBox() || hasStyleRule(w, PseudoElement_GroupBoxTitle)
            || hasStyleRule(w, PseudoElement_GroupBoxIndicator))
            return rule.boxSize(sz);
        break;

    case CT_Menu:
    case CT_MenuBar: // already has everything!
        if (rule.hasBox() || rule.hasBorder())
            return sz;
        break;

    case CT_MenuItem:
    case CT_MenuBarItem:
        if (hasStyleRule(w, PseudoElement_Item)) {
            QRenderRule subRule = renderRule(w, opt, PseudoElement_Item);
            if (subRule.hasBox() || subRule.hasBorder())
                return subRule.boxSize(sz);
        }
        break;

    case CT_ComboBox:
        if (rule.hasBox() || rule.hasBorder())
            return rule.boxSize(sz);
        break;

    case CT_SpinBox:
        if (rule.hasBox() || rule.hasBorder())
            return sz;

    case CT_SizeGrip:
        if (rule.hasContentsSize())
            return rule.contentsSize();
        break;

    case CT_Splitter:
    case CT_ProgressBar:
        if (rule.hasBorder() || rule.hasBox())
            return rule.boxSize(sz);
        break;

    default:
        break;
    }

    return baseStyle()->sizeFromContents(ct, opt, sz, w);
}

/*!
    \internal
*/
QIcon QStyleSheetStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption *opt,
                                                   const QWidget *widget) const
{
    return QCommonStyle::standardIconImplementation(standardIcon, opt, widget);
}

QIcon QStyleSheetStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option,
                                 const QWidget* w) const
{
    return baseStyle()->standardIcon(standardIcon, option, w);
}

QPalette QStyleSheetStyle::standardPalette() const
{
    return baseStyle()->standardPalette();
}

QPixmap QStyleSheetStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
                                     const QWidget *w) const
{
    return baseStyle()->standardPixmap(standardPixmap, option, w);
}

int QStyleSheetStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w,
                           QStyleHintReturn *shret) const
{
    if (!hasStyleRule(w))
        return baseStyle()->styleHint(sh, opt, w, shret);

    QRenderRule rule = renderRule(w, opt);
    QString s;
    switch (sh) {
        case SH_LineEdit_PasswordCharacter: s = "lineedit-password-character"; break;
        case SH_DitherDisabledText: s = "dither-disabled-text"; break;
        case SH_EtchDisabledText: s = "etch-disabled-text"; break;
        case SH_ItemView_ActivateItemOnSingleClick: s = "activate-on-singleclick"; break;
        case SH_ItemView_ShowDecorationSelected: s = "show-decoration-selected"; break;
        case SH_Table_GridLineColor: s = "gridline-color"; break;
        case SH_DialogButtonLayout: s = "button-layout"; break;
        case SH_ToolTipLabel_Opacity: s = "opacity"; break;
        case SH_ComboBox_Popup: s = "combobox-popup"; break;
        case SH_ComboBox_ListMouseTracking: s = "combobox-list-mousetracking"; break;
        case SH_MenuBar_AltKeyNavigation: s = "menubar-altkey-navigation"; break;
        case SH_DrawMenuBarSeparator: s = "menubar-separator"; break;
        case SH_MenuBar_MouseTracking: s = "mouse-tracking"; break;
        case SH_SpinBox_ClickAutoRepeatRate: s = "spinbox-click-autorepeat-rate"; break;
        case SH_SpinControls_DisableOnBounds: s = "spincontrol-disable-on-bounds"; break;
        case SH_MessageBox_TextInteractionFlags: s = "messagebox-text-interaction-flags"; break;
        case SH_GroupBox_TextLabelColor:
            if (rule.hasPalette() && rule.palette()->foreground.isValid())
                return rule.palette()->foreground.rgba();
        default: break;
    }
    if (!s.isEmpty() && rule.hasStyleHint(s)) {
        return rule.styleHint(s);
    }

    return baseStyle()->styleHint(sh, opt, w, shret);
}

QRect QStyleSheetStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                              const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->subControlRect(cc, opt, sc, w);

    QRenderRule rule = renderRule(w, opt);
    switch (cc) {
    case CC_ComboBox:
        if (rule.hasBorder() || rule.hasBox() || hasStyleRule(w, PseudoElement_DropDown)) {
            switch (sc) {
            case SC_ComboBoxArrow: {
                QRenderRule subRule = renderRule(w, PseudoElement_DropDown);
                return positionRect(rule, subRule, PseudoElement_DropDown, opt->rect, opt->direction);
                break;
                                   }
            case SC_ComboBoxEditField:
                return rule.contentsRect(opt->rect);
            case SC_ComboBoxFrame:
                return rule.contentsRect(opt->rect);
            case SC_ComboBoxListBoxPopup:
            default:
               return opt->rect;
            }
        }
        break;

    case CC_SpinBox:
        if (rule.hasBorder() || rule.hasBox() || hasStyleRule(w, PseudoElement_SpinBoxUpButton)
            || hasStyleRule(w, PseudoElement_SpinBoxDownButton)) {
            switch (sc) {
            case SC_SpinBoxUp:
            case SC_SpinBoxDown: {
                int pe = (sc == SC_SpinBoxUp ? PseudoElement_SpinBoxUpButton : PseudoElement_SpinBoxDownButton);
                QRenderRule subRule = renderRule(w, opt, pe);
                return positionRect(rule, subRule, pe, opt->rect, opt->direction);
            }
            case SC_SpinBoxEditField:
            case SC_SpinBoxFrame:
                return rule.contentsRect(opt->rect);
            default:
                return opt->rect;
            }
        }
        break;

    case CC_GroupBox:
        if (rule.hasDrawable() || rule.hasBox() || hasStyleRule(w, PseudoElement_GroupBoxTitle)
            || hasStyleRule(w, PseudoElement_GroupBoxIndicator)) {
            if (const QStyleOptionGroupBox *gb = qstyleoption_cast<const QStyleOptionGroupBox *>(opt)) {
                switch (sc) {
                case SC_GroupBoxFrame:
                    return rule.borderRect(opt->rect);
                case SC_GroupBoxContents:
                    return rule.contentsRect(opt->rect);
                default:
                case SC_GroupBoxLabel:
                case SC_GroupBoxCheckBox: {
                    QRenderRule labelRule = renderRule(w, opt, PseudoElement_GroupBoxTitle);
                    int tw = opt->fontMetrics.width(gb->text);
                    int th = opt->fontMetrics.height();
                    int spacing = pixelMetric(QStyle::PM_CheckBoxLabelSpacing, opt, w);
                    int iw = pixelMetric(QStyle::PM_IndicatorWidth, opt, w);
                    int ih = pixelMetric(QStyle::PM_IndicatorHeight, opt, w);

                    if (gb->subControls & QStyle::SC_GroupBoxCheckBox) {
                        tw = tw + iw + spacing;
                        th = qMax(th, ih);
                    }
                    if (!labelRule.hasGeometry()) {
                        labelRule.geo = new QStyleSheetGeometryData(tw, th, tw, th);
                    } else {
                        labelRule.geo->width = tw;
                        labelRule.geo->height = th;
                    }
                    QRect r = positionRect(rule, labelRule, PseudoElement_GroupBoxTitle,
                                          opt->rect, opt->direction);
                    if (gb->subControls & SC_GroupBoxCheckBox) {
                        r = labelRule.contentsRect(r);
                        if (sc == SC_GroupBoxLabel) {
                            r.setLeft(r.left() + iw + spacing);
                            r.setTop(r.center().y() - th/2);
                        } else {
                            r = QRect(r.left(), r.center().y() - ih/2, iw, ih);
                        }
                        return r;
                    } else {
                        return labelRule.contentsRect(r);
                    }
                }
                }
            }
        }
        break;

    default:
        break;
    }

    return baseStyle()->subControlRect(cc, opt, sc, w);
}

QRect QStyleSheetStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *w) const
{
    if (!hasStyleRule(w))
        return baseStyle()->subElementRect(se, opt, w);

    QRenderRule rule = renderRule(w, opt);
    QRenderRule subRule;

    switch (se) {
    // PushButton
    case SE_PushButtonContents:
    case SE_PushButtonFocusRect:
        if (rule.hasBorder() || rule.hasBox())
            return visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
        break;

    case SE_LineEditContents:
    case SE_FrameContents:
        if (rule.hasBorder() || (se != SE_FrameContents && rule.hasGradientBackground())) {
            return visualRect(opt->direction, opt->rect, rule.contentsRect(opt->rect));
        } else {
            // ugly hack to support native frames with padding
            QRect baseRect = baseStyle()->subElementRect(se, opt, w);
            if (!rule.hasBox())
                return baseRect;
            QRect parentRect = ParentStyle::subElementRect(se, opt, w);
            if (baseRect != parentRect)
                return baseRect;
            return visualRect(opt->direction, opt->rect, rule.contentsRect(baseRect));
        }
        break;

    case SE_CheckBoxIndicator:
    case SE_RadioButtonIndicator:
        if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, PseudoElement_Indicator)) {
            QRenderRule subRule = renderRule(w, opt, PseudoElement_Indicator);
            return positionRect(rule, subRule, PseudoElement_Indicator, opt->rect, opt->direction);
        }
        break;

    case SE_CheckBoxContents:
    case SE_RadioButtonContents:
        if (rule.hasBox() || rule.hasBorder() || hasStyleRule(w, PseudoElement_Indicator)) {
            bool isRadio = se == SE_RadioButtonContents;
            QRect ir = subElementRect(isRadio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator,
                                      opt, w);
            ir = visualRect(opt->direction, opt->rect, ir);
            int spacing = rule.hasBox() ? rule.box()->spacing : -1;
            if (spacing == -1)
                spacing = 6;
            QRect cr = rule.contentsRect(opt->rect);
            ir.setRect(ir.left() + ir.width() + spacing, cr.y(),
                       cr.width() - ir.width() - spacing, cr.height());
            return visualRect(opt->direction, opt->rect, ir);
        }
        break;

    case SE_RadioButtonFocusRect:
    case SE_RadioButtonClickRect: // focusrect | indicator
    case SE_CheckBoxFocusRect:
    case SE_CheckBoxClickRect: // relies on indicator and contents
        return ParentStyle::subElementRect(se, opt, w);

    default:
        break;
    }

    return baseStyle()->subElementRect(se, opt, w);
}

#include "moc_qstylesheetstyle_p.cpp"

#endif // QT_NO_STYLE_STYLESHEET
