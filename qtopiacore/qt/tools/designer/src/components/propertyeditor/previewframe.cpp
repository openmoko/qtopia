/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::PreviewWorkspace
*/

#include "previewframe.h"
#include "previewwidget.h"

#include <QtGui/QPainter>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QPaintEvent>
#include <qdebug.h>

namespace {
    class PreviewMdiArea: public QMdiArea {
    public:
        PreviewMdiArea(QWidget *parent = 0) : QMdiArea(parent) {}
    protected:
        bool viewportEvent ( QEvent * event );
    };

    bool PreviewMdiArea::viewportEvent (QEvent * event) {
        if (event->type() != QEvent::Paint)
            return QMdiArea::viewportEvent (event);
        QWidget *paintWidget = viewport();
        QPainter p(paintWidget);
        p.fillRect(rect(), paintWidget->palette().color(backgroundRole()).dark());
        p.setPen(QPen(Qt::white));
        p.drawText(0, height() / 2,  width(), height(), Qt::AlignHCenter,
                   tr("The moose in the noose\nate the goose who was loose."));
        return true;
    }
}

namespace qdesigner_internal {

PreviewFrame::PreviewFrame(QWidget *parent) :
    QFrame(parent),
    m_mdiArea(new PreviewMdiArea(this))
{
    m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    vbox->addWidget(m_mdiArea);

    setMinimumSize(ensureMdiSubWindow()->minimumSizeHint());
}

void PreviewFrame::setPreviewPalette(const QPalette &pal)
{
    ensureMdiSubWindow()->widget()->setPalette(pal);
}

void PreviewFrame::setSubWindowActive(bool active)
{
    m_mdiArea->setActiveSubWindow (active ? ensureMdiSubWindow() : static_cast<QMdiSubWindow *>(0));
}

QMdiSubWindow *PreviewFrame::ensureMdiSubWindow()
{
    if (!m_mdiSubWindow) {
        PreviewWidget *previewWidget = new PreviewWidget(m_mdiArea);
        m_mdiSubWindow = m_mdiArea->addSubWindow(previewWidget, Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
        m_mdiSubWindow->move(10,10);
        m_mdiSubWindow->showMaximized();
    }

    const Qt::WindowStates state = m_mdiSubWindow->windowState();
    if (state & Qt::WindowMinimized)
        m_mdiSubWindow->setWindowState(state & ~Qt::WindowMinimized);

    return m_mdiSubWindow;
}
}
