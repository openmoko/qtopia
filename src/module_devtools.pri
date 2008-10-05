#This file contains projects that support developers or provide demo implementations. 
#Usually they are not part of shipped devices

PROJECTS*=\
    tools/spygrind \
    tools/vsexplorer \
    settings/logging \
    3rdparty/applications/micro_httpd \
    settings/appservices

PROJECTS*= \
    server/memory/base \                        #OOM component
    server/memory/testmonitor \                 #optional monitor for development purposes
    
enable_cell {
    # This isn't supported but it's included anyway
    PROJECTS*=tools/phonesim
    qbuild:PROJECTS*=tools/phonesim/lib/phonesim
    else:PROJECTS*=tools/phonesim/lib
}


