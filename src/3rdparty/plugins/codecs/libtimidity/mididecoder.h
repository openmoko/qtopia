#ifndef __QTOPIA_MIDIDECODER_H
#define __QTOPIA_MIDIDECODER_H


#include <qtopiamedia/media.h>
#include <qtopiamedia/qmediadecoder.h>


class QMediaDevice;


class MidiDecoderPrivate;

class MidiDecoder : public QMediaDecoder
{
    Q_OBJECT

public:
    MidiDecoder();
    ~MidiDecoder();

    void setInputPipe(QMediaPipe* inputPipe);
    void setOutputPipe(QMediaPipe* outputPipe);

    void start();
    void stop();
    void pause();

    quint32 length();
    void seek(quint32 ms);

    void setVolume(int volume);
    int volume();

    void setMuted(bool mute);
    bool isMuted();

signals:
    void playerStateChanged(QtopiaMedia::State);
    void positionChanged(quint32);
    void lengthChanged(quint32);
    void volumeChanged(int);
    void volumeMuted(bool);

private slots:
    void next();

private:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    size_t readCallback(void* dst, size_t itemSize, size_t numItems);
    int closeCallback();

    static size_t midiReadCallback(void* ctx, void* ptr, size_t size, size_t nmemb);
    static int midiCloseCallback(void* ctx);

    MidiDecoderPrivate* d;
};

#endif  // __QTOPIA_MIDIDECODER_H
