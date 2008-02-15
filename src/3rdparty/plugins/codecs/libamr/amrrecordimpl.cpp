#include "amrrecord.h"
#include "amrrecordimpl.h"


AmrRecorderPluginImpl::AmrRecorderPluginImpl()
{
}


AmrRecorderPluginImpl::~AmrRecorderPluginImpl()
{
}


QStringList AmrRecorderPluginImpl::keys() const
{
    QStringList list;
    return list << "MediaRecorderCodecPlugin";
}


MediaRecorderEncoder *AmrRecorderPluginImpl::encoder()
{
    return new AmrRecorderPlugin;
}
