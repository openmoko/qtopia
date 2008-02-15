// Added by Trolltech.

#include "sipdebug.h"
#include <stdarg.h>
#include <string.h>
#include <qsettings.h>

static int debugEnabled(void)
{
    static int debugMode = -1;
    if ( debugMode == -1 ) {
        QSettings config( "Trolltech", "Log" );
        config.beginGroup( "Sip" );
        debugMode = config.value( "Enabled", false ).toBool();
    }
    return debugMode;
}

void sip_printf( const char *format, ... )
{
    if ( debugEnabled() ) {
        va_list va;
        va_start( va, format );
        if ( strncmp( format, "Sip", 3 ) != 0 )
            printf( "Sip : " );
        vprintf( format, va );
        va_end( va );
    }
}

void sip_perror( const char *msg )
{
    if ( debugEnabled() )
        perror( msg );
}
