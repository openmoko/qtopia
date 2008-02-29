

#include <qurl.h>
#include <qtopia/qpeapplication.h>
#include "kvnc.h"

int main( int argc, char **argv )
{
  QPEApplication app( argc, argv );
  KVNC *view = new KVNC( "Keypebble" );
  app.showMainWidget( view );

  if ( argc > 1 )
    view->openURL( QUrl(argv[1]) );

  return app.exec();
}

