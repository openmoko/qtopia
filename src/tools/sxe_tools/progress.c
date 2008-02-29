/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define TERMWIDTH 80

/*
   Some shell programs produce too much output, but if we silence them
   altogether it looks like the program has hung.  So use the stdout
   to provide a progress indicator of dots.

   If there is an argument, interpret it as an expected number of lines
   so that the dots will not overflow an 80 column tty
*/
int main( int argc, char *argv[] )
{
    char buf[1024];
    int dotcount = 10;  /* default to 800 lines */
    int dots = 0;
    FILE *log_strm = 0;
    int line_end;
    char *err_ptr = 0;
    if ( argc > 1 && ( strcmp( "/dev/null", argv[1] ) != 0 ))
    {
        log_strm = fopen( argv[1], "a" );
        if ( log_strm == 0 )
            perror( argv[0] );
    }
    if ( argc > 2 )
    {
        int expected_lines = atoi( argv[2] );
        if ( expected_lines > TERMWIDTH )
        {
            dotcount = expected_lines / TERMWIDTH;
        }
    }
    while (fgets( buf, 1024, stdin ) != NULL )
    {
        buf[1023] = '\0';
        if ( log_strm )
            fputs( buf, log_strm );
        if (( err_ptr = strstr( buf, "Error" )))
        {
            line_end = strlen( buf ) - 10;
            if ( strstr( buf, "make" ) == buf
                    && ( err_ptr - buf ) > line_end )
            {
                if ( log_strm )
                    fclose( log_strm );
                fprintf( stderr, "error\n" );
                exit( 1 );
            }
        }
        if ( dots == 0 )
        {
            putchar('.');
            fflush( stdout );
            dots = dotcount;
        }
        dots--;
    }
    if ( log_strm )
        fclose(log_strm);
    printf( "done\n" );
    return 0;
}
