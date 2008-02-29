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

#ifndef QTEXTOPTION_H
#define QTEXTOPTION_H

#include <QtCore/qnamespace.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

template <typename T> class QList;
struct QTextOptionPrivate;

class Q_GUI_EXPORT QTextOption
{
public:
    QTextOption();
    QTextOption(Qt::Alignment alignment);
    ~QTextOption();

    QTextOption(const QTextOption &o);
    QTextOption &operator=(const QTextOption &o);

    inline void setAlignment(Qt::Alignment alignment);
    inline Qt::Alignment alignment() const { return Qt::Alignment(align); }

    inline void setTextDirection(Qt::LayoutDirection aDirection) { this->direction = aDirection; }
    inline Qt::LayoutDirection textDirection() const { return Qt::LayoutDirection(direction); }

    enum WrapMode {
        NoWrap,
        WordWrap,
        ManualWrap,
        WrapAnywhere,
        WrapAtWordBoundaryOrAnywhere
    };
    inline void setWrapMode(WrapMode wrap) { wordWrap = wrap; }
    inline WrapMode wrapMode() const { return static_cast<WrapMode>(wordWrap); }

    enum Flag {
        IncludeTrailingSpaces = 0x80000000
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    inline void setFlags(Flags flags);
    inline Flags flags() const { return Flags(f); }

    inline void setTabStop(qreal tabStop);
    inline qreal tabStop() const { return tab; }

    void setTabArray(QList<qreal> tabStops);
    QList<qreal> tabArray() const;

    void setUseDesignMetrics(bool b) { design = b; }
    bool useDesignMetrics() const { return design; }

private:
    uint align : 8;
    uint wordWrap : 4;
    uint design : 1;
    uint direction : 1;
    uint unused : 19;
    uint f;
    qreal tab;
    QTextOptionPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextOption::Flags)

inline void QTextOption::setAlignment(Qt::Alignment aalignment)
{ align = aalignment; }

inline void QTextOption::setFlags(Flags aflags)
{ f = aflags; }

inline void QTextOption::setTabStop(qreal atabStop)
{ tab = atabStop; }

QT_END_HEADER

#endif // QTEXTOPTION_H
