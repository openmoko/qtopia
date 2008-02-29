/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3VBOX_H
#define Q3VBOX_H

#include <Qt3Support/q3hbox.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3VBox : public Q3HBox
{
    Q_OBJECT
public:
    Q3VBox(QWidget* parent=0, const char* name=0, Qt::WindowFlags f=0);

private:
    Q_DISABLE_COPY(Q3VBox)
};

QT_END_HEADER

#endif // Q3VBOX_H
