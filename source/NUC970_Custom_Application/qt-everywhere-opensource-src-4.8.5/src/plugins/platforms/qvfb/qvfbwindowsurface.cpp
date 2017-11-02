/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qvfbwindowsurface.h"
#include "qvfbintegration.h"
#include <QtCore/qdebug.h>
#include <QtGui/qpainter.h>
#include <private/qapplication_p.h>

QT_BEGIN_NAMESPACE

QVFbWindowSurface::QVFbWindowSurface(//QVFbIntegration *graphicsSystem,
                                     QVFbScreen *screen, QWidget *window)
    : QWindowSurface(window),
      mScreen(screen)
{
}

QVFbWindowSurface::~QVFbWindowSurface()
{
}

QPaintDevice *QVFbWindowSurface::paintDevice()
{
    return mScreen->screenImage();
}

void QVFbWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    Q_UNUSED(offset);

//    QRect rect = geometry();
//    QPoint topLeft = rect.topLeft();

    mScreen->setDirty(region.boundingRect());
}

void QVFbWindowSurface::resize(const QSize&)
{

// any size you like as long as it's full-screen...

    QRect rect(mScreen->availableGeometry());
    QWindowSurface::resize(rect.size());
}


QVFbWindow::QVFbWindow(QVFbScreen *screen, QWidget *window)
    : QPlatformWindow(window),
      mScreen(screen)
{
}


void QVFbWindow::setGeometry(const QRect &)
{

// any size you like as long as it's full-screen...

    QRect rect(mScreen->availableGeometry());
    QWindowSystemInterface::handleGeometryChange(this->widget(), rect);

    QPlatformWindow::setGeometry(rect);
}



QT_END_NAMESPACE
