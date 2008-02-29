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
#ifndef __QTOPIA_QEXPORTEDBACKGROUND_H__
#define __QTOPIA_QEXPORTEDBACKGROUND_H__

#include <qtopiaglobal.h>
#include <qobject.h>
class QExportedBackgroundPrivate;
class QPixmap;
class QColor;

class QTOPIA_EXPORT QExportedBackground : public QObject
{
    Q_OBJECT
public:
    explicit QExportedBackground(QObject *parent = 0);
    explicit QExportedBackground(int screen, QObject *parent = 0);
    ~QExportedBackground();

    QPixmap wallpaper() const;
    const QPixmap &background() const;
    bool isAvailable() const;

    static void initExportedBackground(int width, int height, int screen=0 );
    static void clearExportedBackground(int screen=0);
    static void setExportedBackgroundTint(int);
    static void setExportedBackground(const QPixmap &image, int screen=0);

signals:
    void wallpaperChanged();
    void changed();
    void changed(const QPixmap &);

private slots:
    void sysMessage(const QString&,const QByteArray&);

private:
    void getPixmaps();
    static void colorize(QPixmap &, const QPixmap &, const QColor &);

private:
    QExportedBackgroundPrivate *d;
    friend class QExportedBackgroundPrivate;
};
#endif //__QTOPIA_QEXPORTEDBACKGROUND_H__
