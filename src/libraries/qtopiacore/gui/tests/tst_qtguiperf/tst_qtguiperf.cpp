/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <QTest>
#include <sys/time.h>
#include <QtopiaApplication>
#include <QtGui>
#include <QTimeZoneSelector>
#include <QWorldmap>
#include <QFormLayout>
#include <shared/perftest.h>
#include <shared/qtopiaunittest.h>

//TESTED_CLASS=
//TESTED_FILES=

/*
    Performance tests for Qt GUI, especially widget construction and window layouts.
*/
class tst_QtGuiPerf : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void emptyConstructor();
    void widgetPopulation();
    void sizeHints();
    void cachedSizeHints();
    void minimumSizeHints();
    void cachedMinimumSizeHints();
    void layout();
    void painting();
};

QTEST_APP_MAIN( tst_QtGuiPerf, QtopiaApplication )
#include "tst_qtguiperf.moc"

void tst_QtGuiPerf::initTestCase()
{
    {PERF_BEGIN_TASK(1) {
        QFontMetrics *f = new QFontMetrics(qApp->font());
        f->width('w');
    } PERF_END_TASK();
    PERF_LOG("QFontDatabase");}

    {PERF_BEGIN_TASK(1) {
        QTimeZone *tz = new QTimeZone();
        tz->city();
    } PERF_END_TASK();
    PERF_LOG("QTimeZone");}
}

