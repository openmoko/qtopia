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

#ifndef Q3WIZARD_H
#define Q3WIZARD_H

#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class QHBoxLayout;
class Q3WizardPrivate;

class Q_COMPAT_EXPORT Q3Wizard : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( QFont titleFont READ titleFont WRITE setTitleFont )

public:
    Q3Wizard( QWidget* parent=0, const char* name=0, bool modal=false, Qt::WindowFlags f=0 );
    ~Q3Wizard();

    void setVisible(bool);

    void setFont( const QFont & font );

    virtual void addPage( QWidget *, const QString & );
    virtual void insertPage( QWidget*, const QString&, int );
    virtual void removePage( QWidget * );

    QString title( QWidget * ) const;
    void setTitle( QWidget *, const QString & );
    QFont titleFont() const;
    void setTitleFont( const QFont & );

    virtual void showPage( QWidget * );

    QWidget * currentPage() const;

    QWidget* page( int ) const;
    int pageCount() const;
    int indexOf( QWidget* ) const;

    virtual bool appropriate( QWidget * ) const;
    virtual void setAppropriate( QWidget *, bool );

    QPushButton * backButton() const;
    QPushButton * nextButton() const;
    QPushButton * finishButton() const;
    QPushButton * cancelButton() const;
    QPushButton * helpButton() const;

    bool eventFilter( QObject *, QEvent * );

public Q_SLOTS:
    virtual void setBackEnabled( QWidget *, bool );
    virtual void setNextEnabled( QWidget *, bool );
    virtual void setFinishEnabled( QWidget *, bool );

    virtual void setHelpEnabled( QWidget *, bool );

    // obsolete
    virtual void setFinish(  QWidget *, bool ) {}

protected Q_SLOTS:
    virtual void back();
    virtual void next();
    virtual void help();

Q_SIGNALS:
    void helpClicked();
    void selected( const QString& );

protected:
    virtual void layOutButtonRow( QHBoxLayout * );
    virtual void layOutTitleRow( QHBoxLayout *, const QString & );

private:
    void setBackEnabled( bool );
    void setNextEnabled( bool );

    void setHelpEnabled( bool );

    void setNextPage( QWidget * );

    void updateButtons();

    void layOut();

    Q3WizardPrivate *d;

    Q_DISABLE_COPY(Q3Wizard)
};

QT_END_HEADER

#endif // Q3WIZARD_H
