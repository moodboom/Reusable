 /****************************************************************************
 **
 ** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial Usage
 ** Licensees holding valid Qt Commercial licenses may use this file in
 ** accordance with the Qt Commercial License Agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Nokia.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Nokia gives you certain additional
 ** rights.  These rights are described in the Nokia Qt LGPL Exception
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
 ** If you have questions regarding the use of this file, please contact
 ** Nokia at qt-info@nokia.com.
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 #ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
 #include <phonon/audiooutput.h>
 #include <phonon/seekslider.h>
 #include <phonon/mediaobject.h>
 #include <phonon/volumeslider.h>
 #include <phonon/backendcapabilities.h>
 #include <QList>

 class QAction;
 class QTableWidget;
 class QLCDNumber;


 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow();

     QSize sizeHint() const {
         return QSize(500, 300);
     }

 private slots:
     void addFiles();
     void about();
     void stateChanged(Phonon::State newState, Phonon::State oldState);
     void tick(qint64 time);
     void sourceChanged(const Phonon::MediaSource &source);
     void metaStateChanged(Phonon::State newState, Phonon::State oldState);
     void aboutToFinish();
     void tableClicked(int row, int column);

 private:
     void setupActions();
     void setupMenus();
     void setupUi();

     Phonon::SeekSlider *seekSlider;
     Phonon::MediaObject *mediaObject;
     Phonon::MediaObject *metaInformationResolver;
     Phonon::AudioOutput *audioOutput;
     Phonon::VolumeSlider *volumeSlider;
     QList<Phonon::MediaSource> sources;

     QAction *playAction;
     QAction *pauseAction;
     QAction *stopAction;
     QAction *nextAction;
     QAction *previousAction;
     QAction *addFilesAction;
     QAction *exitAction;
     QAction *aboutAction;
     QAction *aboutQtAction;
     QLCDNumber *timeLcd;
     QTableWidget *musicTable;
 };

 #endif
