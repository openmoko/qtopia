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

#ifndef QMOVIE_H
#define QMOVIE_H

#include <QtCore/qobject.h>

#ifndef QT_NO_MOVIE

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtGui/qimagereader.h>

#ifdef QT3_SUPPORT
#include <QtGui/qimage.h>
#include <QtGui/qpixmap.h>
#endif

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QByteArray;
class QColor;
class QIODevice;
class QImage;
class QPixmap;
class QRect;
class QSize;

class QMoviePrivate;
class Q_GUI_EXPORT QMovie : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMovie)
    Q_ENUMS(MovieState CacheMode)
    Q_PROPERTY(int speed READ speed WRITE setSpeed)
    Q_PROPERTY(CacheMode cacheMode READ cacheMode WRITE setCacheMode)
public:
    enum MovieState {
        NotRunning,
        Paused,
        Running
    };
    enum CacheMode {
        CacheNone,
        CacheAll
    };

    QMovie(QObject *parent = 0);
    explicit QMovie(QIODevice *device, const QByteArray &format = QByteArray(), QObject *parent = 0);
    explicit QMovie(const QString &fileName, const QByteArray &format = QByteArray(), QObject *parent = 0);
    ~QMovie();

    static QList<QByteArray> supportedFormats();

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    MovieState state() const;

    QRect frameRect() const;
    QImage currentImage() const;
    QPixmap currentPixmap() const;

    bool isValid() const;

    bool jumpToFrame(int frameNumber);
    int loopCount() const;
    int frameCount() const;
    int nextFrameDelay() const;
    int currentFrameNumber() const;

    int speed() const;

    QSize scaledSize();
    void setScaledSize(const QSize &size);

    CacheMode cacheMode() const;
    void setCacheMode(CacheMode mode);

    CacheMode cacheMode(); // ### Qt 5: remove me

Q_SIGNALS:
    void started();
    void resized(const QSize &size);
    void updated(const QRect &rect);
    void stateChanged(QMovie::MovieState state);
    void error(QImageReader::ImageReaderError error);
    void finished();
    void frameChanged(int frameNumber);

public Q_SLOTS:
    void start();
    bool jumpToNextFrame();
    void setPaused(bool paused);
    void stop();
    void setSpeed(int percentSpeed);

private:
    Q_DISABLE_COPY(QMovie)
    Q_PRIVATE_SLOT(d_func(), void _q_loadNextFrame())

#ifdef QT3_SUPPORT
public:
    inline QT3_SUPPORT bool isNull() const { return isValid(); }
    inline QT3_SUPPORT int frameNumber() const { return currentFrameNumber(); }
    inline QT3_SUPPORT bool running() const { return state() == Running; }
    inline QT3_SUPPORT bool paused() const { return state() == Paused; }
    inline QT3_SUPPORT bool finished() const { return state() == NotRunning; }
    inline QT3_SUPPORT void restart() { stop(); start(); }
    inline QT3_SUPPORT QImage frameImage() const { return currentImage(); }
    inline QT3_SUPPORT QPixmap framePixmap() const { return currentPixmap(); }
    inline QT3_SUPPORT void step() { jumpToNextFrame(); }
    inline QT3_SUPPORT void pause() { setPaused(true); }
    inline QT3_SUPPORT void unpause() { setPaused(false); }
#endif
};

QT_END_HEADER

#endif // QT_NO_MOVIE

#endif // QMOVIE_H
