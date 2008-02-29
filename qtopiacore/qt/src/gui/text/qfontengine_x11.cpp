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

#include "qbitmap.h"

// #define FONTENGINE_DEBUG

#include <qbytearray.h>
#include <qdebug.h>
#include <qtextcodec.h>

#include "qfontdatabase.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qvarlengtharray.h"
#include "qwidget.h"
#include "qsettings.h"
#include "qfile.h"
#include "qabstractfileengine.h"

#include <private/qpaintengine_x11_p.h>
#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qopentype_p.h"
#include <qhash.h>

#include <private/qpainter_p.h>
#include <private/qunicodetables_p.h>
#include <private/qpdf_p.h>

#include <private/qt_x11_p.h>
#include "qx11info_x11.h"

#include <math.h>
#include <limits.h>

#ifndef QT_NO_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPE1_TABLES_H
#include FT_GLYPH_H

/*
 * Freetype 2.1.7 and earlier used width/height
 * for matching sizes in the BDF and PCF loaders.
 * This has been fixed for 2.1.8.
 */
#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
#define X_SIZE(face,i) ((face)->available_sizes[i].x_ppem)
#define Y_SIZE(face,i) ((face)->available_sizes[i].y_ppem)
#else
#define X_SIZE(face,i) ((face)->available_sizes[i].width << 6)
#define Y_SIZE(face,i) ((face)->available_sizes[i].height << 6)
#endif

#define FLOOR(x)    ((x) & -64)
#define CEIL(x)	    (((x)+63) & -64)
#define TRUNC(x)    ((x) >> 6)
#define ROUND(x)    (((x)+32) & -64)


// -------------------------- Freetype support ------------------------------

struct QFreetypeFace
{
    void computeSize(const QFontDef &fontDef, int *xsize, int *ysize, bool *outline_drawing);
    QFontEngine::Properties properties() const;
    QByteArray getSfntTable(uint tag) const;

    static QFreetypeFace *getFace(const QFontEngine::FaceId &face_id);
    void release(const QFontEngine::FaceId &face_id);

    void lock() {
        Q_ASSERT(_lock == 0);
        while (!_lock.testAndSet(0, 1))
            usleep(100);
    }
    void unlock() {
        if (!_lock.testAndSet(1, 0))
            Q_ASSERT(false);
    }

    FT_Face face;
#ifndef QT_NO_FONTCONFIG
    FcCharSet *charset;
#endif
    int xsize; // 26.6
    int ysize; // 26.6
    FT_Matrix matrix;
    FT_CharMap unicode_map;
    FT_CharMap symbol_map;

    enum { cmapCacheSize = 0x200 };
    glyph_t cmapCache[cmapCacheSize];

    int fsType() const;

private:
    QFreetypeFace() {}
    ~QFreetypeFace() {}
    QAtomic ref;
    QAtomic _lock;
    QByteArray fontData;
};

static FT_Library library = 0;
FT_Library qt_getFreetype()
{
    if (!library)
        FT_Init_FreeType(&library);
    return library;
}
static QHash<QFontEngine::FaceId, QFreetypeFace *> *freetypeFaces = 0;

int QFreetypeFace::fsType() const
{
    int fsType = 0;
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
    if (os2)
        fsType = os2->fsType;
    return fsType;
}

QFreetypeFace *QFreetypeFace::getFace(const QFontEngine::FaceId &face_id)
{
    if (face_id.filename.isEmpty())
        return 0;

    if (!library)
        FT_Init_FreeType(&library);
    if (!freetypeFaces)
        freetypeFaces = new QHash<QFontEngine::FaceId, QFreetypeFace *>();

    QFreetypeFace *freetype = freetypeFaces->value(face_id, 0);
    if (!freetype) {
        freetype = new QFreetypeFace;
        FT_Face face;
        QFile file(QString::fromUtf8(face_id.filename));
        if (face_id.filename.startsWith(":qmemoryfonts/")) {
            // from qfontdatabase_x11.cpp
            extern QByteArray qt_fontdata_from_index(int);
            QByteArray idx = face_id.filename;
            idx.remove(0, 14); // remove ':qmemoryfonts/'
            bool ok = false;
            freetype->fontData = qt_fontdata_from_index(idx.toInt(&ok));
            if (!ok)
                freetype->fontData = QByteArray();
        } else if (!(file.fileEngine()->fileFlags(QAbstractFileEngine::FlagsMask) & QAbstractFileEngine::LocalDiskFlag)) {
            if (!file.open(QIODevice::ReadOnly)) {
                delete freetype;
                return 0;
            }
            freetype->fontData = file.readAll();
        }
        if (!freetype->fontData.isEmpty()) {
            if (FT_New_Memory_Face(library, (const FT_Byte *)freetype->fontData.constData(), freetype->fontData.size(), face_id.index, &face)) {
                delete freetype;
                return 0;
            }
        } else if (FT_New_Face(library, face_id.filename, face_id.index, &face)) {
            delete freetype;
            return 0;
        }
        freetype->face = face;
        freetype->ref = 0;
        freetype->_lock = 0;
        freetype->xsize = 0;
        freetype->ysize = 0;
        freetype->matrix.xx = 0x10000;
        freetype->matrix.yy = 0x10000;
        freetype->matrix.xy = 0;
        freetype->matrix.yx = 0;
        freetype->unicode_map = 0;
        freetype->symbol_map = 0;
#ifndef QT_NO_FONTCONFIG
        freetype->charset = 0;
#endif

        memset(freetype->cmapCache, 0, sizeof(freetype->cmapCache));

        for (int i = 0; i < freetype->face->num_charmaps; ++i) {
            FT_CharMap cm = freetype->face->charmaps[i];
            switch(cm->encoding) {
            case ft_encoding_unicode:
                freetype->unicode_map = cm;
                break;
            case ft_encoding_apple_roman:
            case ft_encoding_latin_1:
                if (!freetype->unicode_map || freetype->unicode_map->encoding != ft_encoding_unicode)
                    freetype->unicode_map = cm;
                break;
            case ft_encoding_adobe_custom:
            case ft_encoding_symbol:
                if (!freetype->symbol_map)
                    freetype->symbol_map = cm;
                break;
            default:
                break;
            }
        }

        if (!FT_IS_SCALABLE(freetype->face) && freetype->face->num_fixed_sizes == 1)
            FT_Set_Char_Size (face, X_SIZE(freetype->face, 0), Y_SIZE(freetype->face, 0), 0, 0);
# if 0
        FcChar8 *name;
        FcPatternGetString(pattern, FC_FAMILY, 0, &name);
        qDebug("%s: using maps: default: %x unicode: %x, symbol: %x", name,
               freetype->face->charmap ? freetype->face->charmap->encoding : 0,
               freetype->unicode_map ? freetype->unicode_map->encoding : 0,
               freetype->symbol_map ? freetype->symbol_map->encoding : 0);

        for (int i = 0; i < 256; i += 8)
            qDebug("    %x: %d %d %d %d %d %d %d %d", i,
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i),
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i),
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i),
                   FcCharSetHasChar(freetype->charset, i), FcCharSetHasChar(freetype->charset, i));
#endif

        FT_Set_Charmap(freetype->face, freetype->unicode_map);
        freetypeFaces->insert(face_id, freetype);
    }
    freetype->ref.ref();
    return freetype;
}

void QFreetypeFace::release(const QFontEngine::FaceId &face_id)
{
    if (!ref.deref()) {
        FT_Done_Face(face);
#ifndef QT_NO_FONTCONFIG
        if (charset)
            FcCharSetDestroy(charset);
#endif
        freetypeFaces->take(face_id);
        delete this;
    }
    if (!freetypeFaces->size()) {
        delete freetypeFaces;
        freetypeFaces = 0;
        FT_Done_FreeType(library);
        library = 0;
    }
}


void QFreetypeFace::computeSize(const QFontDef &fontDef, int *xsize, int *ysize, bool *outline_drawing)
{
    *ysize = fontDef.pixelSize << 6;
    *xsize = *ysize * fontDef.stretch / 100;
    *outline_drawing = false;

    /*
     * Bitmap only faces must match exactly, so find the closest
     * one (height dominant search)
     */
    if (!(face->face_flags & FT_FACE_FLAG_SCALABLE)) {
        int best = 0;
        for (int i = 1; i < face->num_fixed_sizes; i++) {
            if (qAbs(*ysize -  Y_SIZE(face,i)) <
                qAbs (*ysize - Y_SIZE(face, best)) ||
                (qAbs (*ysize - Y_SIZE(face, i)) ==
                 qAbs (*ysize - Y_SIZE(face, best)) &&
                 qAbs (*xsize - X_SIZE(face, i)) <
                 qAbs (*xsize - X_SIZE(face, best)))) {
                best = i;
            }
        }
        if (FT_Set_Char_Size (face, X_SIZE(face, best), Y_SIZE(face, best), 0, 0) == 0) {
            *xsize = X_SIZE(face, best);
            *ysize = Y_SIZE(face, best);
        } else
            *xsize = *ysize = 0;
    } else {
        *outline_drawing = (*xsize > (64<<6) || *ysize > (64<<6));
    }
}

