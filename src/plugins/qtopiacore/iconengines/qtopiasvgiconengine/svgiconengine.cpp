/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "svgiconengine.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qsvgrenderer.h>
#include <qpixmapcache.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qstyleoption.h>
#include <qfileinfo.h>
#include <qtopialog.h>
#include <QGlobalPixmapCache>
#include <qiconengineplugin.h>
#include <qstringlist.h>
#include <qiodevice.h>

//#define DEBUG_SVG_ICONENGINE

class QtopiaSvgIconPlugin : public QIconEnginePlugin
{
public:
    QStringList keys() const;
    QIconEngine *create(const QString &filename);
};

QStringList QtopiaSvgIconPlugin::keys() const
{
    return QStringList() << "svg";
}

QIconEngine *QtopiaSvgIconPlugin::create(const QString &file)
{
    QtopiaSvgIconEngine *engine = new QtopiaSvgIconEngine();
    engine->addFile(file, QSize(), QIcon::Normal, QIcon::On);
    return engine;
}

QTOPIA_EXPORT_QT_PLUGIN(QtopiaSvgIconPlugin)


class QtopiaSvgIconEnginePrivate : public QSharedData
{
public:
    explicit QtopiaSvgIconEnginePrivate()
    {
        render = new QSvgRenderer;
        loaded = false;
        pixmaps = 0;
    }
    ~QtopiaSvgIconEnginePrivate()
    {
        delete render;
        render = 0;
    }

    QSvgRenderer *render;
    QString filename;
    bool loaded;
    QMap<QString,QPixmap> *pixmaps;
};

static inline QString createKey(const QString &filename, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QString key = filename + QString::number(size.width()) + QLatin1String("_")
                    + QString::number(size.height()) + QLatin1String("_")
                    + QString::number(mode) + QString::number(state);

    return key;
}

QtopiaSvgIconEngine::QtopiaSvgIconEngine()
    : d(new QtopiaSvgIconEnginePrivate)
{

}


QtopiaSvgIconEngine::~QtopiaSvgIconEngine()
{
}


QSize QtopiaSvgIconEngine::actualSize(const QSize &size, QIcon::Mode,
                                 QIcon::State )
{
    return size;
}


QPixmap QtopiaSvgIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                               QIcon::State state)
{
    QString key = createKey(d->filename, size, mode, state);
    QPixmap pm;

    // Try explicitly added pixmaps first
    if (d->pixmaps) {
        if (d->pixmaps->contains(key))
            return d->pixmaps->value(key);
    }

    // See if we have it in our local cache first.
    if (QPixmapCache::find(key, pm))
        return pm;

    // Perhaps it has already been stored in the global cache.
    bool globalCandidate = false;
    if (size.height() == QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize)) {
        if (QGlobalPixmapCache::find(key, pm)) {
            qLog(Resource) << "Icon found in global cache" << d->filename;
            return pm;
        }
        globalCandidate = true;
        qLog(Resource) << "Icon not found in global cache" << d->filename;
    }

    if (!d->loaded) {
        d->render->load(d->filename);
        qLog(Resource) << "loaded svg icon" << d->filename;
        d->loaded = true;
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0x00000000);
    QPainter p(&img);
    d->render->render(&p);
    p.end();
    pm = QPixmap::fromImage(img);
    QStyleOption opt(0);
    opt.palette = QApplication::palette();
    QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
    if (!generated.isNull())
        pm = generated;

    // We'll only put the standard icon sizes in the cache because
    // there's a high likelyhood that they'll be used by others.
    if (globalCandidate) {
        if (QGlobalPixmapCache::insert(key, pm))
            return pm;
    }

    // Still worthwhile putting in the local cache since it is very likely
    // to be rendered again
    QPixmapCache::insert(key, pm);

    return pm;
}


void QtopiaSvgIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                               QIcon::State state)
{
    QString key = createKey(d->filename, pixmap.size(), mode, state);
    if (!d->pixmaps)
        d->pixmaps = new QMap<QString,QPixmap>;
    d->pixmaps->insert(key, pixmap);
}


void QtopiaSvgIconEngine::addFile(const QString &fileName, const QSize &,
                             QIcon::Mode, QIcon::State)
{
    if (!fileName.isEmpty()) {
        QString abs = fileName;
        if (fileName.at(0) != QLatin1Char(':'))
            abs = QFileInfo(fileName).absoluteFilePath();
        d->filename = abs;
//        d->render->load(abs);
    }
}

void QtopiaSvgIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}
