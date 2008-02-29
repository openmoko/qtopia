/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include <qtopiachannel.h>
#include <QPixmap>
#include <QDebug>
#include <qtopiaipcenvelope.h>
#include <qscreen_qws.h>
#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QDesktopWidget>

#include "qglobalpixmapcache.h"
#include "qexportedbackground.h"

static const int MaxScreens = 2;

class ExportedBackground {
public:
    ExportedBackground() : bgPm(0), bgState(0),
        exportedBackgroundAvailable(false) {}
    QPixmap *bgPm;
    QPixmap *bgState;
    bool exportedBackgroundAvailable;
};

struct ExportedBackgroundLocalInfo {
    ExportedBackgroundLocalInfo()
        : tintAmount(2) {}

    int tintAmount;
    ExportedBackground exportedBg[MaxScreens];
    QSet<QExportedBackground *> localInstance;
};

Q_GLOBAL_STATIC(ExportedBackgroundLocalInfo, localInfo);

class QExportedBackgroundPrivate
{
public:
    void primeWallpaper()
    {
        QSettings cfg("Trolltech","qpe");
        cfg.beginGroup("Appearance");
        wallpaper = cfg.value("BackgroundImage").toString();
        if(!wallpaper.startsWith('/'))
            wallpaper = QString(":image/wallpaper/"+wallpaper);
    }

    QPixmap bg;
    QPixmap state;
    QString wallpaper;
    int screen;
};


/*!
    \class QExportedBackground
    \mainclass
    \brief The QExportedBackground class provides access to the system
    background.

    Qtopia provides a global background to all windows in order to
    give the impression that windows are transparent.  This
    background is automatically available in all windows due to
    QPhoneStyle setting appropriate palettes for all widgets.  In some
    cases it is desireable to use the background directly, or to be
    notified that the background pixmap has changed.  QExportedBackground
    provides this functionality.
*/

/*!
    \fn void QExportedBackground::changed()

    This signal is emitted when the exported background changes.
*/

/*!
    \fn void QExportedBackground::changed(const QPixmap &background)

    This signal is emitted when the exported background changes.  The
    \a background argument is the new background.
*/

/*!
    \fn void QExportedBackground::wallpaperChanged()

    This signal is emitted when the wallpaper changes.

    \sa wallpaper()
*/

/*!
    Constructs a QExportedBackground for screen number \a screen with
    the given \a parent.

    Qt supports multiple screens.  This is commonly seen in flip phones
    with both internal and external screens.  The background for each screen
    may be different.  Qtopia assumes that the primary screen
    is screen number \c 0 and the secondary screen is screen number \c 1.

    \sa QDesktopWidget
*/
QExportedBackground::QExportedBackground(int screen, QObject *parent)
    : QObject(parent)
{
    d = new QExportedBackgroundPrivate;
    d->screen = screen;
    if (screen >= 0 && screen < MaxScreens) {
        QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
        connect( sysChannel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(sysMessage(QString,QByteArray)) );
        getPixmaps();
    }
    localInfo()->localInstance.insert(this);
}

/*!
    Constructs a QExportedBackground for the default screen with the
    given \a parent.
*/
QExportedBackground::QExportedBackground(QObject *parent)
    : QObject(parent)
{
    int scr = 0;
    d = new QExportedBackgroundPrivate;
    d->screen = scr;
    if (scr >= 0 && scr < MaxScreens) {
        QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
        connect( sysChannel, SIGNAL(received(QString,QByteArray)),
                this, SLOT(sysMessage(QString,QByteArray)) );
        getPixmaps();
    }
    localInfo()->localInstance.insert(this);
}

/*!
    Destroys a QExportedBackground.
*/
QExportedBackground::~QExportedBackground()
{
    if ( d )
        delete d;
    d = 0;
    localInfo()->localInstance.remove(this);
}

/*!
    Returns the wallpaper.
    
    The wallpaper is set by the user, for example
    a photo from the camera.  It is most commonly displayed only
    in the homescreen but some themes also export the wallpaper as the
    background.
*/
QPixmap QExportedBackground::wallpaper() const
{
    if (d->wallpaper.isEmpty())
        d->primeWallpaper();
    return QPixmap(d->wallpaper);
}

