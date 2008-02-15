#ifndef __QTOPIA_MIDIDECODER_H
#define __QTOPIA_MIDIDECODER_H


#include <qtopiamedia/media.h>
#include <QMediaDecoder>


class QMediaDevice;


class MidiDecoderPrivate;

class MidiDecoder : public QMediaDecoder
{
    Q_OBJECT

public:
    MidiDecoder();
    ~MidiDecoder();

    QMediaDevice::Info const& dataType() const;

    bool connectToInput(QMediaDevice* input);
    void disconnectFromInput(QMediaDevice* input);

    void start();
    void stop();
    void pause();

    quint64 length();
    bool seek(qint64 ms);

    void setVolume(int volume);
    int volume();

    void setMuted(bool mute);
    bool isMuted();

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
