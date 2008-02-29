/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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