/*!
    Returns the exported background.
*/
const QPixmap &QExportedBackground::background() const
{
    return d->bg;
}

/*!
    Returns whether an exported background is available.
*/
bool QExportedBackground::isAvailable() const
{
    return (!d->state.isNull() && *d->state.qwsBits() && !d->bg.isNull());
}

void QExportedBackground::sysMessage(const QString &msg, const QByteArray&)
{
    if(msg == "backgroundChanged()") {
        getPixmaps();
        emit changed();
        emit changed(background());
    } else if ( msg == "applyStyleSplash()" ||
                msg == "applyStyleNoSplash()" ) {
        QString oldWallpaper = d->wallpaper;
        d->primeWallpaper();
        if(d->wallpaper != oldWallpaper)
            emit wallpaperChanged();
    }
}

void QExportedBackground::getPixmaps()
{
    d->bg = QPixmap();
    d->state = QPixmap();
    QString stateKey = QString("_$QTOPIA_BGSTATE_%1").arg(d->screen);
    QString bgKey = QString("_$QTOPIA_BG_%1").arg(d->screen);
    QGlobalPixmapCache::find(stateKey, d->state);
    QGlobalPixmapCache::find(bgKey, d->bg);
}

// Server side

/*!
    \internal
*/
void QExportedBackground::initExportedBackground(int width, int height, int screen)
{
    if (screen < 0 || screen >= MaxScreens)
        return;

    ExportedBackground &expBg = localInfo()->exportedBg[screen];
    if (expBg.bgState)
        return;
    expBg.exportedBackgroundAvailable = false;

    QString stateKey = QString("_$QTOPIA_BGSTATE_%1").arg(screen);
    QString bgKey = QString("_$QTOPIA_BG_%1").arg(screen);

    expBg.bgState = new QPixmap();
    expBg.bgPm = new QPixmap();
    QGlobalPixmapCache::find(stateKey, *expBg.bgState);
    if (expBg.bgState->isNull()) {
        *expBg.bgState = QPixmap(1,1);
        if (!QGlobalPixmapCache::insert(stateKey, *expBg.bgState)) {
            qWarning() << "Could not store exported background in global cache";
            return;
        }
        *((uchar*)expBg.bgState->qwsBits()) = 0; // Not set
    }
    QGlobalPixmapCache::find(bgKey, *expBg.bgPm);
    if (expBg.bgPm->isNull()) {
        QImage::Format fmt = QApplication::desktop()->depth() <= 16 ? QImage::Format_RGB16 : QImage::Format_ARGB32_Premultiplied;
        QImage img(width, height, fmt);
        *expBg.bgPm = QPixmap::fromImage(img);
        if(!QGlobalPixmapCache::insert(bgKey, *expBg.bgPm)) {
            qWarning() << "Could not store exported background in global cache";
            return;
        }
    }

    expBg.exportedBackgroundAvailable = true;
    QtopiaIpcEnvelope e("QPE/System", "backgroundChanged()");
}

/*!
    \internal
*/
void QExportedBackground::clearExportedBackground(int screen)
{
    if (screen < 0 || screen >= MaxScreens)
        return;

    ExportedBackground &expBg = localInfo()->exportedBg[screen];
    if(!expBg.exportedBackgroundAvailable)
        return;

    *((uchar*)expBg.bgState->qwsBits()) = 0; // Not set

    foreach(QExportedBackground *bg, localInfo()->localInstance)
        bg->getPixmaps();
}

/*!
    \internal
*/
void QExportedBackground::setExportedBackgroundTint(int tint)
{
    localInfo()->tintAmount = tint;
}

