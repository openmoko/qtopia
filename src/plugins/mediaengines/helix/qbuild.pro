requires(contains(QTOPIAMEDIA_ENGINES,helix))
TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=helix

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

QTOPIA*=media video
MODULES*=helix

include(helix.pro)

depends_load_dependencies()
SOURCEPATH+=\
    $$HELIX_PATH/common/util\
    $$HELIX_PATH/video/vidutil
SOURCES+=\
    HXErrorCodeStrings.c\
    colormap.c
SOURCE_DEPENDS_RULES+=$$HELIX_BUILD_RULE

dbg=$$HELIX_OUT_DIR
DEFINES+=CONFIG_H_FILE=$$define_string($$HELIX_PATH/$$dbg/makefile_ribodefs.h)

