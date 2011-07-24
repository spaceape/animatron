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
#include <QtCore/QTimer>
#include "ui_config.h"
#include "fireflies.h"
using namespace std;
using namespace Ui;


class QSizeF;

class Animatron: public Plasma::Wallpaper
{
           Q_OBJECT

           QColor   ctextcolor;
           QFont    ctextfont;
           int      crefresh;
           QSizeF   msize;

           float    scenedt;
           bool     sceners;
           bool     sceneupdate;

         //BackgroundListModel* mmodel;
           Scene    mScene;
           SceneConfig mSceneConfig;
           QTimer*  pTimer;

           config   mConfigUi;

 protected:
 virtual   void init(const KConfigGroup& config);

 protected slots:
           void modified();
           void sync();

 public:
           Animatron(QObject* parent, const QVariantList& args);
           ~Animatron();

 virtual   void save(KConfigGroup& config);
 virtual   QWidget* createConfigurationInterface(QWidget* parent);
 virtual   void paint(QPainter* painter, const QRectF& exposedRect);

 signals:
           void settingsChanged(bool modified);
};

K_EXPORT_PLASMA_WALLPAPER(animatron, Animatron)
#endif
