#ifndef QTOPIA_INPUTMATCH_GLOBAL_H
#define QTOPIA_INPUTMATCH_GLOBAL_H

#include <qglobal.h>

// The _EXPORT macros...

#if defined(QT_VISIBILITY_AVAILABLE)
#   define QTOPIA_IM_VISIBILITY __attribute__((visibility("default")))
#else
#   define QTOPIA_IM_VISIBILITY
#endif

#ifndef QTOPIA_INPUTMATCH_EXPORT
#   define QTOPIA_INPUTMATCH_EXPORT QTOPIA_IM_VISIBILITY
#endif

#endif //QTOPIA_INPUTMATCH_GLOBAL_H
