// -*- c++ -*-

#ifndef KRFBBUFFER_H 
#define KRFBBUFFER_H 

#include <qobject.h>

class QPixmap;
class QImage;
class KRFBDecoder;
class QMouseEvent;
class QKeyEvent;

/**
 * A wrapper around QPixmap that knows how to implement the RFB
 * drawing primitives. If possible it makes use of the MIT XSHM
 * extension to optimise the drawing operations.
 */
class KRFBBuffer : public QObject
{
  Q_OBJECT

public:
  KRFBBuffer( KRFBDecoder *decoder, QObject *parent, const char *name=0 );
  ~KRFBBuffer();

  void paint( QPainter *p, int x, int y, int w, int h ) const;

  /**
   * Draw a chunk of a raw encoded rectangle.
   */
  void drawRawRectChunk( void *data, int x, int y, int w, int h );

  void copyRect( int srcX, int srcY,
                 int destX, int destY, int w, int h );

  void fillRect( int ,int , int , int , unsigned long );

  void resize( int w, int h );

  void mouseEvent( QMouseEvent *e );

  void keyPressEvent( QKeyEvent *e );
  void keyReleaseEvent( QKeyEvent *e );

  void soundBell();
  void setScaling( int s );

  void updateDone( int x, int y, int w, int h );

signals:
  /**
   * Emitted when the size of the buffer changes.
   */
  void sizeChanged( int w, int h );

  void updated( int x, int y, int w, int h );

  void bell();

protected:
    QImage fastScale( QImage &si ) const;

private:
  KRFBDecoder *decoder;
  QImage *buffer;
  int scaler;
};

#endif // KRFBBUFFER_H 
