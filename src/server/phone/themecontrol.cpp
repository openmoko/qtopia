/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "themecontrol.h"
#include <QSettings>
#include <QDebug>
#include <themedview.h>
#include <qtopianamespace.h>
#include <qtopialog.h>

/*!
  \class ThemeControl
  \ingroup QtopiaServer::Task
  \brief The ThemeControl class manages the registered theme views.
 */

/*!
  Returns the ThemeControl instance.
  */
ThemeControl *ThemeControl::instance()
{
    static ThemeControl *control = 0;
    if(!control)
        control = new ThemeControl;
    return control;
}

/*! \internal */
ThemeControl::ThemeControl()
: m_exportBackground(false)
{
    refresh();
}

/*!
  Register a theme \a view with the given \a name.
  */
void ThemeControl::registerThemedView(ThemedView *view,
                                      const QString &name)
{
    m_themes.append(qMakePair(view, name));
    doTheme(view, name);
}

/*!
  Returns true if the theme configuration indicates that the background should
  be exported.
 */
bool ThemeControl::exportBackground() const
{
    return m_exportBackground;
}

/*! \internal */
QString ThemeControl::themeDir() const
{
    return m_theme;
}

/*!
  Returns the current theme directory.
  */
QString ThemeControl::currentTheme() const
{
    return m_themeDir;
}

/*!
  Update themed views.
  */
void ThemeControl::refresh()
{
    emit themeChanging();

    QSettings qpeCfg("Trolltech","qpe");
    qpeCfg.beginGroup("Appearance");
    m_exportBackground = qpeCfg.value("ExportBackground", true).toBool();

    m_themeDir = Qtopia::qtopiaDir() + "etc/themes/";
    m_theme = qpeCfg.value("Theme").toString(); // The server ensures this value is present and correct

    QSettings cfg(m_themeDir + m_theme, QSettings::IniFormat);
    cfg.beginGroup("Theme");
    m_themeName = cfg.value("Name[]", "Unnamed").toString(); //we must use the untranslated theme name
    if ( m_themeName == "Unnamed" )
        qLog(I18n) << "Invalid theme name: Cannot load theme translations";

    // XXX hack around broken QSettings
    QString str = cfg.value("ContextConfig").toString();
    QStringList keys = cfg.childKeys();

    m_themeFiles.clear();
    foreach(QString screen, keys)
        m_themeFiles.insert(screen, cfg.value(screen).toString());

    for(int ii = 0; ii < m_themes.count(); ++ii) {
        doTheme(m_themes.at(ii).first, m_themes.at(ii).second);
    }

    emit themeChanged();
}

/*!
  \fn ThemeControl::themeChanging()

  Emitted just before the theme is changed.
  */

/*!
  \fn ThemeControl::themeChanged()

  Emitted immediately after the theme changes.
 */

void ThemeControl::doTheme(ThemedView *view, const QString &name)
{
    QString path = m_themeFiles[name + "Config"];
    if(!path.isEmpty()) {
        QString file = m_themeDir + path;
        view->setThemeName(m_themeName);
        view->loadSource(m_themeDir + path);
    } else {
        qWarning("Invalid %s theme.", name.toAscii().constData());
    }
}

