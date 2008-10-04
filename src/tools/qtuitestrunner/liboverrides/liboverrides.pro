TARGET=qtuitestoverrides
SOURCES = \
        overrides.c

!qbuild{
    CONFIG *= no_tr

    qtopia_project(embedded lib)
    CONFIG+=no_singleexec
    idep(LIBS+=-l$$TARGET)
    idep(CONFIG+=no_singleexec)
}

