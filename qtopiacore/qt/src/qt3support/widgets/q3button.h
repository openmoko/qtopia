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

#ifndef Q3BUTTON_H
#define Q3BUTTON_H

#include <QtGui/qabstractbutton.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3Button : public QAbstractButton
{
    Q_OBJECT
public:
    Q3Button( QWidget* parent=0, const char* name=0, Qt::WindowFlags f=0 );
    ~Q3Button();

protected:
    virtual void drawButton( QPainter * );
    virtual void drawButtonLabel( QPainter * );
    void	paintEvent( QPaintEvent * );

};

QT_END_HEADER

#endif // Q3BUTTON_H