QFontEngine::Properties QFreetypeFace::properties() const
{
    QFontEngine::Properties p;
    p.postscriptName = FT_Get_Postscript_Name(face);
    PS_FontInfoRec font_info;
    if (FT_Get_PS_Font_Info(face, &font_info) == 0)
        p.copyright = font_info.notice;
    if (FT_IS_SCALABLE(face)) {
        p.ascent = face->ascender;
        p.descent = -face->descender;
        p.leading = face->height - face->ascender + face->descender;
        p.emSquare = face->units_per_EM;
        p.boundingBox = QRectF(face->bbox.xMin, -face->bbox.yMax,
                               face->bbox.xMax - face->bbox.xMin,
                               face->bbox.yMax - face->bbox.yMin);
    } else {
        p.ascent = QFixed::fromFixed(face->size->metrics.ascender);
        p.descent = QFixed::fromFixed(-face->size->metrics.descender);
        p.leading = QFixed::fromFixed(face->size->metrics.height - face->size->metrics.ascender + face->size->metrics.descender);
        p.emSquare = face->size->metrics.y_ppem;
        p.boundingBox = QRectF(-p.ascent.toReal(), 0, (p.ascent + p.descent).toReal(), face->size->metrics.max_advance/64.);
    }
    p.italicAngle = 0;
    p.capHeight = p.ascent;
    p.lineWidth = face->underline_thickness;
    return p;
}

QByteArray QFreetypeFace::getSfntTable(uint tag) const
{
    QByteArray table;
#if (FREETYPE_MAJOR*10000 + FREETYPE_MINOR*100 + FREETYPE_PATCH) > 20103
    if (FT_IS_SFNT(face)) {
        FT_ULong length = 0;
        FT_Load_Sfnt_Table(face, tag, 0, 0, &length);
        if (length != 0) {
            table.resize(length);
            FT_Load_Sfnt_Table(face, tag, 0, (FT_Byte *)table.data(), &length);
        }
    }
#endif
    return table;
}

static void addGlyphToPath(FT_GlyphSlot g, const QFixedPoint &point, QPainterPath *path, bool no_scale = false)
{
    qreal factor = no_scale ? 1. : 1./64.;

    QPointF cp = point.toPointF();

    // convert the outline to a painter path
    int i = 0;
    for (int j = 0; j < g->outline.n_contours; ++j) {
        int last_point = g->outline.contours[j];
        QPointF start = cp + QPointF(g->outline.points[i].x*factor, -g->outline.points[i].y*factor);
        if(!(g->outline.tags[i] & 1)) {
            start += cp + QPointF(g->outline.points[last_point].x*factor, -g->outline.points[last_point].y*factor);
            start /= 2;
        }
//                 qDebug("contour: %d -- %d", i, g->outline.contours[c]);
//                 qDebug("first point at %f %f", start.x(), start.y());
        path->moveTo(start);

        QPointF c[4];
        c[0] = start;
        int n = 1;
        while (i < last_point) {
            ++i;
            c[n] = cp + QPointF(g->outline.points[i].x*factor, -g->outline.points[i].y*factor);
//                     qDebug() << "    i=" << i << " flag=" << (int)g->outline.tags[i] << "point=" << c[n];
            ++n;
            switch (g->outline.tags[i] & 3) {
            case 2:
                // cubic bezier element
                if (n < 4)
                    continue;
                c[3] = (c[3] + c[2])/2;
                --i;
                break;
            case 0:
                // quadratic bezier element
                if (n < 3)
                    continue;
                c[3] = (c[1] + c[2])/2;
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
                --i;
                break;
            case 1:
            case 3:
                if (n == 2) {
//                             qDebug() << "lineTo" << c[1];
                    path->lineTo(c[1]);
                    c[0] = c[1];
                    n = 1;
                    continue;
                } else if (n == 3) {
                    c[3] = c[2];
                    c[2] = (2*c[1] + c[3])/3;
                    c[1] = (2*c[1] + c[0])/3;
                }
                break;
            }
//                     qDebug() << "cubicTo" << c[1] << c[2] << c[3];
            path->cubicTo(c[1], c[2], c[3]);
            c[0] = c[3];
            n = 1;
        }
        if (n == 1) {
//                     qDebug() << "closeSubpath";
            path->closeSubpath();
        } else {
            c[3] = start;
            if (n == 2) {
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
            }
//                     qDebug() << "cubicTo" << c[1] << c[2] << c[3];
            path->cubicTo(c[1], c[2], c[3]);
        }
        ++i;
    }
}

extern void qt_addBitmapToPath(qreal x0, qreal y0, const uchar *image_data, int bpl, int w, int h, QPainterPath *path);

static void addBitmapToPath(FT_GlyphSlot slot, const QFixedPoint &point, QPainterPath *path, bool = false)
{
    if (slot->format != FT_GLYPH_FORMAT_BITMAP
        || slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO)
        return;

    QPointF cp = point.toPointF();
    qt_addBitmapToPath(cp.x() + TRUNC(slot->metrics.horiBearingX), cp.y() - TRUNC(slot->metrics.horiBearingY),
                       slot->bitmap.buffer, slot->bitmap.pitch, slot->bitmap.width, slot->bitmap.rows, path);
}

#endif // QT_NO_FREETYPE


// ------------------------------------------------------------------
// Multi XLFD engine
// ------------------------------------------------------------------

QFontEngineMultiXLFD::QFontEngineMultiXLFD(const QFontDef &r, const QList<int> &l, int s)
    : QFontEngineMulti(l.size()), encodings(l), screen(s), request(r)
{
    loadEngine(0);
    fontDef = engines[0]->fontDef;
}

QFontEngineMultiXLFD::~QFontEngineMultiXLFD()
{ }

void QFontEngineMultiXLFD::loadEngine(int at)
{
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);
    const int encoding = encodings.at(at);
    QFontEngine *fontEngine = QFontDatabase::loadXlfd(0, QUnicodeTables::Common, request, encoding);
    Q_ASSERT(fontEngine != 0);
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}

// ------------------------------------------------------------------
// Xlfd font engine
// ------------------------------------------------------------------

#ifndef QT_NO_FREETYPE

static QStringList *qt_fontpath = 0;

static QStringList fontPath()
{
    if (qt_fontpath)
        return *qt_fontpath;

    // append qsettings fontpath
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
    settings.beginGroup(QLatin1String("Qt"));

    QStringList fontpath;

    int npaths;
    char** font_path;
    font_path = XGetFontPath(X11->display, &npaths);
    bool xfsconfig_read = false;
    for (int i=0; i<npaths; i++) {
        // If we're using xfs, append font paths from /etc/X11/fs/config
        // can't hurt, and chances are we'll get all fonts that way.
        if (((font_path[i])[0] != '/') && !xfsconfig_read) {
            // We're using xfs -> read its config
            bool finished = false;
            QFile f(QLatin1String("/etc/X11/fs/config"));
            if (!f.exists())
                f.setFileName(QLatin1String("/usr/X11R6/lib/X11/fs/config"));
            if (!f.exists())
                f.setFileName(QLatin1String("/usr/X11/lib/X11/fs/config"));
            if (f.exists()) {
                f.open(QIODevice::ReadOnly);
                while (f.error()==QFile::NoError && !finished) {
                    QString fs = QString::fromLocal8Bit(f.readLine(1024));
                    fs=fs.trimmed();
                    if (fs.left(9)==QLatin1String("catalogue") && fs.contains(QLatin1Char('='))) {
                        fs = fs.mid(fs.indexOf(QLatin1Char('=')) + 1).trimmed();
                        bool end = false;
                        while (f.error()==QFile::NoError && !end) {
                            if (fs[int(fs.length())-1] == QLatin1Char(','))
                                fs = fs.left(fs.length()-1);
                            else
                                end = true;

                            fs = fs.left(fs.indexOf(QLatin1String(":unscaled")));
                            if (fs[0] != QLatin1Char('#'))
                                fontpath += fs;
                            fs = QLatin1String(f.readLine(1024));
                            fs = fs.trimmed();
                            if (fs.isEmpty())
                                end = true;
                        }
                        finished = true;
                    }
                }
                f.close();
            }
            xfsconfig_read = true;
        } else {
            QString fs = QString::fromLocal8Bit(font_path[i]);
            fontpath += fs.left(fs.indexOf(QLatin1String(":unscaled")));
        }
    }
    XFreeFontPath(font_path);

    // append qsettings fontpath
    QStringList fp = settings.value(QLatin1String("fontPath")).toStringList();
    if (!fp.isEmpty())
        fontpath += fp;

    qt_fontpath = new QStringList(fontpath);
    return fontpath;
}

