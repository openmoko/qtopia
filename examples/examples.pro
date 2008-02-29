qtopia_project(subdirs)

SUBDIRS+=\
    application

!phone:SUBDIRS+=\
    inputmethod/composing\
    inputmethod/popup

