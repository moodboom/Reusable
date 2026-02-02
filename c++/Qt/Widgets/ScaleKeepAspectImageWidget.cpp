// Copyright (c) 2011 A better Software
// Original code taken from here: 
// http://doc.trolltech.com/4.7/desktop-screenshot.html
// Original license:

 /****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QResizeEvent>

#include "Qt/Widgets/ScaleKeepAspectImageWidget.h"

ScaleKeepAspectImageWidget::ScaleKeepAspectImageWidget(QWidget * parent)
:
    // Call base class.
    inherited(parent)
{
    m_imageLabel = new QLabel;
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding,
                                QSizePolicy::Expanding);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setMinimumSize(10, 10);

    mainLayout = new QVBoxLayout;

    // This tightens things up - do we need a constructor param?
    mainLayout->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(m_imageLabel);
    setLayout(mainLayout);

    m_imageLabel->hide();
}


void ScaleKeepAspectImageWidget::SetImagePixmap(QPixmap& pm)
{
    m_imageLabel->show();
    originalPixmap = pm;
    updateLabel();
}


void ScaleKeepAspectImageWidget::ClearImagePixmap()
{
    m_imageLabel->hide();
}


bool ScaleKeepAspectImageWidget::b_has_image_pixmap()
{
    return m_imageLabel->isVisible();
}


void ScaleKeepAspectImageWidget::resizeEvent(QResizeEvent* event)
{
    QSize scaledSize = originalPixmap.size();
    scaledSize.scale(m_imageLabel->size(), Qt::KeepAspectRatio);

    // Qt 6: QLabel::pixmap() returns QPixmap by value, not pointer
    QPixmap currentPixmap = m_imageLabel->pixmap();
    if (currentPixmap.isNull() || scaledSize != currentPixmap.size()) {

        // DEBUG
        // We were recursing here over and over at one point on the Mac!
        // Then it stopped, wtf.
        /*
        QPixmap debugPixmap = m_imageLabel->pixmap();
        if (!debugPixmap.isNull())
        {
            QSize current_size = debugPixmap.size();
            int macdebuggersucks = 234;
        }
        */

        updateLabel();
    }

	inherited::resizeEvent(event);
}

void ScaleKeepAspectImageWidget::updateLabel()
{
    m_imageLabel->setPixmap(
        originalPixmap.scaled(m_imageLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation)
    );
}