static QFontEngine::FaceId fontFile(const QByteArray &_xname, QFreetypeFace **freetype, int *synth)
{
    *freetype = 0;
    *synth = 0;

    QByteArray xname = _xname.toLower();

    int pos = 0;
    int minus = 0;
    while (minus < 5 && (pos = xname.indexOf('-', pos + 1)))
        ++minus;
    QByteArray searchname = xname.left(pos);
    while (minus < 12 && (pos = xname.indexOf('-', pos + 1)))
        ++minus;
    QByteArray encoding = xname.mid(pos + 1);
    //qDebug("xname='%s', searchname='%s', encoding='%s'", xname.data(), searchname.data(), encoding.data());
    QStringList fontpath = ::fontPath();
    QFontEngine::FaceId face_id;
    face_id.index = 0;

    QByteArray best_mapping;

    for (QStringList::ConstIterator it = fontpath.constBegin(); it != fontpath.constEnd(); ++it) {
        if ((*it).left(1) != QLatin1String("/"))
            continue; // not a path name, a font server
        QString fontmapname;
        int num = 0;
        // search font.dir and font.scale for the right file
        while (num < 2) {
            if (num == 0)
                fontmapname = (*it) + QLatin1String("/fonts.scale");
            else
                fontmapname = (*it) + QLatin1String("/fonts.dir");
            ++num;
            //qWarning(fontmapname);
            QFile fontmap(fontmapname);
            if (!fontmap.open(QIODevice::ReadOnly))
                continue;
            while (!fontmap.atEnd()) {
                QByteArray mapping = fontmap.readLine();
                QByteArray lmapping = mapping.toLower();

                //qWarning(xfontname);
                //qWarning(mapping);
                if (!lmapping.contains(searchname))
                    continue;
                int index = mapping.indexOf(' ');
                QByteArray ffn = mapping.mid(0,index);
                // remove bitmap formats freetype can't handle
                if (ffn.contains(".spd") || ffn.contains(".phont"))
                    continue;
                bool best_match = false;
                if (!best_mapping.isEmpty()) {
                    if (lmapping.contains("-0-0-0-0-")) { // scalable font
                        best_match = true;
                        goto found;
                    }
                    if (lmapping.contains(encoding) && !best_mapping.toLower().contains(encoding))
                        goto found;
                    continue;
                }

            found:
                int colon = ffn.lastIndexOf(':');
                if (colon != -1) {
                    QByteArray s = ffn.left(colon);
                    ffn = ffn.mid(colon + 1);
                    if (s.contains("ds="))
                        *synth |= QFontEngine::SynthesizedBold;
                    if (s.contains("ai="))
                        *synth |= QFontEngine::SynthesizedItalic;
                }
                face_id.filename = (*it).toLocal8Bit() + '/' + ffn;
                best_mapping = mapping;
                if (best_match)
                    goto end;
            }
        }
    }
end:
//     qDebug("fontfile for %s is from '%s'\n    got %s synth=%d", xname.data(),
//            best_mapping.data(), face_id.filename.data(), *synth);
    *freetype = QFreetypeFace::getFace(face_id);
    if (!*freetype) {
        face_id.index = 0;
        face_id.filename = QByteArray();
    }
    return face_id;
}

#endif // QT_NO_FREETYPE

// defined in qfontdatabase_x11.cpp
extern int qt_mib_for_xlfd_encoding(const char *encoding);
extern int qt_xlfd_encoding_id(const char *encoding);

static inline XCharStruct *charStruct(XFontStruct *xfs, uint ch)
{
    XCharStruct *xcs = 0;
    unsigned char r = ch>>8;
    unsigned char c = ch&0xff;
    if (xfs->per_char &&
         r >= xfs->min_byte1 &&
         r <= xfs->max_byte1 &&
         c >= xfs->min_char_or_byte2 &&
         c <= xfs->max_char_or_byte2) {
        xcs = xfs->per_char + ((r - xfs->min_byte1) *
                               (xfs->max_char_or_byte2 -
                                xfs->min_char_or_byte2 + 1)) +
              (c - xfs->min_char_or_byte2);
        if (xcs->width == 0 && xcs->ascent == 0 &&  xcs->descent == 0)
            xcs = 0;
    }
    return xcs;
}

QFontEngineXLFD::QFontEngineXLFD(XFontStruct *fs, const QByteArray &name, int mib)
    : _fs(fs), _name(name), _codec(0), _cmap(mib)
{
    if (_cmap) _codec = QTextCodec::codecForMib(_cmap);

    cache_cost = (((fs->max_byte1 - fs->min_byte1) *
                   (fs->max_char_or_byte2 - fs->min_char_or_byte2 + 1)) +
                  fs->max_char_or_byte2 - fs->min_char_or_byte2);
    cache_cost = ((fs->max_bounds.ascent + fs->max_bounds.descent) *
                  (fs->max_bounds.width * cache_cost / 8));
    lbearing = SHRT_MIN;
    rbearing = SHRT_MIN;
    face_id.index = -1;
    freetype = 0;
    synth = 0;
}

QFontEngineXLFD::~QFontEngineXLFD()
{
    XFreeFont(QX11Info::display(), _fs);
    _fs = 0;
#ifndef QT_NO_FREETYPE
    if (freetype)
        freetype->release(face_id);
#endif
}

bool QFontEngineXLFD::stringToCMap(const QChar *s, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    // filter out surrogates, we can't handle them anyway with XLFD fonts
    QVarLengthArray<ushort> _s(len);
    QChar *str = (QChar *)_s.data();
    for (int i = 0; i < len; ++i) {
        if (i < len - 1
            && s[i].unicode() >= 0xd800 && s[i].unicode() < 0xdc00
            && s[i+1].unicode() >= 0xdc00 && s[i].unicode() < 0xe000) {
            *str = QChar();
            ++i;
        } else {
            *str = s[i];
        }
        ++str;
    }

    len = str - (QChar *)_s.data();
    str = (QChar *)_s.data();

    bool mirrored = flags & QTextEngine::RightToLeft;
    if (_codec) {
        bool haveNbsp = false;
        for (int i = 0; i < len; i++)
            if (str[i].unicode() == 0xa0) {
                haveNbsp = true;
                break;
            }

        QVarLengthArray<unsigned short> ch(len);
        QChar *chars = (QChar *)ch.data();
        if (haveNbsp || mirrored) {
            for (int i = 0; i < len; i++)
                chars[i] = (str[i].unicode() == 0xa0 ? 0x20 :
                            (mirrored ? QUnicodeTables::mirroredChar(str[i]).unicode() : str[i].unicode()));
        } else {
            for (int i = 0; i < len; i++)
                chars[i] = str[i].unicode();
        }
        QTextCodec::ConverterState state;
        state.flags = QTextCodec::ConvertInvalidToNull;
        QByteArray ba = _codec->fromUnicode(chars, len, &state);
        if (ba.length() == 2*len) {
            // double byte encoding
            const uchar *data = (const uchar *)ba.constData();
            for (int i = 0; i < len; i++) {
                glyphs[i].glyph = ((ushort)data[0] << 8) + data[1];
                data += 2;
            }
        } else {
            const uchar *data = (const uchar *)ba.constData();
            for (int i = 0; i < len; i++)
                glyphs[i].glyph = (ushort)data[i];
        }
    } else {
        QGlyphLayout *g = glyphs + len;
        const QChar *c = str + len;
        if (mirrored) {
            while (c != str)
                (--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : QUnicodeTables::mirroredChar(*c).unicode();
        } else {
            while (c != str)
                (--g)->glyph = (--c)->unicode() == 0xa0 ? 0x20 : c->unicode();
        }
    }
    *nglyphs = len;

    QGlyphLayout *g = glyphs + len;
    XCharStruct *xcs;
    // inlined for better perfomance
    if (!_fs->per_char) {
        xcs = &_fs->min_bounds;
        while (g != glyphs) {
            --g;
            const unsigned char r = g->glyph >> 8;
            const unsigned char c = g->glyph & 0xff;
            if (r >= _fs->min_byte1 &&
                r <= _fs->max_byte1 &&
                c >= _fs->min_char_or_byte2 &&
                c <= _fs->max_char_or_byte2) {
                g->advance.x = xcs->width;
            } else {
                g->glyph = 0;
            }
        }
    }
    else if (!_fs->max_byte1) {
        XCharStruct *base = _fs->per_char - _fs->min_char_or_byte2;
        while (g != glyphs) {
            unsigned int gl = (--g)->glyph;
            xcs = (gl >= _fs->min_char_or_byte2 && gl <= _fs->max_char_or_byte2) ?
                  base + gl : 0;
            if (!xcs || (!xcs->width && !xcs->ascent && !xcs->descent)) {
                g->glyph = 0;
            } else {
                g->advance.x = xcs->width;
            }
        }
    }
    else {
        while (g != glyphs) {
            xcs = charStruct(_fs, (--g)->glyph);
            if (!xcs) {
                g->glyph = 0;
            } else {
                g->advance.x = xcs->width;
            }
        }
    }
    return true;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{
    int i;

    glyph_metrics_t overall;
    // initialize with line height, we get the same behaviour on all platforms
    overall.y = -ascent();
    overall.height = ascent() + descent() + 1;
    QFixed ymax;
    QFixed xmax;
    for (i = 0; i < numGlyphs; i++) {
        XCharStruct *xcs = charStruct(_fs, glyphs[i].glyph);
        if (xcs) {
            QFixed x = overall.xoff + glyphs[i].offset.x + xcs->lbearing;
            QFixed y = overall.yoff + glyphs[i].offset.y - xcs->ascent;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, overall.xoff + glyphs[i].offset.x + xcs->rbearing);
            ymax = qMax(ymax, y + xcs->ascent + xcs->descent);
            overall.xoff += glyphs[i].advance.x;
        } else {
            QFixed size = _fs->ascent;
            overall.x = qMin(overall.x, overall.xoff);
            overall.y = qMin(overall.y, overall.yoff - size);
            ymax = qMax(ymax, overall.yoff);
            overall.xoff += size;
            xmax = qMax(xmax, overall.xoff);
        }
    }
    overall.height = qMax(overall.height, ymax - overall.y);
    overall.width = xmax - overall.x;

    return overall;
}

glyph_metrics_t QFontEngineXLFD::boundingBox(glyph_t glyph)
{
    glyph_metrics_t gm;
    XCharStruct *xcs = charStruct(_fs, glyph);
    if (xcs) {
        gm = glyph_metrics_t(xcs->lbearing, -xcs->ascent, xcs->rbearing- xcs->lbearing, xcs->ascent + xcs->descent,
                              xcs->width, 0);
    } else {
        QFixed size = ascent();
        gm = glyph_metrics_t(0, size, size, size, size, 0);
    }
    return gm;
}

QFixed QFontEngineXLFD::ascent() const
{
    return _fs->ascent;
}

QFixed QFontEngineXLFD::descent() const
{
    return (_fs->descent-1);
}

QFixed QFontEngineXLFD::leading() const
{
    QFixed l = QFixed(qMin<int>(_fs->ascent, _fs->max_bounds.ascent)
                      + qMin<int>(_fs->descent, _fs->max_bounds.descent)) * QFixed::fromReal(0.15);
    return l.ceil();
}

qreal QFontEngineXLFD::maxCharWidth() const
{
    return _fs->max_bounds.width;
}


// Loads the font for the specified script
static inline int maxIndex(XFontStruct *f) {
    return (((f->max_byte1 - f->min_byte1) *
             (f->max_char_or_byte2 - f->min_char_or_byte2 + 1)) +
            f->max_char_or_byte2 - f->min_char_or_byte2);
}

qreal QFontEngineXLFD::minLeftBearing() const
{
    if (lbearing == SHRT_MIN) {
        if (_fs->per_char) {
            XCharStruct *cs = _fs->per_char;
            int nc = maxIndex(_fs) + 1;
            int mx = cs->lbearing;

            for (int c = 1; c < nc; c++) {
                // ignore the bearings for characters whose ink is
                // completely outside the normal bounding box
                if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
                    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
                    continue;

                int nmx = cs[c].lbearing;

                if (nmx < mx)
                    mx = nmx;
            }

            ((QFontEngineXLFD *)this)->lbearing = mx;
        } else
            ((QFontEngineXLFD *)this)->lbearing = _fs->min_bounds.lbearing;
    }
    return lbearing;
}

qreal QFontEngineXLFD::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        if (_fs->per_char) {
            XCharStruct *cs = _fs->per_char;
            int nc = maxIndex(_fs) + 1;
            int mx = cs->rbearing;

            for (int c = 1; c < nc; c++) {
                // ignore the bearings for characters whose ink is
                // completely outside the normal bounding box
                if ((cs[c].lbearing <= 0 && cs[c].rbearing <= 0) ||
                    (cs[c].lbearing >= cs[c].width && cs[c].rbearing >= cs[c].width))
                    continue;

                int nmx = cs[c].rbearing;

                if (nmx < mx)
                    mx = nmx;
            }

            ((QFontEngineXLFD *)this)->rbearing = mx;
        } else
            ((QFontEngineXLFD *)this)->rbearing = _fs->min_bounds.rbearing;
    }
    return rbearing;
}

