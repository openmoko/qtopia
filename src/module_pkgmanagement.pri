#This file contains projects that make up the PackageManagement module

enable_sxe {
    PROJECTS*=server/security/monitor \         #(re)starts sxe monitor process
              server/security/sandboxedexe      #sxe application launcher
        
    PROJECTS*=\
            libraries/qtopiasecurity\
            tools/sxe_installer \
            tools/sxe_policy_runner \
            tools/sxemonitor \
            server/phone/telephony/msgcontrol \ #handles system breach messages
            tools/rlimiter \
            tools/sysmessages
}

PROJECTS*=\
    3rdparty/libraries/tar\
    settings/packagemanager \
