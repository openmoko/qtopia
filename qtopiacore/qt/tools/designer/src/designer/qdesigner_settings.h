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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDESIGNER_SETTINGS_H
#define QDESIGNER_SETTINGS_H

#include <QtCore/QMap>
#include <QtCore/QRect>
#include <QtCore/QSettings>
#include <QtCore/QStringList>

struct Preferences;

class QDesignerSettings : public QSettings
{
public:
    QDesignerSettings();

    QStringList formTemplatePaths() const;
    void setFormTemplatePaths(const QStringList &paths);

    QString defaultUserWidgetBoxXml() const;

    void setGeometryFor(QWidget *w, const QRect &fallBack = QRect()) const;
    void saveGeometryFor(const QWidget *w);

    QStringList recentFilesList() const;
    void setRecentFilesList(const QStringList &list);

    void setShowNewFormOnStartup(bool showIt);
    bool showNewFormOnStartup() const;
    
    void setPreferences(const Preferences&);
    Preferences preferences() const;

    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &mainWindowState);

    QByteArray toolBoxState() const;
    void setToolBoxState(const QByteArray &state);

    void clearBackup();
    void setBackup(const QMap<QString, QString> &map);
    QMap<QString, QString> backup() const;
    
    static const QStringList &defaultFormTemplatePaths();
private:
    void setGeometryHelper(QWidget *w, const QString &key, const QRect &fallBack) const;
    void saveGeometryHelper(const QWidget *w, const QString &key);
    QStringList additionalFormTemplatePaths() const;
};

#endif // QDESIGNER_SETTINGS_H