const char *QFontEngineXLFD::name() const
{
    return _name;
}

bool QFontEngineXLFD::canRender(const QChar *string, int len)
{
    QVarLengthArray<QGlyphLayout, 256> glyphs(len);
    int nglyphs = len;
    if (stringToCMap(string, len, glyphs.data(), &nglyphs, 0) == false) {
        glyphs.resize(nglyphs);
        stringToCMap(string, len, glyphs.data(), &nglyphs, 0);
    }

    bool allExist = true;
    for (int i = 0; i < nglyphs; i++) {
        if (!glyphs[i].glyph || !charStruct(_fs, glyphs[i].glyph)) {
            allExist = false;
            break;
        }
    }

    return allExist;
}

void QFontEngineXLFD::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
}

QFontEngine::FaceId QFontEngineXLFD::faceId() const
{
#ifndef QT_NO_FREETYPE
    if (face_id.index == -1) {
        face_id = ::fontFile(_name, &freetype, &synth);
        if (_codec)
            face_id.encoding = _codec->mibEnum();
        if (freetype) {
            const_cast<QFontEngineXLFD *>(this)->fsType = freetype->fsType();
        } else {
            QFontEngine::Properties properties = QFontEngine::properties();
            face_id.index = 0;
            face_id.filename = "-" + properties.postscriptName;
        }
    }
#endif

    return face_id;
}

QFontEngine::Properties QFontEngineXLFD::properties() const
{
    if (face_id.index == -1)
        (void)faceId();

#ifndef QT_NO_FREETYPE
    if (freetype)
        return freetype->properties();
#endif
    return QFontEngine::properties();
}

void QFontEngineXLFD::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    if (face_id.index == -1)
        (void)faceId();
#ifndef QT_NO_FREETYPE
    if (!freetype)
#endif
    {
        QFontEngine::getUnscaledGlyph(glyph, path, metrics);
        return;
    }

#ifndef QT_NO_FREETYPE
    freetype->lock();

    FT_Face face = freetype->face;
    FT_Set_Transform(face, 0, 0);
    glyph = glyphIndexToFreetypeGlyphIndex(glyph);
    FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE);

    int left  = face->glyph->metrics.horiBearingX;
    int right = face->glyph->metrics.horiBearingX + face->glyph->metrics.width;
    int top    = face->glyph->metrics.horiBearingY;
    int bottom = face->glyph->metrics.horiBearingY - face->glyph->metrics.height;

    QFixedPoint p;
    p.x = 0;
    p.y = 0;
    if (!FT_IS_SCALABLE(freetype->face)) {
        metrics->width = QFixed::fromFixed(right-left);
        metrics->height = QFixed::fromFixed(top-bottom);
        metrics->x = QFixed::fromFixed(left);
        metrics->y = QFixed::fromFixed(-top);
        metrics->xoff = QFixed::fromFixed(face->glyph->advance.x);

        ::addBitmapToPath(face->glyph, p, path);
    } else {
        metrics->width = right-left;
        metrics->height = top-bottom;
        metrics->x = left;
        metrics->y = -top;
        metrics->xoff = face->glyph->advance.x;

        ::addGlyphToPath(face->glyph, p, path, true /* no_scale */);
    }
    FT_Set_Transform(face, &freetype->matrix, 0);
    freetype->unlock();
#endif // QT_NO_FREETYPE
}


QByteArray QFontEngineXLFD::getSfntTable(uint tag) const
{
#ifndef QT_NO_FREETYPE
    if (face_id.index == -1)
        (void)faceId();
    if (!freetype)
        return QByteArray();
    return freetype->getSfntTable(tag);
#else
    Q_UNUSED(tag);
    return QByteArray();
#endif
}

int QFontEngineXLFD::synthesized() const
{
    return synth;
}

#ifndef QT_NO_FREETYPE

FT_Face QFontEngineXLFD::non_locked_face() const
{
    return freetype ? freetype->face : 0;
}

uint QFontEngineXLFD::toUnicode(glyph_t g) const
{
    if (_codec) {
        QTextCodec::ConverterState state;
        state.flags = QTextCodec::ConvertInvalidToNull;
        uchar data[2];
        int l = 1;
        if (g > 255) {
            data[0] = (g >> 8);
            data[1] = (g & 255);
            l = 2;
        } else {
            data[0] = g;
        }
        QString s = _codec->toUnicode((char *)data, l, &state);
        Q_ASSERT(s.length() == 1);
        g = s.at(0).unicode();
    }
    return g;
}

glyph_t QFontEngineXLFD::glyphIndexToFreetypeGlyphIndex(glyph_t g) const
{
    return FT_Get_Char_Index(freetype->face, toUnicode(g));
}
#endif

#ifndef QT_NO_FONTCONFIG

// ------------------------------------------------------------------
// Multi FT engine
// ------------------------------------------------------------------

static QFontEngine *engineForPattern(FcPattern *pattern, const QFontDef &request,
                                     int screen)
{
    FcResult res;
    FcPattern *match = FcFontMatch(0, pattern, &res);
    QFontEngineFT *engine = new QFontEngineFT(match, request, screen);
    if (!engine->invalid())
        return engine;

    delete engine;
    QFontEngine *fe = new QFontEngineBox(request.pixelSize);
    fe->fontDef = request;
    return fe;
}

QFontEngineMultiFT::QFontEngineMultiFT(QFontEngine *fe, FcPattern *p, int s, const QFontDef &req)
    : QFontEngineMulti(2), request(req), pattern(p), fontSet(0), screen(s)
{

    engines[0] = fe;
    engines.at(0)->ref.ref();
    fontDef = engines[0]->fontDef;
    cache_cost = 100;
}

QFontEngineMultiFT::~QFontEngineMultiFT()
{
    FcPatternDestroy(pattern);
    if (fontSet)
        FcFontSetDestroy(fontSet);
}


void QFontEngineMultiFT::loadEngine(int at)
{
    extern void qt_addPatternProps(FcPattern *pattern, int screen, int script,
                                   const QFontDef &request);
    extern QFontDef qt_FcPatternToQFontDef(FcPattern *pattern, const QFontDef &);
    extern FcFontSet *qt_fontSetForPattern(FcPattern *pattern, const QFontDef &request);

    Q_ASSERT(at > 0);
    if (!fontSet) {
        fontSet = qt_fontSetForPattern(pattern, request);
        engines.resize(fontSet->nfont);
    }
    Q_ASSERT(at < engines.size());
    Q_ASSERT(engines.at(at) == 0);

    FcPattern *pattern = FcPatternDuplicate(fontSet->fonts[at]);
    qt_addPatternProps(pattern, screen, QUnicodeTables::Common, request);

    QFontDef fontDef = qt_FcPatternToQFontDef(pattern, this->request);

    // note: we use -1 for the script to make sure that we keep real
    // FT engines separate from Multi engines in the font cache
    QFontCache::Key key(fontDef, -1, screen);
    QFontEngine *fontEngine = QFontCache::instance->findEngine(key);
    if (!fontEngine) {
        FcConfigSubstitute(0, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);
        fontEngine = engineForPattern(pattern, request, screen);
        FcPatternDestroy(pattern);
        QFontCache::instance->insertEngine(key, fontEngine);
    }
    fontEngine->ref.ref();
    engines[at] = fontEngine;
}

