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

#ifndef Q3GRIDLAYOUT_H
#define Q3GRIDLAYOUT_H

#include <QtGui/qboxlayout.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3GridLayout : public QGridLayout
{
public:
    inline explicit Q3GridLayout(QWidget *parent)
        : QGridLayout(parent) { setMargin(0); setSpacing(0); }

    inline Q3GridLayout(QWidget *parent, int nRows, int nCols = 1, int margin = 0,
                        int spacing = -1, const char *name = 0)
        : QGridLayout(parent, nRows, nCols, margin, spacing, name) {}

    inline Q3GridLayout(int nRows, int nCols = 1, int spacing = -1, const char *name = 0)
        : QGridLayout(nRows, nCols, spacing, name) {}

    inline Q3GridLayout(QLayout *parentLayout, int nRows =1, int nCols = 1, int spacing = -1,
                        const char *name = 0)
        : QGridLayout(parentLayout, nRows, nCols, spacing, name) {}

private:
    Q_DISABLE_COPY(Q3GridLayout)
};

QT_END_HEADER

#endif // Q3GRIDLAYOUT_H
