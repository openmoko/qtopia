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

#ifndef QWIDGETITEMDATA_P_H
#define QWIDGETITEMDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QWidgetItemData
{
public:
    inline QWidgetItemData() : role(-1) {}
    inline QWidgetItemData(int r, QVariant v) : role(r), value(v) {}
    int role;
    QVariant value;
};

#ifndef QT_NO_DATASTREAM

inline QDataStream &operator>>(QDataStream &in, QWidgetItemData &data)
{
    in >> data.role;
    in >> data.value;
    return in;
}

inline QDataStream &operator<<(QDataStream &out, const QWidgetItemData &data)
{
    out << data.role;
    out << data.value;
    return out;
}

#endif

#endif // QWIDGETITEMDATA_P_H