// ------------------------------------------------------------------
// FT font engine
// ------------------------------------------------------------------


QFontEngineFT::Glyph::~Glyph()
{
    delete [] data;
}

static QFontEngine::FaceId face_id(FcPattern *pattern)
{
    char *file_name;
    FcPatternGetString(pattern, FC_FILE, 0, (FcChar8 **)&file_name);
    int face_index;
    if (!FcPatternGetInteger(pattern, FC_INDEX, 0, &face_index))
        face_index = 0;
    QFontEngine::FaceId face_id;
    face_id.filename = file_name;
    face_id.index = face_index;
    return face_id;
}

QFontEngineFT::QFontEngineFT(FcPattern *pattern, const QFontDef &fd, int screen)
{
    _openType = 0;
    cache_cost = 100;
    fontDef = fd;
    _pattern = pattern;
    transform = false;
    matrix.xx = 0x10000;
    matrix.yy = 0x10000;
    matrix.xy = 0;
    matrix.yx = 0;
//     FcPatternPrint(pattern);

    antialias = X11->fc_antialias;
    FcBool b;
    if (FcPatternGetBool(pattern, FC_ANTIALIAS, 0, &b) == FcResultMatch)
        antialias = b;
    if (FcPatternGetInteger(pattern, FC_RGBA, 0, &subpixel) == FcResultNoMatch)
        subpixel = X11->screens[screen].subpixel;
    if (!antialias || subpixel == FC_RGBA_UNKNOWN)
        subpixel = FC_RGBA_NONE;

#ifdef FC_HINT_STYLE
    if (FcPatternGetInteger (pattern, FC_HINT_STYLE, 0, &hint_style) == FcResultNoMatch)
	hint_style = X11->fc_hint_style;
#endif

    autohint = false;
#ifdef FC_AUTOHINT
    if (FcPatternGetBool(pattern, FC_AUTOHINT, 0, &b) == FcResultMatch)
        autohint = b;
#endif

    face_id = ::face_id(pattern);

    freetype = QFreetypeFace::getFace(face_id);
    if (!freetype) {
        xsize = 0;
        ysize = 0;
        return;
    }

    if (!freetype->charset) {
        FcCharSet *cs;
        FcPatternGetCharSet (pattern, FC_CHARSET, 0, &cs);
        freetype->charset = FcCharSetCopy(cs);
    }
    symbol = freetype->symbol_map != 0;

    lbearing = rbearing = SHRT_MIN;
    freetype->computeSize(fontDef, &xsize, &ysize, &outline_drawing);

    FT_Face face = lockFace();

    //underline metrics
    if (FT_IS_SCALABLE(face)) {
        line_thickness =  QFixed::fromFixed(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale));
        underline_position = QFixed::fromFixed(-FT_MulFix(face->underline_position, face->size->metrics.y_scale));
        bool fake_oblique = (fontDef.style != QFont::StyleNormal) && !(face->style_flags & FT_STYLE_FLAG_ITALIC);
        if (fake_oblique)
            matrix.xy = 0x10000*3/10;
        FT_Set_Transform(face, &matrix, 0);
        if (fake_oblique)
            transform = true;
    } else {
        // copied from QFontEngineQPF
        // ad hoc algorithm
        int score = fontDef.weight * fontDef.pixelSize;
        line_thickness = score / 700;
        // looks better with thicker line for small pointsizes
        if (line_thickness < 2 && score >= 1050)
            line_thickness = 2;
        underline_position =  ((line_thickness * 2) + 3) / 6;
    }
    if (line_thickness < 1)
        line_thickness = 1;

    metrics = face->size->metrics;

    unlockFace();

    fsType = freetype->fsType();

#ifndef QT_NO_XRENDER
    if (X11->use_xrender) {
        int format = PictStandardA8;
        if (!antialias)
            format = PictStandardA1;
        else if (subpixel == FC_RGBA_RGB
                 || subpixel == FC_RGBA_BGR
                 || subpixel == FC_RGBA_VRGB
                 || subpixel == FC_RGBA_VBGR)
            format = PictStandardARGB32;
        fnt.glyphSet = XRenderCreateGlyphSet(X11->display,
                                             XRenderFindStandardFormat(X11->display, format));
        xglyph_format = format;
    } else {
        fnt.glyphSet = 0;
    }
#endif
}

QFontEngineFT::~QFontEngineFT()
{
    delete _openType;
    _openType = 0;

    if (freetype)
    freetype->release(face_id);

    FcPatternDestroy(_pattern);
    _pattern = 0;
}

FT_Face QFontEngineFT::lockFace() const
{
    freetype->lock();
    FT_Face face = freetype->face;
    if (freetype->xsize != xsize || freetype->ysize != ysize) {
        FT_Set_Char_Size(face, xsize, ysize, 0, 0);
        freetype->xsize = xsize;
        freetype->ysize = ysize;
    }
    if (freetype->matrix.xx != matrix.xx ||
        freetype->matrix.yy != matrix.yy ||
        freetype->matrix.xy != matrix.xy ||
        freetype->matrix.yx != matrix.yx) {
        freetype->matrix = matrix;
        FT_Set_Transform(face, &freetype->matrix, 0);
    }

    return face;
}

void QFontEngineFT::unlockFace() const
{
    freetype->unlock();
}

FT_Face QFontEngineFT::non_locked_face() const
{
    return freetype->face;
}

static const uint subpixel_filter[3][3] = {
    { 180, 60, 16 },
    { 38, 180, 38 },
    { 16, 60, 180 }
};

QFontEngineFT::Font::Font()
#ifndef QT_NO_XRENDER
    : glyphSet(0)
#endif
{
    transformationMatrix.xx = 0x10000;
    transformationMatrix.yy = 0x10000;
    transformationMatrix.xy = 0;
    transformationMatrix.yx = 0;
}

QFontEngineFT::Font::~Font()
{
    qDeleteAll(glyph_data);
#ifndef QT_NO_XRENDER
    if (glyphSet != 0)
        XRenderFreeGlyphSet(X11->display, glyphSet);
#endif
}

