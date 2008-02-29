/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QWSPROPERTY_QWS_H
#define QWSPROPERTY_QWS_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

/*********************************************************************
 *
 * Class: QWSPropertyManager
 *
 *********************************************************************/

#ifndef QT_NO_QWS_PROPERTIES

class QWSPropertyManager
{
public:
    enum Mode {
        PropReplace = 0,
        PropPrepend,
        PropAppend
    };

    // pre-defined properties
    enum Atom {
        PropSelection = 0
    };

    QWSPropertyManager();
    ~QWSPropertyManager();

    bool setProperty(int winId, int property, int mode, const char *data, int len);
    bool hasProperty(int winId, int property);
    bool removeProperty(int winId, int property);
    bool addProperty(int winId, int property);
    bool getProperty(int winId, int property, const char *&data, int &len);
    bool removeProperties(int winId);

private:
    class Data;
    Data* d;
};

#endif // QT_NO_QWS_PROPERTIES

QT_END_HEADER

#endif // QWSPROPERTY_QWS_H
