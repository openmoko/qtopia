#include <qwidget.h>
#include <qpointarray.h>
#include <qregion.h>
#include <qpixmap.h>


class QVFb;
class QVFbView;
class CursorWindow;
class QTextStream;


class Skin : public QWidget
{
    Q_OBJECT
public:
    Skin( QVFb *p, const QString &skinFile, int &viewW, int &viewH );
    ~Skin( );
    void setView( QVFbView *v );
    void setZoom( double );
    bool isValid() {return skinValid;}

    bool hasCursor() const;
    static QSize screenSize(const QString &skinFile);

protected slots:
    void skinKeyRepeat();
    void moveParent();

protected:
    virtual void paintEvent( QPaintEvent * );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent * );

private:
    QVFb *parent;
    QVFbView *view;
    QPoint parentpos;
    QPoint clickPos;
    bool buttonPressed;
    int buttonIndex;
    bool skinValid;
    double zoom;
    void calcRegions();
    void flip(bool open);

    static QString skinFileName(const QString &skinFile, QString* prefix);
    static bool parseSkinFileHeader(QTextStream& ts,
		    int *viewX1, int *viewY1,
		    int *viewW, int *viewH,
		    int *numberOfAreas,
		    QString* skinImageUpFileName,
		    QString* skinImageDownFileName,
		    QString* skinImageClosedFileName,
		    QString* skinCursorFileName,
		    QPoint* cursorHot);

    void loadImages();
    QString skinImageUpFileName;
    QString skinImageDownFileName;
    QString skinImageClosedFileName;
    QString skinCursorFileName;
    QPixmap skinImageUp;
    QPixmap skinImageDown;
    QPixmap skinImageClosed;
    QPixmap skinCursor;
    int viewX1, viewY1;
    int numberOfAreas;

    typedef struct {
	QString	name;
        int	keyCode;
        QPointArray area;
        QRegion region;
	QString text;
    } ButtonAreas;

    void startPress(int);
    void endPress();

    ButtonAreas *areas;

    CursorWindow *cursorw;
    QPoint cursorHot;

    int joystick;
    bool joydown;
    QTimer *t_skinkey;
    QTimer *t_parentmove;
    int onjoyrelease;

    bool flipped_open;

    void setupDefaultButtons();
    QString prefix;
};