QFontEngineFT::Glyph *QFontEngineFT::Font::loadGlyph(const QFontEngineFT *fe, uint glyph, GlyphFormat format) const
{
//     Q_ASSERT(freetype->lock == 1);

    bool add_to_glyphset = false;
    if (format == Format_None) {
        format = Format_Mono;
        if (X11->use_xrender) {
            add_to_glyphset = true;
            if (fe->subpixel != FC_RGBA_NONE)
                format = Format_A32;
            else if (fe->antialias)
                format = Format_A8;
        }
    }
    Q_ASSERT(format != Format_None);
    bool hsubpixel = false;
    int vfactor = 1;
    int load_flags = FT_LOAD_DEFAULT;
    if (fe->outline_drawing) {
        load_flags = FT_LOAD_NO_BITMAP|FT_LOAD_NO_HINTING;
    } else if (format == Format_Mono) {
        load_flags |= FT_LOAD_TARGET_MONO;
    } else if (format == Format_A32) {
        if (fe->subpixel == FC_RGBA_RGB || fe->subpixel == FC_RGBA_BGR) {
            load_flags |= FT_LOAD_TARGET_LCD;
            hsubpixel = true;
        } else if (fe->subpixel == FC_RGBA_VRGB || fe->subpixel == FC_RGBA_VBGR) {
            load_flags |= FT_LOAD_TARGET_LCD_V;
            vfactor = 3;
        }

    }
    if (format != Format_Mono)
        load_flags |= FT_LOAD_NO_BITMAP;

#ifdef FC_HINT_STYLE
    if (fe->hint_style == FC_HINT_NONE)
        load_flags |= FT_LOAD_NO_HINTING;
    else if (fe->hint_style < FC_HINT_FULL)
        load_flags |= FT_LOAD_TARGET_LIGHT;
#endif

#ifdef FT_LOAD_FORCE_AUTOHINT
    if (fe->autohint)
        load_flags |= FT_LOAD_FORCE_AUTOHINT;
#endif

    bool transform = fe->transform
                     || transformationMatrix.xx != 0x10000
                     || transformationMatrix.yy != 0x10000
                     || transformationMatrix.xy != 0
                     || transformationMatrix.yx != 0;

    if (transform)
        load_flags |= FT_LOAD_NO_BITMAP;

    {
        Glyph *g = glyph_data.value(glyph);
        if (g && g->format == format)
            return g;
    }

    const QFreetypeFace * const freetype = fe->freetype;
    FT_Face face = freetype->face;
    FT_Error err = FT_Load_Glyph(face, glyph, load_flags);
    if (err && (load_flags & FT_LOAD_NO_BITMAP)) {
        load_flags &= ~FT_LOAD_NO_BITMAP;
        err = FT_Load_Glyph(face, glyph, load_flags);
    }
    if (err == FT_Err_Too_Few_Arguments) {
        // this is an error in the bytecode interpreter, just try to run without it
        load_flags |= FT_LOAD_FORCE_AUTOHINT;
        err = FT_Load_Glyph(face, glyph, load_flags);
    }
    if (err != FT_Err_Ok)
        qWarning("load glyph failed err=%x face=%p, glyph=%d", err, face, glyph);

    if (fe->outline_drawing)
        return 0;

    FT_GlyphSlot slot = face->glyph;

    FT_Matrix matrix = freetype->matrix;

    int left  = slot->metrics.horiBearingX;
    int right = slot->metrics.horiBearingX + slot->metrics.width;
    int top    = slot->metrics.horiBearingY;
    int bottom = slot->metrics.horiBearingY - slot->metrics.height;
    if(transform && slot->format != FT_GLYPH_FORMAT_BITMAP) {
        int l, r, t, b;
        FT_Vector vector;
        vector.x = left;
        vector.y = top;
        FT_Vector_Transform(&vector, &matrix);
        l = r = vector.x;
        t = b = vector.y;
        vector.x = right;
        vector.y = top;
        FT_Vector_Transform(&vector, &matrix);
        if (l > vector.x) l = vector.x;
        if (r < vector.x) r = vector.x;
        if (t < vector.y) t = vector.y;
        if (b > vector.y) b = vector.y;
        vector.x = right;
        vector.y = bottom;
        FT_Vector_Transform(&vector, &matrix);
        if (l > vector.x) l = vector.x;
        if (r < vector.x) r = vector.x;
        if (t < vector.y) t = vector.y;
        if (b > vector.y) b = vector.y;
        vector.x = left;
        vector.y = bottom;
        FT_Vector_Transform(&vector, &matrix);
        if (l > vector.x) l = vector.x;
        if (r < vector.x) r = vector.x;
        if (t < vector.y) t = vector.y;
        if (b > vector.y) b = vector.y;
        left = l;
        right = r;
        top = t;
        bottom = b;
    }
    left = FLOOR(left);
    right = CEIL(right);
    bottom = FLOOR(bottom);
    top = CEIL(top);

    int hpixels = TRUNC(right - left);
    if (hsubpixel)
        hpixels = hpixels*3 + 8;
#ifndef QT_NO_XRENDER
    XGlyphInfo info;
#else
    typedef struct _XGlyphInfoDummy {
        unsigned short  width;
        unsigned short  height;
        short           x;
        short           y;
        short           xOff;
        short           yOff;
    } XGlyphInfoDummy;
    XGlyphInfoDummy info;
#endif
    info.width = hpixels;
    info.height = TRUNC(top - bottom);
    info.x = -TRUNC(left);
    info.y = TRUNC(top);
    info.xOff = TRUNC(ROUND(slot->advance.x));
    info.yOff = 0;
    if (hsubpixel) {
        info.width /= 3;
        info.x += 1;
    }

    int pitch = (format == Format_Mono ? ((info.width + 31) & ~31) >> 3 :
                 (format == Format_A8 ? (info.width + 3) & ~3 : info.width * 4));
    int size = pitch * info.height;
    uchar *glyph_buffer = new uchar[size];

    if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
        FT_Bitmap bitmap;
        bitmap.rows = info.height*vfactor;
        bitmap.width = hpixels;
        bitmap.pitch = format == Format_Mono ? (((info.width + 31) & ~31) >> 3) : ((bitmap.width + 3) & ~3);
        if (!hsubpixel && vfactor == 1)
            bitmap.buffer = glyph_buffer;
        else
            bitmap.buffer = new uchar[bitmap.rows*bitmap.pitch];
        memset(bitmap.buffer, 0, bitmap.rows*bitmap.pitch);
        bitmap.pixel_mode = format == Format_Mono ? ft_pixel_mode_mono : ft_pixel_mode_grays;
        FT_Matrix matrix;
        matrix.xx = (hsubpixel ? 3 : 1) << 16;
        matrix.yy = vfactor << 16;
        matrix.yx = matrix.xy = 0;

        FT_Outline_Transform(&slot->outline, &matrix);
        FT_Outline_Translate (&slot->outline, (hsubpixel ? -3*left +(4<<6) : -left), -bottom*vfactor);
        FT_Outline_Get_Bitmap(library, &slot->outline, &bitmap);
        if (hsubpixel) {
            Q_ASSERT (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
            Q_ASSERT(fe->antialias);
            const uchar *src = bitmap.buffer;
            uchar *convoluted = new uchar[bitmap.rows*bitmap.pitch];
            uchar *c = convoluted;
            // convolute the bitmap with a triangle filter to get rid of color fringes
            // If we take account for a gamma value of 2, we end up with
            // weights of 1, 4, 9, 4, 1. We use an approximation of 1, 3, 8, 3, 1 here,
            // as this nicely sums up to 16 :)
            int h = info.height;
            while (h--) {
                c[0] = c[1] = 0;
                //
                for (int x = 2; x < bitmap.width - 2; ++x) {
                    uint sum = src[x-2] + 3*src[x-1] + 8*src[x] + 3*src[x+1] + src[x+2];
                    c[x] = (uchar) (sum >> 4);
                }
                c[bitmap.width - 2] = c[bitmap.width -1] = 0;
                src += bitmap.pitch;
                c += bitmap.pitch;
            }

            uint *dst = (uint *)glyph_buffer;
            src = convoluted;
            h = info.height;
            if (fe->subpixel == FC_RGBA_RGB) {
                while (h--) {
                    uint *dd = dst;
                    for (int x = 1; x < bitmap.width - 1; x += 3) {
                        uint red = src[x];
                        uint green = src[x+1];
                        uint blue = src[x+2];
                        uint res = (red << 16) + (green << 8) + blue;
                        *dd = res;
                        ++dd;
                    }
                    dst += info.width;
                    src += bitmap.pitch;
                }
            } else {
                while (h--) {
                    uint *dd = dst;
                    for (int x = 1; x < bitmap.width - 1; x += 3) {
                        uint blue = src[x];
                        uint green = src[x+1];
                        uint red = src[x+2];
                        uint res = (red << 16) + (green << 8) + blue;
                        *dd = res;
                        ++dd;
                    }
                    dst += info.width;
                    src += bitmap.pitch;
                }
            }
            delete [] convoluted;
            delete [] bitmap.buffer;
        } else if (vfactor != 1) {
            uchar *src = bitmap.buffer;
            size = info.width * 4 * info.height;
            uint *dst = (uint *)glyph_buffer;
            int h = info.height;
            if (fe->subpixel == FC_RGBA_VRGB) {
                while (h--) {
                    for (int x = 0; x < info.width; x++) {
                        uint red = src[x];
                        uint green = src[x+bitmap.pitch];
                        uint blue = src[x+2*bitmap.pitch];
                        uint high = (red*subpixel_filter[0][0] + green*subpixel_filter[0][1] + blue*subpixel_filter[0][2]) >> 8;
                        uint mid = (red*subpixel_filter[1][0] + green*subpixel_filter[1][1] + blue*subpixel_filter[1][2]) >> 8;
                        uint low = (red*subpixel_filter[2][0] + green*subpixel_filter[2][1] + blue*subpixel_filter[2][2]) >> 8;
                        uint res = (high << 16) + (mid << 8) + low;
                        dst[x] = res;
                    }
                    dst += info.width;
                    src += 3*bitmap.pitch;
                }
            } else {
                while (h--) {
                    for (int x = 0; x < info.width; x++) {
                        uint blue = src[x];
                        uint green = src[x+bitmap.pitch];
                        uint red = src[x+2*bitmap.pitch];
                        uint high = (red*subpixel_filter[0][0] + green*subpixel_filter[0][1] + blue*subpixel_filter[0][2]) >> 8;
                        uint mid = (red*subpixel_filter[1][0] + green*subpixel_filter[1][1] + blue*subpixel_filter[1][2]) >> 8;
                        uint low = (red*subpixel_filter[2][0] + green*subpixel_filter[2][1] + blue*subpixel_filter[2][2]) >> 8;
                        uint res = (high << 16) + (mid << 8) + low;
                        dst[x] = res;
                    }
                    dst += info.width;
                    src += 3*bitmap.pitch;
                }
            }
            delete [] bitmap.buffer;
        }
    } else if (slot->format == FT_GLYPH_FORMAT_BITMAP) {
        Q_ASSERT(slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO);
        uchar *src = slot->bitmap.buffer;
        uchar *dst = glyph_buffer;
        int h = slot->bitmap.rows;
        if (format == Format_Mono) {
            int bytes = ((info.width + 7) & ~7) >> 3;
            while (h--) {
                memcpy (dst, src, bytes);
                dst += pitch;
                src += slot->bitmap.pitch;
            }
        } else {
            if (hsubpixel) {
                while (h--) {
                    uint *dd = (uint *)dst;
                    *dd++ = 0;
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        uint a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xffffff : 0x000000);
                        *dd++ = a;
                    }
                    *dd++ = 0;
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            } else if (vfactor != 1) {
                while (h--) {
                    uint *dd = (uint *)dst;
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        uint a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xffffff : 0x000000);
                        *dd++ = a;
                    }
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            } else {
                while (h--) {
                    for (int x = 0; x < slot->bitmap.width; x++) {
                        unsigned char a = ((src[x >> 3] & (0x80 >> (x & 7))) ? 0xff : 0x00);
                        dst[x] = a;
                    }
                    dst += pitch;
                    src += slot->bitmap.pitch;
                }
            }
        }
    } else {
        qWarning("QFontEngine: Glyph neither outline nor bitmap format=%d", slot->format);
        delete [] glyph_buffer;
        return 0;
    }

