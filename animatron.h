/*
    Copyright (c) 2011, spaceape [[ spaceape99@gmail.com ]]
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the organization nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ANIMATRON_HEADER
#define ANIMATRON_HEADER

#include <Plasma/Wallpaper>
#include <QTimer>
#include <QImage>
#include <QtDBus>
#include <KFileDialog>
#include "dbusettings.h"
#include "fireflies.h"
#include "ui_config.h"
using namespace std;
using namespace Ui;


class QSizeF;

//org.kde.screensaver Screensaver/
//org.kde.krunner
//org.freedesktop.upower

class Animatron: public Plasma::Wallpaper
{
           Q_OBJECT

           QDBusConnection bus;

           QColor   ctextcolor;
           QFont    ctextfont;
           QString  cstyle;
           int      crefresh;
           QSizeF   msize;

           float    scenedt;
           bool     sceners;
           bool     sceneupdate;
           bool     ready;

         //BackgroundListModel* mmodel;
           Scene    mScene;
           SceneConfig mSceneConfig;
           QImage   mStyle;
           QTimer*  pTimer;
           KFileDialog* pOpenDialog;
           void* pBrowseDialog;

           config   mConfigUi;

 private slots:
           void modified();
           void launchOpenWall();
           void dialogOpenWallOkay();
           void dialogOpenWallDone();
           void launchBrowseWall();
           void dialogBrowseWallOkay();
           void dialogBrowseWallDone();
           void sync();

           void suspend(bool);
 signals:
           void settingsChanged(bool modified);

 protected:
 virtual   void init(const KConfigGroup& config);


 public:
           Animatron(QObject* parent, const QVariantList& args);
           ~Animatron();

 virtual   void save(KConfigGroup& config);
 virtual   QWidget* createConfigurationInterface(QWidget* parent);
 virtual   void paint(QPainter* painter, const QRectF& exposedRect);

 signals:
           void statusupdate(bool);

 public slots:
           bool freeze();
           bool unfreeze();
           bool getstatus();
};

K_EXPORT_PLASMA_WALLPAPER(animatron, Animatron)
#endif
