#ifndef AMR_RECORD_IMPL_H 
#define AMR_RECORD_IMPL_H


#include <mediarecorderplugininterface.h>


class AmrRecorderPlugin;


class AmrRecorderPluginImpl : public MediaRecorderCodecPlugin
{
public:
    AmrRecorderPluginImpl();
    virtual ~AmrRecorderPluginImpl();

    QStringList keys() const;

    virtual MediaRecorderEncoder *encoder();
};

QTOPIA_EXPORT_PLUGIN( AmrRecorderPluginImpl )


#endif