#ifndef QT_NO_XRENDER
    if (add_to_glyphset) {
        if (format == Format_Mono) {
            /*
             * swap bit order around; FreeType is always MSBFirst
             */
            if (BitmapBitOrder(X11->display) != MSBFirst) {
                unsigned char *line = (unsigned char *) glyph_buffer;
                int i = size;
                i = size;
                while (i--) {
                    unsigned char c;
                    c = *line;
                    c = ((c << 1) & 0xaa) | ((c >> 1) & 0x55);
                    c = ((c << 2) & 0xcc) | ((c >> 2) & 0x33);
                    c = ((c << 4) & 0xf0) | ((c >> 4) & 0x0f);
                    *line++ = c;
                }
            }
        }

        ::Glyph xglyph = glyph;
        XRenderAddGlyphs (X11->display, glyphSet, &xglyph, &info, 1, (const char *)glyph_buffer, size);
        delete [] glyph_buffer;
        glyph_buffer = 0;
    }
#endif


    bool large_glyph = (((signed char)(slot->linearHoriAdvance>>16) != slot->linearHoriAdvance>>16)
                        || ((uchar)(info.width) != info.width)
                        || ((uchar)(info.height) != info.height)
                        || ((signed char)(info.x) != info.x)
                        || ((signed char)(info.y) != info.y)
                        || ((signed char)(info.xOff) != info.xOff));

    if (large_glyph) {
//         qDebug("got a large glyph!");
        return 0;
    }

    Glyph *g = new Glyph;

    g->linearAdvance = slot->linearHoriAdvance >> 10;
    g->width = info.width;
    g->height = TRUNC(top - bottom);
    g->x = -info.x;
    g->y = TRUNC(top);
    g->advance = TRUNC(ROUND(slot->advance.x));
    g->format = add_to_glyphset ? Format_None : format;
    g->data = glyph_buffer;

    // make sure we delete the old cached glyph
    delete glyph_data.value(glyph);
    glyph_data[glyph] = g;

    return g;
}
#ifndef QT_NO_XRENDER
bool QFontEngineFT::loadTransformedGlyphSet(glyph_t *glyphs, int num_glyphs, const QMatrix &matrix, GlyphSet *gs)
{
    // don't try to load huge fonts
    if (fontDef.pixelSize * sqrt(matrix.det()) >= 64) {
        *gs = 0;
        return false;
    }

    FT_Matrix m;
    m.xx = FT_Fixed(matrix.m11() * 65536);
    m.xy = FT_Fixed(-matrix.m21() * 65536);
    m.yx = FT_Fixed(-matrix.m12() * 65536);
    m.yy = FT_Fixed(matrix.m22() * 65536);

    Font *font = 0;

    for (int i = 0; i < transformedFonts.count(); ++i) {
        const Font &f = transformedFonts.at(i);
        if (f.transformationMatrix.xx == m.xx
            && f.transformationMatrix.xy == m.xy
            && f.transformationMatrix.yx == m.yx
            && f.transformationMatrix.yy == m.yy) {

            // found a match, move it to the front
            transformedFonts.move(i, 0);
            font = &transformedFonts[0];
            break;
        }
    }

    if (!font) {
        // don't cache more than 10 transformations
        if (transformedFonts.count() >= 10) {
            transformedFonts.move(transformedFonts.size() - 1, 0);
            XRenderFreeGlyphSet(X11->display, transformedFonts.at(0).glyphSet);
        } else {
            transformedFonts.prepend(Font());
        }
        font = &transformedFonts[0];

        qDeleteAll(font->glyph_data);
        font->glyph_data.clear();

        font->glyphSet = XRenderCreateGlyphSet(X11->display,
                         XRenderFindStandardFormat(X11->display, xglyph_format));

        font->transformationMatrix = m;
    }

    FT_Face face = 0;
    bool lockedFace = false;

    for (int i = 0; i < num_glyphs; ++i) {
        if (!font->glyph_data.contains(glyphs[i])) {
            if (!lockedFace) {
                face = lockFace();
                m = this->matrix;
                FT_Matrix_Multiply(&font->transformationMatrix, &m);
                FT_Set_Transform(face, &m, 0);
                freetype->matrix = m;
                lockedFace = true;
            }
            if (!font->loadGlyph(this, glyphs[i])) {
                FT_Set_Transform(face, &freetype->matrix, 0);
                unlockFace();
                return false;
            }
        }
    }
    *gs = font->glyphSet;

    if (lockedFace) {
        FT_Set_Transform(face, &freetype->matrix, 0);
        unlockFace();
    }

    return true;
}
#endif

inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

bool QFontEngineFT::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs,
                                 QTextEngine::ShaperFlags flags) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    bool mirrored = flags & QTextEngine::RightToLeft;
    int glyph_pos = 0;
    if (freetype->symbol_map) {
        FT_Face face = freetype->face;
        for ( int i = 0; i < len; ++i ) {
            unsigned int uc = getChar(str, i, len);
            if (mirrored)
                uc = QUnicodeTables::mirroredChar(uc);
            glyphs[glyph_pos].glyph = uc < QFreetypeFace::cmapCacheSize ? freetype->cmapCache[uc] : 0;
            if ( !glyphs[glyph_pos].glyph ) {
                glyph_t glyph;
                if (FcCharSetHasChar(freetype->charset, uc)) {
                redo0:
                    glyph = FT_Get_Char_Index(face, uc);
                    if (!glyph && (uc == 0xa0 || uc == 0x9)) {
                        uc = 0x20;
                        goto redo0;
                    }
                } else {
                    FT_Set_Charmap(face, freetype->symbol_map);
                    glyph = FT_Get_Char_Index(face, uc);
                    FT_Set_Charmap(face, freetype->unicode_map);
                }
                glyphs[glyph_pos].glyph = glyph;
                if (uc < QFreetypeFace::cmapCacheSize)
                    freetype->cmapCache[uc] = glyph;
            }
            ++glyph_pos;
        }
    } else {
        FT_Face face = freetype->face;
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            if (mirrored)
                uc = QUnicodeTables::mirroredChar(uc);
            glyphs[glyph_pos].glyph = uc < QFreetypeFace::cmapCacheSize ? freetype->cmapCache[uc] : 0;
            if (!glyphs[glyph_pos].glyph && FcCharSetHasChar(freetype->charset, uc)) {
            redo:
                glyph_t glyph = FT_Get_Char_Index(face, uc);
                if (!glyph && (uc == 0xa0 || uc == 0x9)) {
                    uc = 0x20;
                    goto redo;
                }
                glyphs[glyph_pos].glyph = glyph;
                if (uc < QFreetypeFace::cmapCacheSize)
                    freetype->cmapCache[uc] = glyph;
            }
            ++glyph_pos;
        }
    }

    *nglyphs = glyph_pos;
    recalcAdvances(*nglyphs, glyphs, flags);

    return true;
}

void QFontEngineFT::recalcAdvances(int len, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    FT_Face face = 0;
    if (flags & QTextEngine::DesignMetrics) {
        for (int i = 0; i < len; i++) {
            Glyph *g = fnt.glyph_data.value(glyphs[i].glyph);
            if (!g) {
                if (!face)
                    face = lockFace();
                g = loadGlyph(glyphs[i].glyph);
            }
            // for uncachable glyph, get advance from glyphslot
            glyphs[i].advance.x = QFixed::fromFixed(g ? g->linearAdvance : (face->glyph->linearHoriAdvance >> 10));
            glyphs[i].advance.y = 0;
        }
    } else {
        for (int i = 0; i < len; i++) {
            Glyph *g = fnt.glyph_data.value(glyphs[i].glyph);
            if (!g) {
                if (!face)
                    face = lockFace();
                g = loadGlyph(glyphs[i].glyph);
            }
            // for uncachable glyph, get advance from glyphslot
            glyphs[i].advance.x = g ? QFixed(g->advance) : QFixed::fromFixed(face->glyph->metrics.horiAdvance);
            glyphs[i].advance.y = 0;
        }
    }
    if (face)
        unlockFace();
}

glyph_metrics_t QFontEngineFT::boundingBox(const QGlyphLayout *glyphs, int numGlyphs)
{

    FT_Face face = 0;

    glyph_metrics_t overall;
    // initialize with line height, we get the same behaviour on all platforms
    overall.y = -ascent();
    overall.height = ascent() + descent() + 1;

    QFixed ymax = 0;
    QFixed xmax = 0;
    for (int i = 0; i < numGlyphs; i++) {
        Glyph *g = fnt.glyph_data.value(glyphs[i].glyph);
        if (!g) {
            if (!face)
                face = lockFace();
            g = loadGlyph(glyphs[i].glyph);
        }
        if (g) {
            QFixed x = overall.xoff + glyphs[i].offset.x + g->x;
            QFixed y = overall.yoff + glyphs[i].offset.y - g->y;
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, x + g->width);
            ymax = qMax(ymax, y + g->height);
            overall.xoff += qRound(g->advance);
        } else {
            int left  = FLOOR(face->glyph->metrics.horiBearingX);
            int right = CEIL(face->glyph->metrics.horiBearingX + face->glyph->metrics.width);
            int top    = CEIL(face->glyph->metrics.horiBearingY);
            int bottom = FLOOR(face->glyph->metrics.horiBearingY - face->glyph->metrics.height);

            QFixed x = overall.xoff + glyphs[i].offset.x - (-TRUNC(left));
            QFixed y = overall.yoff + glyphs[i].offset.y - TRUNC(top);
            overall.x = qMin(overall.x, x);
            overall.y = qMin(overall.y, y);
            xmax = qMax(xmax, x + TRUNC(right - left));
            ymax = qMax(ymax, y + TRUNC(top - bottom));
            overall.xoff += qRound(TRUNC(ROUND(face->glyph->advance.x)));
        }
    }
    overall.height = qMax(overall.height, ymax - overall.y);
    overall.width = xmax - overall.x;

    if (face)
        unlockFace();

    return overall;
}

