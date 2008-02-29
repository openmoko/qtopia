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

#include "previewwidgetbase.h"

#include <QVariant>

/*
 *  Constructs a PreviewWidgetBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
PreviewWidgetBase::PreviewWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : QWidget(parent, name, fl)
{
    setupUi(this);


    // signals and slots connections
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
PreviewWidgetBase::~PreviewWidgetBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PreviewWidgetBase::languageChange()
{
    retranslateUi(this);
}

void PreviewWidgetBase::init()
{
}

void PreviewWidgetBase::destroy()
{
}
