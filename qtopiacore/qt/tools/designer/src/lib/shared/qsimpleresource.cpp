/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#include "qsimpleresource_p.h"
#include <QtDesigner/QtDesigner>

using namespace qdesigner_internal;

QSimpleResource::QSimpleResource(QDesignerFormEditorInterface *core)
    : QAbstractFormBuilder(), m_core(core)
{
    setWorkingDirectory(QDir(QDir::homePath()
                    + QDir::separator()
                    + QLatin1String(".designer")));
}

QSimpleResource::~QSimpleResource()
{

}

QBrush QSimpleResource::setupBrush(DomBrush *brush)
{
    return QAbstractFormBuilder::setupBrush(brush);
}

DomBrush *QSimpleResource::saveBrush(const QBrush &brush)
{
    return QAbstractFormBuilder::saveBrush(brush);
}

QIcon QSimpleResource::nameToIcon(const QString &filePath, const QString &qrcPath)
{
    QString file_path = filePath;
    QString qrc_path = qrcPath;

    if (qrc_path.isEmpty())
        file_path = workingDirectory().absoluteFilePath(file_path);
    else
        qrc_path = workingDirectory().absoluteFilePath(qrc_path);

    return core()->iconCache()->nameToIcon(file_path, qrc_path);
}

QString QSimpleResource::iconToFilePath(const QIcon &pm) const
{
    QString file_path = core()->iconCache()->iconToFilePath(pm);
    QString qrc_path = core()->iconCache()->iconToQrcPath(pm);
    if (qrc_path.isEmpty())
        return workingDirectory().relativeFilePath(file_path);

    return file_path;
}

QString QSimpleResource::iconToQrcPath(const QIcon &pm) const
{
    QString qrc_path = core()->iconCache()->iconToQrcPath(pm);
    if (qrc_path.isEmpty())
        return QString();

    return workingDirectory().relativeFilePath(qrc_path);
}

QPixmap QSimpleResource::nameToPixmap(const QString &filePath, const QString &qrcPath)
{
    QString file_path = filePath;
    QString qrc_path = qrcPath;

    if (qrc_path.isEmpty())
        file_path = workingDirectory().absoluteFilePath(file_path);
    else
        qrc_path = workingDirectory().absoluteFilePath(qrc_path);

    return core()->iconCache()->nameToPixmap(file_path, qrc_path);
}

QString QSimpleResource::pixmapToFilePath(const QPixmap &pm) const
{
    QString file_path = core()->iconCache()->pixmapToFilePath(pm);
    QString qrc_path = core()->iconCache()->pixmapToQrcPath(pm);
    if (qrc_path.isEmpty())
        return workingDirectory().relativeFilePath(file_path);

    return file_path;
}

QString QSimpleResource::pixmapToQrcPath(const QPixmap &pm) const
{
    QString qrc_path = core()->iconCache()->pixmapToQrcPath(pm);
    if (qrc_path.isEmpty())
        return QString();

    return workingDirectory().relativeFilePath(qrc_path);
}

