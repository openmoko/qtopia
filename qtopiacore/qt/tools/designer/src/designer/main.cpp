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

#include "qdesigner.h"
#include <QLibraryInfo>
#include <QDir>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(designer);

    // report Qt usage for commercial customers with a "metered license" (currently experimental)
#if QT_EDITION != QT_EDITION_OPENSOURCE
    QString reporterPath = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator()
                           + "qtusagereporter";
#if defined(Q_OS_WIN)
    reporterPath += ".exe";
#endif
    if (QFile::exists(reporterPath))
        system(qPrintable(reporterPath + " designer"));
#endif

    QDesigner app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    return app.exec();
}
