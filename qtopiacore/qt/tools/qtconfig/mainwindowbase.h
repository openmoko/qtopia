/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MAINWINDOWBASE_H
#define MAINWINDOWBASE_H

#include "ui_mainwindowbase.h"
#include <QVariant>

class ColorButton;
class PreviewFrame;

class MainWindowBase : public Q3MainWindow, public Ui::MainWindowBase
{
    Q_OBJECT

public:
    MainWindowBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~MainWindowBase();

public slots:
    virtual void addFontpath();
    virtual void addSubstitute();
    virtual void browseFontpath();
    virtual void buildFont();
    virtual void buildPalette();
    virtual void downFontpath();
    virtual void downSubstitute();
    virtual void familySelected( const QString & );
    virtual void fileExit();
    virtual void fileSave();
    virtual void helpAbout();
    virtual void helpAboutQt();
    virtual void new_slot();
    virtual void pageChanged( QWidget * );
    virtual void paletteSelected( int );
    virtual void removeFontpath();
    virtual void removeSubstitute();
    virtual void somethingModified();
    virtual void styleSelected( const QString & );
    virtual void substituteSelected( const QString & );
    virtual void tunePalette();
    virtual void upFontpath();
    virtual void upSubstitute();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();
};

#endif // MAINWINDOWBASE_H
