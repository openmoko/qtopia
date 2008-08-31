/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "touchscreenlockdlg.h"
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QTimeLine>
#include <QFileInfo>
#ifndef QT_NO_PICTURE
#include <QPicture>
#else
#ifndef QTOPIA_NO_SVG
#include <QSvgRenderer>
#endif
#endif

class KeyItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    KeyItem(const QPixmap &pixmap, QGraphicsItem *parent = 0)
      : QObject(0), QGraphicsPixmapItem(pixmap, parent)
    {
        setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
        b_unlocked = false;
        timeLine = new QTimeLine(500, this);
        timeLine->setFrameRange(0, 100);
        connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(move(int)));
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        qreal newPos = event->scenePos().y() - event->lastPos().y();

        qreal y1 = scene()->sceneRect().height() * 0.18;
        qreal y2 = scene()->sceneRect().height() * 0.2;

        if (newPos > y1 && newPos < initialY) {
            setPos(pos().x(), newPos);
            b_unlocked = false;
        }
        if (newPos <= y2) {
            setPos(pos().x(), y1);
            b_unlocked = true;
        }
    }

    void setInitialPos(qreal x, qreal y)
    {
        initialY = y;
        setPos(x, y);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        timeLine->stop();
        timeLine->setStartFrame(0);
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        if (b_unlocked) {
            emit unlocked();
            return;
        }
        d = (initialY - scenePos().y()) / 100.0;
        ini = scenePos().y();
        timeLine->start();
    }

protected Q_SLOTS:
    void move(int i)
    {
        setPos(pos().x(), ini + (i * d));
    }

Q_SIGNALS:
    void unlocked();

private:
    bool b_unlocked;
    QTimeLine *timeLine;
    qreal d;
    qreal ini;
    qreal initialY;
};

TouchScreenLockDialog::TouchScreenLockDialog(QWidget *parent, Qt::WFlags fl)
  : QDialog(parent, fl), scene(0)
{
    //set palette before setting window state so we get correct transparency
    QPalette p = palette();
    p.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(p);
    
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowState(Qt::WindowFullScreen);
    setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vb = new QVBoxLayout;
    vb->setMargin(0);
    QGraphicsView *v = new QGraphicsView(this);
    v->setFrameStyle(QFrame::NoFrame);
    v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scene = new QGraphicsScene(this);
    QDesktopWidget *desktop = QApplication::desktop();
    scene->setSceneRect(desktop->screenGeometry(desktop->primaryScreen()));
    v->setScene(scene);
    vb->addWidget(v);
    setLayout(vb);

    int kw = (int)(scene->sceneRect().width() * 0.14);
    int kh = (int)(scene->sceneRect().height() * 0.21);
    KeyItem *key = new KeyItem(generatePixmap(":image/qpe/Key", kw, kh), 0);
    QObject::connect (key, SIGNAL(unlocked()), this, SLOT(close()));

    int lw = (int)(scene->sceneRect().width() * 0.27);
    int lh = (int)(scene->sceneRect().height() * 0.29);
    QGraphicsPixmapItem *lock = new QGraphicsPixmapItem(generatePixmap(":image/qpe/Lock", lw, lh), 0);
    scene->addItem(key);
    scene->addItem(lock);

    int dw = desktop->screenGeometry(desktop->primaryScreen()).width();
    int dh = desktop->screenGeometry(desktop->primaryScreen()).height();

    lock->setPos(dw/2 - lock->boundingRect().width()/2, 0);
    lock->setZValue(2);

    key->setInitialPos(dw/2 - key->boundingRect().width()/2, dh - key->boundingRect().height());
    key->setFlag(QGraphicsItem::ItemIsMovable);
    lock->setZValue(1);

    activateWindow();
    raise();
}

QPixmap TouchScreenLockDialog::generatePixmap(const QString &filename, int width, int height) const
{
    QFileInfo fileInfo(filename);

    if (!fileInfo.exists())
        return QPixmap();

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);
    QPainter painter(&image);

#ifndef QT_NO_PICTURE
    QPicture picture;
    picture.load(fileInfo.filePath());
    QRect br = picture.boundingRect();
    painter.scale(qreal(width) / br.width(), qreal(height) / br.height());
    painter.drawPicture(0, 0, picture);
#else
#ifndef QTOPIA_NO_SVG
    QSvgRenderer renderer(fileInfo.filePath());
    renderer.render(&painter);
#endif
#endif
    painter.end();
    return QPixmap::fromImage(image);
}

#include "touchscreenlockdlg.moc"

