/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QPIXMAPWHEEL_H_
#define _QPIXMAPWHEEL_H_

#include <QWidget>
#include <QPixmap>
#include <QList>
class QPainter;
class QTimeLine;
class QString;

// declare QPixmapWheelData
class QPixmapWheelData
{
public:
    QPixmapWheelData();
    QPixmapWheelData(const QPixmapWheelData &other);
    QPixmapWheelData &operator=(const QPixmapWheelData &other);

    void appendItem(const QString &name, const QPixmap &pix,
                    const QString &text);

    int count() const;

    QString name(int) const;
    QString text(int) const;
    QPixmap pixmap(int) const;

private:
    QList<QPixmap> m_pics;
    QList<QString> m_names;
    QList<QString> m_texts;
};

// declare QPixmapWheel
class QPixmapWheel : public QWidget
{
Q_OBJECT
public:
    QPixmapWheel(QWidget *parent = 0);

    void setWheel(const QPixmapWheelData &);
    void moveToWheel(const QPixmapWheelData &);

    int maximumVisibleIcons() const;
    void setMaximumVisibleIcons(int);

signals:
    void itemSelected(const QString &);
    void moveToCompleted();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void timelinePos(qreal value);

private:
    enum Direction { Forward, Backward };


    // static qreal addDegrees(qreal orig, qreal inc);
    static qreal normalizeDegrees(qreal orig, qreal arcSize);
    static qreal arcSize(qreal orig, qreal dest, qreal sweep, Direction dir);

    struct PixState
    {
        qreal x;
        qreal y;
        qreal size;
        qreal trans;
    };
    PixState itemPosition(qreal degrees);

    int circleWidth() const;
    int circleHeight() const;


    qreal m_pos; // Represents the wheel position NOW
    qreal m_startPos; // Represents the position the wheel was last time
                      // it started moving
    qreal m_moveArc; // Represents the arc over which the wheel is moving
    Direction m_direction; // Direction the wheel is moving over the arc

    QPixmapWheelData m_wheel;
    QPixmapWheelData m_incomingWheel;
    bool m_changingWheel;
    bool m_sweepStarted;
    int m_selected;

    QTimeLine *m_timeLine;

    int m_maximumIcons;

    void moveOverArc(qreal arc, Direction dir);
    void moveToPosition(int pos, Direction dir);
    void moveToRotation(qreal pos, Direction dir);

    void draw(QPainter *, qreal degrees, const QPixmapWheelData &, int);

    qreal degrees(const QPixmapWheelData &, int);
    qreal wheelArc(const QPixmapWheelData &);
    int segments(const QPixmapWheelData &);

    static qreal squashDegrees(qreal itemPos, qreal wheelPos, qreal wheelArc);
};


#endif // _QPIXMAPWHEEL_H_