glyph_metrics_t QFontEngineFT::boundingBox(glyph_t glyph)
{
    FT_Face face = 0;
    glyph_metrics_t overall;
    Glyph *g = fnt.glyph_data.value(glyph);
    if (!g) {
        face = lockFace();
        g = loadGlyph(glyph);
    }
    if (g) {
        overall.x = g->x;
        overall.y = -g->y;
        overall.width = g->width;
        overall.height = g->height;
        overall.xoff = g->advance;
    } else {
        int left  = FLOOR(face->glyph->metrics.horiBearingX);
        int right = CEIL(face->glyph->metrics.horiBearingX + face->glyph->metrics.width);
        int top    = CEIL(face->glyph->metrics.horiBearingY);
        int bottom = FLOOR(face->glyph->metrics.horiBearingY - face->glyph->metrics.height);

        overall.width = TRUNC(right-left);
        overall.height = TRUNC(top-bottom);
        overall.x = TRUNC(left);
        overall.y = -TRUNC(top);
        overall.xoff = TRUNC(ROUND(face->glyph->advance.x));
    }
    if (face)
        unlockFace();
    return overall;
}

bool QFontEngineFT::canRender(const QChar *string, int len)
{
    FT_Face face = freetype->face;
#if 0
    if (_cmap != -1) {
        lockFace();
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!FcCharSetHasChar (_font->charset, uc) && getAdobeCharIndex(face, _cmap, uc) == 0) {
                allExist = false;
                break;
            }
        }
        unlockFace();
    } else
#endif
    {
        for ( int i = 0; i < len; i++ ) {
            unsigned int uc = getChar(string, i, len);
            if (!FT_Get_Char_Index(face, uc))
                    return false;
        }
    }
    return true;
}

void QFontEngineFT::doKerning(int num_glyphs, QGlyphLayout *g, QTextEngine::ShaperFlags flags) const
{
    if (!FT_HAS_KERNING(freetype->face))
        return;
    FT_Face face = lockFace();
    uint f = (flags == QTextEngine::DesignMetrics ? FT_KERNING_UNFITTED : FT_KERNING_DEFAULT);
    for (int i = 0; i < num_glyphs-1; ++i) {
        FT_Vector kerning;
        FT_Get_Kerning(face, g[i].glyph, g[i+1].glyph, f, &kerning);
        g[i].advance.x += QFixed::fromFixed(kerning.x);
        g[i].advance.y += QFixed::fromFixed(kerning.y);
    }
    unlockFace();
}


void QFontEngineFT::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs,
                                    QPainterPath *path, QTextItem::RenderFlags)
{
    FT_Face face = lockFace();

    for (int gl = 0; gl < numGlyphs; gl++) {
        FT_UInt glyph = glyphs[gl];

        FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);

        FT_GlyphSlot g = face->glyph;
        if (g->format != FT_GLYPH_FORMAT_OUTLINE)
            continue;
        addGlyphToPath(g, positions[gl], path);
    }
    unlockFace();
}

void QFontEngineFT::addOutlineToPath(qreal x, qreal y, const QGlyphLayout *glyphs, int numGlyphs, QPainterPath *path, QTextItem::RenderFlags flags)
{
    if (FT_IS_SCALABLE(freetype->face)) {
        QFontEngine::addOutlineToPath(x, y, glyphs, numGlyphs, path, flags);
    } else {
        addBitmapFontToPath(x, y, glyphs, numGlyphs, path, flags);
    }
}

QImage QFontEngineFT::alphaMapForGlyph(glyph_t g)
{
    lockFace();

    Glyph *glyph = loadGlyph(g, Format_A8);
    if (!glyph) {
        unlockFace();
        return QFontEngine::alphaMapForGlyph(g);
    }

    Q_ASSERT(glyph->format == QFontEngineFT::Format_A8);

    const int pitch = (glyph->width + 3) & ~3;

    QImage img(glyph->width, glyph->height, QImage::Format_Indexed8);
    QVector<QRgb> colors(256);
    for (int i=0; i<256; ++i)
        colors[i] = qRgba(0, 0, 0, i);
    img.setColorTable(colors);

    for (int y = 0; y < glyph->height; ++y)
        memcpy(img.scanLine(y), &glyph->data[y * pitch], glyph->width);
    unlockFace();

    return img;
}

QFixed QFontEngineFT::ascent() const
{
    return QFixed::fromFixed(metrics.ascender);
}

QFixed QFontEngineFT::descent() const
{
    return QFixed::fromFixed(-metrics.descender + (1<<6));
}

QFixed QFontEngineFT::leading() const
{
    return QFixed::fromFixed(metrics.height - metrics.ascender + metrics.descender);
}

QFixed QFontEngineFT::xHeight() const
{
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(freetype->face, ft_sfnt_os2);
    if (os2 && os2->sxHeight)
        return QFixed(os2->sxHeight*freetype->face->size->metrics.y_ppem)/freetype->face->units_per_EM;
    return QFontEngine::xHeight();
}

QFixed QFontEngineFT::averageCharWidth() const
{
    TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(freetype->face, ft_sfnt_os2);
    if (os2 && os2->xAvgCharWidth)
        return QFixed(os2->xAvgCharWidth*freetype->face->size->metrics.y_ppem)/freetype->face->units_per_EM;
    return QFontEngine::averageCharWidth();
}


qreal QFontEngineFT::maxCharWidth() const
{
    return metrics.max_advance >> 6;
}

static const ushort char_table[] = {
        40,
        67,
        70,
        75,
        86,
        88,
        89,
        91,
        102,
        114,
        124,
        127,
        205,
        645,
        884,
        922,
        1070,
        12386
};

static const int char_table_entries = sizeof(char_table)/sizeof(ushort);


qreal QFontEngineFT::minLeftBearing() const
{
    if (lbearing == SHRT_MIN)
        (void) minRightBearing(); // calculates both
    return lbearing.toReal();
}

qreal QFontEngineFT::minRightBearing() const
{
    if (rbearing == SHRT_MIN) {
        lbearing = rbearing = 0;
        const QChar *ch = (const QChar *)char_table;
        QGlyphLayout glyphs[char_table_entries];
        int ng = char_table_entries;
        stringToCMap(ch, char_table_entries, glyphs, &ng, 0);
        while (--ng) {
            if (glyphs[ng].glyph) {
                glyph_metrics_t gi = ((QFontEngineFT *)this)->boundingBox(glyphs[ng].glyph);
                lbearing = qMin(lbearing, gi.x);
                rbearing = qMin(rbearing, (gi.xoff - gi.x - gi.width));
            }
        }
    }
    return rbearing.toReal();
}

QFixed QFontEngineFT::lineThickness() const
{
    return line_thickness;
}

QFixed QFontEngineFT::underlinePosition() const
{
    return underline_position;
}

QFontEngine::FaceId QFontEngineFT::faceId() const
{
    return face_id;
}

QFontEngine::Properties QFontEngineFT::properties() const
{
    Properties p = freetype->properties();
    if (p.postscriptName.isEmpty()) {
        p.postscriptName = fontDef.family.toUtf8();
        p.postscriptName = QPdf::stripSpecialCharacters(p.postscriptName);
    }

    return freetype->properties();
}


void QFontEngineFT::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    FT_Face face = lockFace();
    FT_Set_Transform(face, 0, 0);
    FT_Load_Glyph(face, glyph, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE);

    int left  = face->glyph->metrics.horiBearingX;
    int right = face->glyph->metrics.horiBearingX + face->glyph->metrics.width;
    int top    = face->glyph->metrics.horiBearingY;
    int bottom = face->glyph->metrics.horiBearingY - face->glyph->metrics.height;

    QFixedPoint p;
    p.x = 0;
    p.y = 0;

    if (!FT_IS_SCALABLE(freetype->face)) {
        metrics->width = QFixed::fromFixed(right-left);
        metrics->height = QFixed::fromFixed(top-bottom);
        metrics->x = QFixed::fromFixed(left);
        metrics->y = QFixed::fromFixed(-top);
        metrics->xoff = QFixed::fromFixed(face->glyph->advance.x);

        ::addBitmapToPath(face->glyph, p, path);
    } else {
        metrics->width = right-left;
        metrics->height = top-bottom;
        metrics->x = left;
        metrics->y = -top;
        metrics->xoff = face->glyph->advance.x;

        ::addGlyphToPath(face->glyph, p, path, true /* no_scale */);
    }

    FT_Set_Transform(face, &freetype->matrix, 0);
    unlockFace();
}

QByteArray QFontEngineFT::getSfntTable(uint tag) const
{
    return freetype->getSfntTable(tag);
}

int QFontEngineFT::synthesized() const
{
    int s = 0;
    if ((fontDef.style != QFont::StyleNormal) && !(freetype->face->style_flags & FT_STYLE_FLAG_ITALIC))
        s = SynthesizedItalic;
    if (fontDef.stretch != 100 && FT_IS_SCALABLE(freetype->face))
        s |= SynthesizedStretch;
    return s;
}

QOpenType *QFontEngineFT::openType() const
{
    if (_openType)
         return _openType;

    FT_Face face = lockFace();
    if (!face || !FT_IS_SFNT(face)) {
        unlockFace();
        return 0;
    }

    _openType = new QOpenType(const_cast<QFontEngineFT *>(this), face);
    unlockFace();
    return _openType;
}

#endif // QT_NO_FONTCONFIG
