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

#ifndef PALETTEEDITORADVANCEDBASE_H
#define PALETTEEDITORADVANCEDBASE_H

#include "ui_paletteeditoradvancedbase.h"
#include <QVariant>

class ColorButton;

class PaletteEditorAdvancedBase : public QDialog, public Ui::PaletteEditorAdvancedBase
{
    Q_OBJECT

public:
    PaletteEditorAdvancedBase(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~PaletteEditorAdvancedBase();

protected slots:
    virtual void languageChange();

    virtual void init();
    virtual void destroy();
    virtual void onCentral(int);
    virtual void onChooseCentralColor();
    virtual void onChooseEffectColor();
    virtual void onEffect(int);
    virtual void onToggleBuildDisabled(bool);
    virtual void onToggleBuildEffects(bool);
    virtual void onToggleBuildInactive(bool);
    virtual void paletteSelected(int);

};

#endif // PALETTEEDITORADVANCEDBASE_H
