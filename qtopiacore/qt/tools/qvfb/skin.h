/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SKIN_H
#define SKIN_H

#include <QWidget>
#include <QPolygon>
#include <QRegion>
#include <QPixmap>

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
    void setSecondaryView( QVFbView *v );
    void setZoom( double );
    bool isValid() {return skinValid;}

    bool hasCursor() const;
    static QSize screenSize(const QString &skinFile);
    static QSize secondaryScreenSize(const QString &skinFile);
    static bool hasSecondaryScreen(const QString &skinFile);

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
    QVFbView *secondaryView;
    QPoint parentpos;
    QPoint clickPos;
    bool buttonPressed;
    int buttonIndex;
    bool skinValid;
    double zoom;
    void calcRegions();
    void flip(bool open);
    void updateSecondaryScreen();

    static QString skinFileName(const QString &skinFile, QString* prefix);
    static void parseRect(const QString &, QRect *);
    static bool parseSkinFileHeader(QTextStream& ts,
                    QRect *screen, QRect *outsideScreen,
                    QRect *outsideScreenClosed,
		    int *numberOfAreas,
		    QString* skinImageUpFileName,
		    QString* skinImageDownFileName,
		    QString* skinImageClosedFileName,
		    QString* skinCursorFileName,
		    QPoint* cursorHot,
                    QStringList* closedAreas);

    void loadImages();
    QString skinImageUpFileName;
    QString skinImageDownFileName;
    QString skinImageClosedFileName;
    QString skinCursorFileName;
    QPixmap skinImageUp;
    QPixmap skinImageDown;
    QPixmap skinImageClosed;
    QPixmap skinCursor;
    QRect screenRect;
    QRect backScreenRect;
    QRect closedScreenRect;
    int numberOfAreas;

    typedef struct {
	QString	name;
        int	keyCode;
        QPolygon area;
        QRegion region;
	QString text;
        bool activeWhenClosed;
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

#endif
