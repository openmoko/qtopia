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

#ifndef Q3HBOX_H
#define Q3HBOX_H

#include <Qt3Support/q3frame.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class QBoxLayout;

class Q_COMPAT_EXPORT Q3HBox : public Q3Frame
{
    Q_OBJECT
public:
    Q3HBox(QWidget* parent=0, const char* name=0, Qt::WindowFlags f=0);

    void setSpacing(int);
    bool setStretchFactor(QWidget*, int stretch);
    QSize sizeHint() const;

protected:
    Q3HBox(bool horizontal, QWidget* parent, const char* name, Qt::WindowFlags f = 0);
    void frameChanged();

private:
    Q_DISABLE_COPY(Q3HBox)
};

QT_END_HEADER

#endif // Q3HBOX_H
