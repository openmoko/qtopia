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

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_PICTURE)

class QPicture;
class QImage;
class QString;
class QStringList;

struct Q_GUI_EXPORT QPictureFormatInterface : public QFactoryInterface
{
    virtual bool loadPicture(const QString &format, const QString &filename, QPicture *) = 0;
    virtual bool savePicture(const QString &format, const QString &filename, const QPicture &) = 0;

    virtual bool installIOHandler(const QString &) = 0;
};

#define QPictureFormatInterface_iid "com.trolltech.Qt.QPictureFormatInterface"
Q_DECLARE_INTERFACE(QPictureFormatInterface, QPictureFormatInterface_iid)


class Q_GUI_EXPORT QPictureFormatPlugin : public QObject, public QPictureFormatInterface
{
    Q_OBJECT
    Q_INTERFACES(QPictureFormatInterface:QFactoryInterface)
public:
    explicit QPictureFormatPlugin(QObject *parent = 0);
    ~QPictureFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadPicture(const QString &format, const QString &filename, QPicture *pic);
    virtual bool savePicture(const QString &format, const QString &filename, const QPicture &pic);
    virtual bool installIOHandler(const QString &format) = 0;

};

#endif // QT_NO_LIBRARY || QT_NO_PICTURE

QT_END_HEADER

#endif // QPICTUREFORMATPLUGIN_H
