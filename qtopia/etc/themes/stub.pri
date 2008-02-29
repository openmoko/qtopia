!builtin_theme {
    include($$(QPEDIR)/src/.qmake.cache)
    TARGET		= theme
    QMAKE_POST_LINK = @true
    win32:QMAKE_POST_LINK = echo
    QMAKE_LINK	= @true
    win32:QMAKE_LINK = echo
}

