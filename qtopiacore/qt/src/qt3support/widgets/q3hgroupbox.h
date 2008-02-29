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

#ifndef Q3HGROUPBOX_H
#define Q3HGROUPBOX_H

#include <Qt3Support/q3groupbox.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3HGroupBox : public Q3GroupBox
{
    Q_OBJECT
public:
    Q3HGroupBox( QWidget* parent=0, const char* name=0 );
    Q3HGroupBox( const QString &title, QWidget* parent=0, const char* name=0 );
    ~Q3HGroupBox();

private:
    Q_DISABLE_COPY(Q3HGroupBox)
};

QT_END_HEADER

#endif // Q3HGROUPBOX_H
