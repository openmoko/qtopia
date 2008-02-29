/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qmainwindow.h>
#include <qstringlist.h>

class QVFbView;
class QVFbRateDialog;
class QPopupMenu;
class QMenuData;
class QFileDialog;
class Config;
class Skin;
class QVFb;
class QLabel;

class Zoomer : public QWidget {
    Q_OBJECT
public:
    Zoomer(QVFb* target);

private slots:
    void zoom(int);

private:
    QVFb *qvfb;
    QLabel *label;
};

class QVFb: public QMainWindow
{
    Q_OBJECT
public:
    QVFb( int display_id, int w, int h, int d, int r, const QString &skin, bool useMap, QWidget *parent = 0,
		const char *name = 0, uint wflags = 0 );
    ~QVFb();

    void enableCursor( bool e );
    void popupMenu();

    QSize sizeHint() const;

protected slots:
    void saveImage();
    void toggleAnimation();
    void toggleCursor();
    void changeRate();
    void about();

    void configure();
    void skinConfigChosen(int i);
    void chooseSize(const QSize& sz);

    void setZoom1();
    void setZoom2();
    void setZoom3();
    void setZoom4();
    void setZoomHalf();
    void setZoom075();

    void setZoom();

public slots:
    void setZoom(double);

protected:
    void createMenu(QMenuData *menu);

private:
    void findSkins(const QString &currentSkin);
    void init( int display_id, int w, int h, int d, int r, const QString& skin, bool useMap );
    Skin *skin;
    double skinscaleH,skinscaleV;
    QVFbView *view;
    QVFbRateDialog *rateDlg;
    QFileDialog* imagesave;
    QPopupMenu *viewMenu;
    int cursorId;
    Config* config;
    QStringList skinnames;
    QStringList skinfiles;
    int currentSkinIndex;
    Zoomer *zoomer;

private slots:
    void setGamma400(int n);
    void setR400(int n);
    void setG400(int n);
    void setB400(int n);
    void updateGammaLabels();
};

