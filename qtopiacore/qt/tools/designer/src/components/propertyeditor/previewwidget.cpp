/****************************************************************************
**
** Copyright (C) 2005-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "previewwidget.h"

using namespace qdesigner_internal;

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

PreviewWidget::~PreviewWidget()
{
}