/*!
    \internal
*/
void QExportedBackground::setExportedBackground(const QPixmap &image, int screen)
{
    if (screen < 0 || screen >= MaxScreens)
        return;

    ExportedBackground &expBg = localInfo()->exportedBg[screen];
    if(!expBg.exportedBackgroundAvailable)
        return;

    if(image.isNull()) {
        clearExportedBackground();
        return;
    }

    // Get background color direct from settings in case style has
    // messed with it.
    QColor bgCol;
    QSettings config(QLatin1String("Trolltech"),QLatin1String("qpe"));
    config.beginGroup( QLatin1String("Appearance") );
    QString value = config.value("Background", "#EEEEEE").toString();
    if ( value[0] == '#' ) {
        bgCol = QColor(value);
        int alpha = config.value("Background_alpha", "64").toInt();
        bgCol.setAlpha(alpha);
    } else {
        bgCol = QApplication::palette().color(QPalette::Window);
    }

    colorize(*expBg.bgPm, image, bgCol);

    *((uchar*)expBg.bgState->qwsBits()) = 1; // Set
    QtopiaIpcEnvelope e("QPE/System", "backgroundChanged()");
   
    foreach(QExportedBackground *bg, localInfo()->localInstance)
        bg->getPixmaps();
}

void QExportedBackground::colorize(QPixmap &dest, const QPixmap &src,
                                   const QColor &colour)
{
    int sr, sg, sb;
    colour.getRgb(&sr, &sg, &sb);
    int level = colour.alpha();
    int div = 255;
    int mult = 255-level;
    QSize dataSize = QSize(src.width(),src.height());
    if (src.depth() == 16 && dest.depth() == 16) {
        sr = (sr << 8) & 0xF800;
        sg = (sg << 3) & 0x07e0;
        sb = sb >> 3;
        int const_sr = sr*level;
        int const_sg = sg*level;
        int const_sb = sb*level;
        int count = src.qwsBytesPerLine()/2 * dataSize.height();
        ushort *sp = (ushort *)src.qwsBits();
        ushort *dp = (ushort *)dest.qwsBits();
        for (int x = 0; x < count; x++, dp++, sp++) {
            quint32 spix = *sp;
            quint32 r = ((spix & 0x0000F800)*mult + const_sr)/div;
            quint32 g = ((spix & 0x000007e0)*mult + const_sg)/div;
            quint32 b = ((spix & 0x0000001f)*mult + const_sb)/div;
            *dp = (r&0xF800) | (g&0x07e0) | (b&0x001f);
        }
    } else if (src.depth() == 32 && dest.depth() == 32) {
        int map[3*256];
        int const_sr = sr*level;
        int const_sg = sg*level;
        int const_sb = sb*level;
        for (int i = 0; i < 256; i++)
        {
            map[i] = ((const_sr+i*mult)/div);
            map[i+256] = ((const_sg+i*mult)/div);
            map[i+512] = ((const_sb+i*mult)/div);
        }
        QRgb *srgb = (QRgb*)src.qwsBits();
        QRgb *drgb = (QRgb*)dest.qwsBits();
        int count = src.qwsBytesPerLine()/sizeof(QRgb) * src.height();
        for (int i = 0; i < count; i++, srgb++, drgb++) {
            int r = (*srgb >> 16) & 0xff;
            int g = (*srgb >> 8) & 0xff;
            int b = *srgb & 0xff;
            r = map[r];
            g = map[g+256];
            b = map[b+512];
            *drgb = qRgb(r, g, b);
        }
    } else if (src.depth() == 32 && dest.depth() == 16) {
        int map[3*256];
        int const_sr = sr*level;
        int const_sg = sg*level;
        int const_sb = sb*level;
        for (int i = 0; i < 256; i++)
        {
            map[i] = ((const_sr+i*mult)/div);
            map[i+256] = ((const_sg+i*mult)/div);
            map[i+512] = ((const_sb+i*mult)/div);
        }
        QRgb *srgb = (QRgb*)src.qwsBits();
        ushort *dp = (ushort *)dest.qwsBits();
        int count = src.qwsBytesPerLine()/sizeof(QRgb) * src.height();
        for (int i = 0; i < count; i++, srgb++, dp++) {
            int r = (*srgb >> 16) & 0xff;
            int g = (*srgb >> 8) & 0xff;
            int b = *srgb & 0xff;
            r = map[r];
            g = map[g+256];
            b = map[b+512];
            *dp = (((r>>3)<<11)&0xF800) | (((g>>2)<<5)&0x07e0) | ((b>>3)&0x001f);
        }
    }
}