/*?
    Test the time taken to construct empty widgets of various types.
*/
void tst_QtGuiPerf::emptyConstructor()
{
    {PERF_BEGIN_TASK(50) {
        QWidget *w = new QWidget();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QWidget");}

    {PERF_BEGIN_TASK(50) {
        QLineEdit *w = new QLineEdit();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QLineEdit");}

    {PERF_BEGIN_TASK(50) {
        QLabel *w = new QLabel();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QLabel");}

    {PERF_BEGIN_TASK(50) {
        QSpinBox *w = new QSpinBox();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QSpinBox");}

    {PERF_BEGIN_TASK(50) {
        QAbstractSpinBox *w = new QAbstractSpinBox();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QAbstractSpinBox");}

    {PERF_BEGIN_TASK(50) {
        QDateEdit *w = new QDateEdit();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QDateEdit");}

    {PERF_BEGIN_TASK(50) {
        QTimeEdit *w = new QTimeEdit();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTimeEdit");}

    {PERF_BEGIN_TASK(50) {
        QFrame *w = new QFrame();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QFrame");}

    {PERF_BEGIN_TASK(50) {
        QCalendarWidget *w = new QCalendarWidget();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QCalendarWidget");}

    {PERF_BEGIN_TASK(50) {
        QComboBox *w = new QComboBox();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QComboBox");}

    {PERF_BEGIN_TASK(50) {
        QComboBox *w = new QComboBox();
        w->setEditable(true);
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QComboBox (Editable)");}

    {PERF_BEGIN_TASK(50) {
        QSlider *w = new QSlider();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QSlider");}

    {PERF_BEGIN_TASK(50) {
        QPushButton *w = new QPushButton();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QPushButton");}

    {PERF_BEGIN_TASK(50) {
        QRadioButton *w = new QRadioButton();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QRadioButton");}

    {PERF_BEGIN_TASK(50) {
        QCheckBox *w = new QCheckBox();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QCheckBox");}

    {PERF_BEGIN_TASK(50) {
        QScrollBar *w = new QScrollBar();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QScrollBar");}

    {PERF_BEGIN_TASK(50) {
        QTabBar *w = new QTabBar();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTabBar");}

    {PERF_BEGIN_TASK(50) {
        QTabWidget *w = new QTabWidget();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTabWidget");}

    {PERF_BEGIN_TASK(50) {
        QGroupBox *w = new QGroupBox();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QGroupBox");}

    {PERF_BEGIN_TASK(50) {
        QAbstractScrollArea *w = new QAbstractScrollArea();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QAbstractScrollArea");}

    {PERF_BEGIN_TASK(50) {
        QScrollArea *w = new QScrollArea();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QScrollArea");}

    {PERF_BEGIN_TASK(50) {
        QTextEdit *w = new QTextEdit();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTextEdit");}

    {PERF_BEGIN_TASK(50) {
        QTextBrowser *w = new QTextBrowser();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTextBrowser");}

    {PERF_BEGIN_TASK(50) {
        QListWidget *w = new QListWidget();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QListWidget");}

    {PERF_BEGIN_TASK(50) {
        QTableView *w = new QTableView();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTableView");}
}

void tst_QtGuiPerf::widgetPopulation()
{
    {QComboBox *w = new QComboBox();
    w->clear(); //to make sure the model is constructed
    PERF_BEGIN_TASK(100) {
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        w->addItem("CB Item");
        PERF_BEGIN_SKIP() {
            w->clear();
        } PERF_END_SKIP();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QComboBox addItem");}

    {QComboBox *w = new QComboBox();
    w->clear(); //to make sure the model is constructed
    PERF_BEGIN_TASK(100) {
        QStringList list;
        list << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item"
            << "CB Item";
        w->addItems(list);
        PERF_BEGIN_SKIP() {
            w->clear();
        } PERF_END_SKIP();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QComboBox addItems");}

    {QTabWidget *w = new QTabWidget();
    PERF_BEGIN_TASK(5) {
        w->addTab(new QLabel("My Tab"), "My Tab");
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTabWidget addTab");}

    {QMenu *w = new QMenu();
     QAction *a = new QAction("Action", w);
    PERF_BEGIN_TASK(100) {
        w->addAction(a);
    } PERF_END_TASK();
    delete a;
    delete w;
    PERF_LOG("QMenu addAction");}
}

void tst_QtGuiPerf::sizeHints()
{
    QWidget *w;
    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QWidget();
        } PERF_END_SKIP();
        w->sizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QWidget");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QLineEdit("Text");
        } PERF_END_SKIP();    
        w->sizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QLineEdit");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QLabel("Label");
        } PERF_END_SKIP();    
        w->sizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QLabel");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QDateEdit();
        } PERF_END_SKIP();    
        w->sizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QDateEdit");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QTimeEdit();
        } PERF_END_SKIP();    
        w->sizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTimeEdit");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QComboBox();
            ((QComboBox*)w)->addItem("CB Item");
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QComboBox");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QScrollArea();
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QScrollArea");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QCalendarWidget();
        } PERF_END_SKIP();    
        w->sizeHint();
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QCalendarWidget");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QPushButton("Button");
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QPushButton");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QRadioButton("Exclusive");
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QRadioButton");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QCheckBox("Choice");
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QCheckBox");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QTabBar();
            ((QTabBar*)w)->addTab("Tab 1");
            ((QTabBar*)w)->addTab("Tab 2");
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTabBar");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QTextEdit("Text Edit");
        } PERF_END_SKIP();    
        w->sizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTextEdit");}
}

void tst_QtGuiPerf::cachedSizeHints()
{
    QWidget *w;
    {w = new QWidget();
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QWidget");}

    {w = new QLineEdit("Text");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QLineEdit");}

    {w = new QLabel("Label");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QLabel");}

    {w = new QDateEdit();
     PERF_BEGIN_TASK(10) {
        w->sizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QDateEdit");}

    {w = new QTimeEdit();
     PERF_BEGIN_TASK(10) {
        w->sizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTimeEdit");}

    {w = new QComboBox();
     ((QComboBox*)w)->addItem("CB Item");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QComboBox");}

    {w = new QScrollArea();
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QScrollArea");}

    {w = new QCalendarWidget();
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QCalendarWidget");}

    {w = new QPushButton("Button");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QPushButton");}

    {w = new QRadioButton("Exclusive");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QRadioButton");}

    {w = new QCheckBox("Choice");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QCheckBox");}

    {w = new QTabBar();
     ((QTabBar*)w)->addTab("Tab 1");
     ((QTabBar*)w)->addTab("Tab 2");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTabBar");}

    {w = new QTextEdit("Text Edit");
     PERF_BEGIN_TASK(10) {
        w->sizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTextEdit");}
}

void tst_QtGuiPerf::minimumSizeHints()
{
    QWidget *w;
    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QWidget();
        } PERF_END_SKIP();
        w->minimumSizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QWidget");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QLineEdit("Text");
        } PERF_END_SKIP();    
        w->minimumSizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QLineEdit");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QLabel("Label");
        } PERF_END_SKIP();    
        w->minimumSizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QLabel");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QDateEdit();
        } PERF_END_SKIP();    
        w->minimumSizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QDateEdit");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QTimeEdit();
        } PERF_END_SKIP();    
        w->minimumSizeHint();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTimeEdit");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QComboBox();
            ((QComboBox*)w)->addItem("CB Item");
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QComboBox");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QScrollArea();
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QScrollArea");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QCalendarWidget();
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QCalendarWidget");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QPushButton("Button");
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QPushButton");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QRadioButton("Exclusive");
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QRadioButton");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QCheckBox("Choice");
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QCheckBox");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QTabBar();
            ((QTabBar*)w)->addTab("Tab 1");
            ((QTabBar*)w)->addTab("Tab 2");
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTabBar");}

    {PERF_BEGIN_TASK(50) {
        PERF_BEGIN_SKIP() {
            w = new QTextEdit("Text Edit");
        } PERF_END_SKIP();    
        w->minimumSizeHint();
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("QTextEdit");}
}

void tst_QtGuiPerf::cachedMinimumSizeHints()
{
    QWidget *w;
    {w = new QWidget();
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QWidget");}

    {w = new QLineEdit("Text");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QLineEdit");}

    {w = new QLabel("Label");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QLabel");}

    {w = new QDateEdit();
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QDateEdit");}

    {w = new QTimeEdit();
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();  
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTimeEdit");}

    {w = new QComboBox();
     ((QComboBox*)w)->addItem("CB Item");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QComboBox");}

    {w = new QScrollArea();
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QScrollArea");}

    {w = new QCalendarWidget();
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QCalendarWidget");}

    {w = new QPushButton("Button");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QPushButton");}

    {w = new QRadioButton("Exclusive");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QRadioButton");}

    {w = new QCheckBox("Choice");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QCheckBox");}

    {w = new QTabBar();
     ((QTabBar*)w)->addTab("Tab 1");
     ((QTabBar*)w)->addTab("Tab 2");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTabBar");}

    {w = new QTextEdit("Text Edit");
     PERF_BEGIN_TASK(10) {
        w->minimumSizeHint();
    } PERF_END_TASK();
    delete w;
    PERF_LOG("QTextEdit");}
}

void tst_QtGuiPerf::layout()
{
    {PERF_BEGIN_TASK(50) {
        QWidget *w = new QWidget();
        QVBoxLayout *v = new QVBoxLayout(w);

        for (int j = 0; j < 10; j++) {
            QHBoxLayout *h = new QHBoxLayout();
            h->addWidget(new QLabel("First Name"));
            h->addSpacing(1);
            h->addWidget(new QLineEdit());
            v->addLayout(h);
        }
        w->showMaximized();  
        PERF_BEGIN_SKIP() {
            delete w;
        } PERF_END_SKIP();
    } PERF_END_TASK();
    PERF_LOG("Box Layout");}

    {PERF_BEGIN_TASK(10) {
        QMainWindow *mw = new QMainWindow();
        mw->setWindowTitle( tr("Date/Time") );

        QWidget *timePage, *formatPage;
        QFormLayout *timeLayout, *formatLayout;

        QTabWidget *tb = new QTabWidget;

        timePage = new QWidget;
        timeLayout = new QFormLayout;
        timeLayout->setMargin(0);
        timePage->setLayout(timeLayout);

        formatPage = new QWidget;
        formatLayout = new QFormLayout;
        formatLayout->setMargin(0);
        formatPage->setLayout(formatLayout);

        tb->addTab(timePage, tr("Time"));
        tb->addTab(formatPage, tr("Format"));

        mw->setCentralWidget( tb );

        QLabel *lblZone = new QLabel(tr( "Time Zone" ));
        QTimeZoneSelector *tz = new QTimeZoneSelector;
        lblZone->setBuddy(tz->focusProxy());
        timeLayout->addRow( lblZone, tz );
        
        QLabel *timeLabel = new QLabel( tr("Time"));
        QTimeEdit *time = new QTimeEdit( QTime::currentTime() );
        timeLabel->setBuddy(time);
        timeLayout->addRow( timeLabel, time );

        QLabel *dateLabel = new QLabel( tr("Date"));
        QDateEdit *date = new QDateEdit( QDate::currentDate() );
        date->setCalendarPopup( true );
        dateLabel->setBuddy(date);
        timeLayout->addRow( dateLabel, date );

        // on to the format page/layout

        QLabel *l = new QLabel( tr("Time format") );
        QComboBox *ampmCombo = new QComboBox;
        ampmCombo->addItem( tr("24 hour") );
        ampmCombo->addItem( tr("12 hour") );
        l->setBuddy(ampmCombo);
        formatLayout->addRow( l, ampmCombo );

        int show12hr = QTimeString::currentAMPM() ? 1 : 0;
        ampmCombo->setCurrentIndex( show12hr );
        time->setDisplayFormat("h:mm ap");

        l = new QLabel( tr("Week starts" ) );
        QComboBox *weekStartCombo = new QComboBox;
        weekStartCombo->addItem( tr("Sunday") );
        weekStartCombo->addItem( tr("Monday") );
        l->setBuddy(weekStartCombo);
        formatLayout->addRow( l, weekStartCombo );
        
        int startMonday = Qtopia::weekStartsOnMonday() ? 1 : 0;
        weekStartCombo->setCurrentIndex( startMonday );

        l = new QLabel( tr("Date format" ) );
        QComboBox *dateFormatCombo = new QComboBox;
        l->setBuddy(dateFormatCombo);
        formatLayout->addRow( l, dateFormatCombo );
        
        QString df = QTimeString::currentFormat();
        QStringList date_formats = QTimeString::formatOptions();
        date_formats.prepend("loc");
        int currentdf = date_formats.indexOf(df);
        if (currentdf < 0)
            currentdf = 0;

        QStringList translated_date_formats;
        translated_date_formats.append( tr("locale", "Use the date format for the current language") );
        for (int i = 1; i< date_formats.count(); i++ ) {
            QString entry = date_formats[i];
            entry.replace( "D", tr("D", "D == day") );
            entry.replace( "M", tr("M", "M == month") );
            entry.replace( "Y", tr("Y", "Y == year") );
            translated_date_formats.append( entry );
        }

        QStringListModel *model = new QStringListModel(translated_date_formats, dateFormatCombo);
        dateFormatCombo->setModel(model);
        dateFormatCombo->setCurrentIndex( currentdf );

        mw->showMaximized();

    } PERF_END_TASK();
    PERF_LOG("Simulated Date/Time");}
}

void tst_QtGuiPerf::painting()
{
    QPicture picture;
    picture.load(":icon/addressbook/AddressBook");
    QRect bounds = picture.boundingRect();
    
    PERF_BEGIN_TASK(100) {
        QImage image = QImage(bounds.width(),bounds.height(),QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter p(&image);
        p.drawPicture(0,0, picture);        
        p.end();
    } PERF_END_TASK();
    PERF_LOG("drawPicture");
    
}
