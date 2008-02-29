qtopia_project(qtopiacore)
QTOPIACORE_CONFIG=app rpath
qt=host
dir=tools/$$TARGET
depends(libraries/qt/*)
symlink_files.commands=$$COMMAND_HEADER
files=qvfbhdr.h qlock_p.h qlock.cpp qwssignalhandler_p.h qwssignalhandler.cpp
for(f,files) {
    dest=$$QT_DEPOT_PATH/tools/qvfb/$$f
    src=$$QT_DEPOT_PATH/src/gui/embedded/$$f
    symlink_files.commands+=\
        rm -f $$LITERAL_QUOTE$$dest$$LITERAL_QUOTE $$LINE_SEP\
        ln -s $$LITERAL_QUOTE$$src$$LITERAL_QUOTE $$LITERAL_QUOTE$$dest$$LITERAL_QUOTE $$LINE_SEP
}
symlink_files.commands+=$$DUMMY_COMMAND
QMAKE_EXTRA_TARGETS+=symlink_files
redirect_all.depends+=symlink_files
qt_qmake.depends+=symlink_files
