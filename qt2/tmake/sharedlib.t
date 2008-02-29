# Default link type (static linking is still be used where required)
SYSCONF_LINK_LIB	= $(SYSCONF_LINK_LIB_SHARED)
SYSCONF_LINK_TARGET	= $(SYSCONF_LINK_TARGET_SHARED)
# Compiling library source
SYSCONF_CXXFLAGS_LIB	= #$ Expand('TMAKE_CXXFLAGS_SHLIB');
SYSCONF_CFLAGS_LIB	= #$ Expand('TMAKE_CFLAGS_SHLIB');
# Compiling shared-object source
SYSCONF_CXXFLAGS_SHOBJ	= #$ Expand('TMAKE_CXXFLAGS_SHLIB');
SYSCONF_CFLAGS_SHOBJ	= #$ Expand('TMAKE_CFLAGS_SHLIB');
# Linking Qt
SYSCONF_LIBS_QTLIB	= $(SYSCONF_LFLAGS_X11) $(QT_LIBS_MT) $(QT_LIBS_OPT)
# Linking Qt applications
SYSCONF_LIBS_QTAPP	= 
