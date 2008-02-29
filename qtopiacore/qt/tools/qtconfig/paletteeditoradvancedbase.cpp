/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "paletteeditoradvancedbase.h"
#include "colorbutton.h"

#include <QVariant>

/*
 *  Constructs a PaletteEditorAdvancedBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
PaletteEditorAdvancedBase::PaletteEditorAdvancedBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(paletteCombo, SIGNAL(activated(int)), this, SLOT(paletteSelected(int)));
    connect(comboCentral, SIGNAL(activated(int)), this, SLOT(onCentral(int)));
    connect(buttonCentral, SIGNAL(clicked()), this, SLOT(onChooseCentralColor()));
    connect(buttonEffect, SIGNAL(clicked()), this, SLOT(onChooseEffectColor()));
    connect(comboEffect, SIGNAL(activated(int)), this, SLOT(onEffect(int)));
    connect(checkBuildEffect, SIGNAL(toggled(bool)), this, SLOT(onToggleBuildEffects(bool)));
    connect(checkBuildEffect, SIGNAL(toggled(bool)), comboEffect, SLOT(setDisabled(bool)));
    connect(checkBuildEffect, SIGNAL(toggled(bool)), buttonEffect, SLOT(setDisabled(bool)));
    connect(checkBuildInactive, SIGNAL(toggled(bool)), this, SLOT(onToggleBuildInactive(bool)));
    connect(checkBuildDisabled, SIGNAL(toggled(bool)), this, SLOT(onToggleBuildDisabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
PaletteEditorAdvancedBase::~PaletteEditorAdvancedBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PaletteEditorAdvancedBase::languageChange()
{
    retranslateUi(this);
}

void PaletteEditorAdvancedBase::init()
{
}

void PaletteEditorAdvancedBase::destroy()
{
}

void PaletteEditorAdvancedBase::onCentral(int)
{
    qWarning("PaletteEditorAdvancedBase::onCentral(int): Not implemented yet");
}

void PaletteEditorAdvancedBase::onChooseCentralColor()
{
    qWarning("PaletteEditorAdvancedBase::onChooseCentralColor(): Not implemented yet");
}

void PaletteEditorAdvancedBase::onChooseEffectColor()
{
    qWarning("PaletteEditorAdvancedBase::onChooseEffectColor(): Not implemented yet");
}

void PaletteEditorAdvancedBase::onEffect(int)
{
    qWarning("PaletteEditorAdvancedBase::onEffect(int): Not implemented yet");
}

void PaletteEditorAdvancedBase::onToggleBuildDisabled(bool)
{
    qWarning("PaletteEditorAdvancedBase::onToggleBuildDisabled(bool): Not implemented yet");
}

void PaletteEditorAdvancedBase::onToggleBuildEffects(bool)
{
    qWarning("PaletteEditorAdvancedBase::onToggleBuildEffects(bool): Not implemented yet");
}

void PaletteEditorAdvancedBase::onToggleBuildInactive(bool)
{
    qWarning("PaletteEditorAdvancedBase::onToggleBuildInactive(bool): Not implemented yet");
}

void PaletteEditorAdvancedBase::paletteSelected(int)
{
    qWarning("PaletteEditorAdvancedBase::paletteSelected(int): Not implemented yet");
}
