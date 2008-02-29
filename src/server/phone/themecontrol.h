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

#ifndef _THEMECONTROL_H_
#define _THEMECONTROL_H_

#include <QObject>
#include <QList>
#include <QPair>
#include <QMap>

class ThemedView;
class QAbstractThemeWidgetFactory;

class ThemeControl : public QObject
{
    Q_OBJECT
public:
    static ThemeControl *instance();

    void registerThemedView(ThemedView *, const QString &);
    bool exportBackground() const;

    void refresh();

    void setThemeWidgetFactory(QAbstractThemeWidgetFactory *);

signals:
    void themeChanging();
    void themeChanged();

private:
    QString findFile(const QString &) const;
    void doTheme(ThemedView *, const QString &);
    void doThemeWidgets(ThemedView *view);

    ThemeControl();

    QString m_themeName;
    bool m_exportBackground;

    QMap<QString, QString> m_themeFiles;
    QList<QPair<ThemedView *, QString> > m_themes;

    QAbstractThemeWidgetFactory *m_widgetFactory;
};

#endif // _THEMECONTROL_H_

