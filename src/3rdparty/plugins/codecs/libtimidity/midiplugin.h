#ifndef __QTOPIA_MIDIPLUGIN_H
#define __QTOPIA_MIDIPLUGIN_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstringlist.h>

#include <qtopiamedia/qmediacodecplugin.h>



class MidiPlugin :
    public QObject,
    public QMediaCodecPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMediaCodecPlugin)

    class MidiPluginPrivate;

public:
    MidiPlugin();
    ~MidiPlugin();

    QString name() const;
    QString comment() const;
    QStringList mimeTypes() const;
    QStringList fileExtensions() const;

    double version() const;

    bool canEncode() const;
    bool canDecode() const;

    QMediaEncoder* encoder(QString const& mimeType);
    QMediaDecoder* decoder(QString const& mimeType);

private:
    MidiPluginPrivate*  d;
};

#endif  // __QTOPIA_MIDIPLUGIN_H
