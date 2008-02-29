/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "trwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QTextCodec>
#include <QTranslator>
#include <QSettings>
#include <QSplashScreen>
#include <QLibraryInfo>
#include <QLocale>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(linguist);

    QApplication app(argc, argv);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QTranslator translator(0);
    translator.load(QLatin1String("linguist_") + QLocale::system().name(), ".");
    app.installTranslator(&translator);

    QTranslator qtTranslator(0);
    qtTranslator.load(QLatin1String("qt_") + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    app.setOrganizationName("Trolltech");
    app.setApplicationName("Linguist");
    QString keybase(QString::number( (QT_VERSION >> 16) & 0xff ) +
                     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config;

    QWidget tmp;
    tmp.restoreGeometry(config.value(keybase + "Geometry/WindowGeometry").toByteArray());
    
    QSplashScreen *splash = 0;
    int screenId = QApplication::desktop()->screenNumber(tmp.geometry().center());
    splash = new QSplashScreen(QApplication::desktop()->screen(screenId),
        QPixmap(":/images/splash.png"));
    if (QApplication::desktop()->isVirtualDesktop()) {
        QRect srect(0, 0, splash->width(), splash->height());
        splash->move(QApplication::desktop()->availableGeometry(screenId).center() - srect.center() );
    }
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    TrWindow tw;
    tw.show();

    splash->finish(&tw);

    if (app.argc() > 1)
        tw.openFile(QString(app.argv()[app.argc() - 1]));

    QApplication::restoreOverrideCursor();

    return app.exec();
}
