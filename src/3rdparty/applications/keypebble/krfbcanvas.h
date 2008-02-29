// -*- c++ -*-

#ifndef KRFBCANVAS_H
#define KRFBCANVAS_H

#include <qscrollview.h>
#include <qurl.h>

class KRFBConnection;
class QTimer;

/**
 * Displays data from an KRFBDecoder, and sends events to the
 * KRFBConnection.
 */
class KRFBCanvas : public QScrollView
{
  Q_OBJECT
public:
  KRFBCanvas( QWidget *parent, const char *name=0 );
  ~KRFBCanvas();

  void setConnection( KRFBConnection * );
  KRFBConnection *connection() { return connection_; };
  void setViewScale( int s );

public slots:
  void openConnection();
  void openURL( const QUrl & );
  void closeConnection();
  void passwordRequired( KRFBConnection * );

  void refresh();
  void bell();

protected:
  virtual void keyPressEvent( QKeyEvent * );
  virtual void keyReleaseEvent( QKeyEvent * );
  virtual void contentsMousePressEvent( QMouseEvent * );
  virtual void contentsMouseReleaseEvent( QMouseEvent * );
  virtual void contentsMouseMoveEvent( QMouseEvent * );

  virtual void viewportPaintEvent( QPaintEvent *e );

protected slots:
  void loggedIn();
  void viewportUpdate( int x, int y, int w, int h );
  void clipboardChanged();
  void doAutoScroll();
  void updateSizeChanged(int, int);

private:
  KRFBConnection *connection_;
  QString password;
  bool loggedIn_;
  int dx;
  int dy;
  QMouseEvent *autoMouseEvent;
  QTimer *autoTimer;
};

#endif // KRFBCANVAS_H
