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

#ifndef Q3GRID_H
#define Q3GRID_H

#include <Qt3Support/q3frame.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class QGridLayout;

class Q_COMPAT_EXPORT Q3Grid : public Q3Frame
{
    Q_OBJECT
public:
    Q3Grid(int n, QWidget* parent=0, const char* name=0, Qt::WindowFlags f = 0);
    Q3Grid(int n, Qt::Orientation orient, QWidget* parent=0, const char* name=0,
	   Qt::WindowFlags f = 0);

    void setSpacing(int);
    QSize sizeHint() const;

    typedef Qt::Orientation Direction;

protected:
    void frameChanged();

private:
    Q_DISABLE_COPY(Q3Grid)
};

QT_END_HEADER

#endif // Q3GRID_H
