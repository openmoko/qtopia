# Compiling application source
SYSCONF_CXXFLAGS	= #$ Expand('TMAKE_CXXFLAGS'); Expand('TMAKE_CXXFLAGS_RELEASE'); Expand('TMAKE_CXXFLAGS_WARN_ON'); if ( Config("qt") && !Config("debug") ) { $text.=' -DNO_DEBUG'; };
SYSCONF_CFLAGS		= #$ Expand('TMAKE_CFLAGS'); Expand('TMAKE_CFLAGS_RELEASE'); Expand('TMAKE_CFLAGS_WARN_ON');
