/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef _THEMEBACKGROUND_H_
#define _THEMEBACKGROUND_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <themedview.h>
#include <qdrmcontent.h>
#include <QPixmap>
// This class must be visible to homescreen.cpp and secondarythemeddisplay.cpp
#include <qtopia/private/themedviewinterface_p.h>


class QPainter;
class QExportedBackground;
class ThemedView;


class ThemeBackground : public QObject
{
    Q_OBJECT
public:
    //XXX Should probably get secondary screen number from config.
    static const int PrimaryScreen;
    static const int SecondaryScreen;

    ThemeBackground(int screen = ThemeBackground::PrimaryScreen, QObject *parent=0);
    virtual ~ThemeBackground() {}

    void updateBackground(ThemedView *);

    static void polishWindows(int screen);

private:
    int screen;
    QExportedBackground *exportedBackground;
};

class ThemeBackgroundImagePlugin : public ThemedItemPlugin
{
    Q_OBJECT
public:
    ThemeBackgroundImagePlugin(int screen = ThemeBackground::PrimaryScreen);
    virtual ~ThemeBackgroundImagePlugin() {}

    void resize(int w, int h);
    void paint(QPainter *p, const QRect &r);

    enum DisplayMode { ScaleAndCrop, Stretch, Tile, Center, Scale };

#ifdef QTOPIA4_TODO
    QRESULT queryInterface(const QUuid&, QUnknownInterface**);
    Q_REFCOUNT
#endif

private:
    void renderSvg(int width, int height, Qt::AspectRatioMode mode);

private slots:
    void rightsExpired( const QDrmContent &content );

private:
    QDrmContent imgContent;
    QString imgName;
    QPixmap bg;
    int width;
    int height;
    DisplayMode dpMode;
    ulong ref;
    int screen;
};

#endif
//_THEMEBACKGROUND_H_
