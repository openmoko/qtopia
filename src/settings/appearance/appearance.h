/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H


#include <qstrlist.h> 
#include <qasciidict.h>
#include "appearancesettingsbase.h"

struct WindowDecorationInterface;
struct StyleInterface;
class PluginLoader;
class SampleWindow;

class AppearanceSettings : public AppearanceSettingsBase
{ 
    Q_OBJECT

public:
    AppearanceSettings( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~AppearanceSettings();

protected:
    void accept();
    void done(int r);

protected slots:
    void colorSelected( int );
    void styleSelected( int );
    void decorationSelected( int );
    void fontSelected( const QString &name );
    void fontSizeSelected( const QString &sz );
    void fixSampleGeometry();

private:
    void populateStyleList();
    void populateColorList();
    void populateDecorationList();
    void populateFontList(const QString&, int);
    void setStyle( QWidget *w, QStyle *s );

private:
    WindowDecorationInterface *wdiface;
    PluginLoader *wdLoader;
    bool wdIsPlugin;
    StyleInterface *styleiface;
    PluginLoader *styleLoader;
    SampleWindow *sample;
    int prefFontSize;
    int maxFontSize;
    QStringList colorListIDs;
};


#endif // SETTINGS_H

