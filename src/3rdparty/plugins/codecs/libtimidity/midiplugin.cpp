
#include "mididecoder.h"
#include "midiplugin.h"


class MidiPlugin::MidiPluginPrivate
{
public:
    QStringList mimeTypes;
    QStringList fileExtensions;
};



MidiPlugin::MidiPlugin():
    d(new MidiPluginPrivate)
{
    d->mimeTypes << "audio/midi" << "audio/x-midi";
    d->fileExtensions << "mid" << "midi";
}

MidiPlugin::~MidiPlugin()
{
    delete d;
}


QString MidiPlugin::name() const
{
    return QLatin1String("Midi decoder");
}

QString MidiPlugin::comment() const
{
    return QString();
}

QStringList MidiPlugin::mimeTypes() const
{
    return d->mimeTypes;
}

QStringList MidiPlugin::fileExtensions()  const
{
    return d->fileExtensions;
}

double MidiPlugin::version() const
{
    return 0.01;
}

bool MidiPlugin::canEncode() const
{
    return false;
}

bool MidiPlugin::canDecode() const
{
    return true;
}

bool MidiPlugin::supportsVideo() const
{
    return false;
}

bool MidiPlugin::supportsAudio() const
{
    return true;
}


QMediaEncoder* MidiPlugin::encoder(QString const&)
{
    return 0;
}

QMediaDecoder* MidiPlugin::decoder(QString const& mimeType)
{
//    if (d->mimeTypes.indexOf(mimeType) != -1)
        return new MidiDecoder;

//   return 0;
}


QTOPIA_EXPORT_PLUGIN(MidiPlugin);

