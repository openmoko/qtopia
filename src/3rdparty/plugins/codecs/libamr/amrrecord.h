#ifndef AMR_RECORD_H 
#define AMR_RECORD_H


#include <qstring.h>
#include <qapplication.h>
#include <mediarecorderplugininterface.h>

class AmrAudioFormatHandler;

class AmrRecorderPlugin : public MediaRecorderEncoder {

public:
    AmrRecorderPlugin();
    virtual ~AmrRecorderPlugin() { end(); }

    // About Plugin 
    int pluginNumFormats() const { return 1; }
    QString pluginFormatName( int ) const { return "MMS AMR"; }
    QString pluginFormatTag( int ) const { return "amr"; }
    QString pluginComment() const { return qApp->translate("AmrRecorder", "This plugin is used to record AMR files"); }
    double pluginVersion() const { return 1.0; }
    QString pluginMimeType() const { return "audio/amr"; }

    // I/O device management
    bool begin( QIODevice *device, const QString& formatTag );
    bool end();
    bool isActive() const { return ( device != 0 ); }

    // Audio record functionality
    bool setAudioChannels( int channels );
    bool setAudioFrequency( int frequency );
    bool writeAudioSamples( const short *samples, long numSamples );

    // Add comments and other meta information
    bool addComment( const QString&, const QString& ) { return TRUE; }

    // Space estimation.
    long estimateAudioBps( int frequency, int channels, const QString& formatTag );

    // Capabilities
    bool supportsAudio() const { return TRUE; }
    bool supportsVideo() const { return FALSE; }
    bool supportsComments() const { return FALSE; }
    bool requiresDirectAccess() const { return FALSE; }

private:

    QIODevice *device;
    int channels;
    int frequency;
    bool writtenHeader;
    short block[160];
    int blockLen;
    void *amrState;
    void *sidSync;
    AmrAudioFormatHandler *handler;

    // Write the AMR file header to the I/O device.  Called once
    // before outputting the first sample block, and then again
    // at the end of recording to update the length values.
    bool writeHeader();

    // Flush the current AMR block.
    bool amrFlush();

};


#endif

