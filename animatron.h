#ifndef animatron_h
#define animatron_h
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
#include <QTimer>
#include <QImage>
#include <Plasma/Wallpaper>
#include <Plasma/Package>

#include "config.h"
#include "fireflies.h"
#include "switches.h"
using namespace std;

class QSizeF;
class QRectF;

class Animatron: public Plasma::Wallpaper
{
           Q_OBJECT

           GlobalConfig mGlobalConfig;
           SceneConfig mSceneConfig;

           Scene mScene;
           KillSwitch  Ena;
           DesktopSwitch Act;

           QImage   mStyle;
           QRectF   sheet;
           QRectF   copy;
           QRectF   erase;

           QSizeF   mSize;

           float    scenedt;
           bool     sceners;
           bool     sceneupdate;
           bool     status;
           bool     ready;

           ConfigWidget* pConfigWidget;
           QTimer*  pTimer;

 private slots:
           void hard_set(bool);
           void soft_set(bool);
           void sync();

 protected:
 virtual   void init(const KConfigGroup& config);

 public:
           Animatron(QObject* parent, const QVariantList& args);
           ~Animatron();
           //void enableHardLocks();
           //void disableHardLocks();

 virtual   void save(KConfigGroup& config);
 virtual   QWidget* createConfigurationInterface(QWidget* parent);
           void updateScreenshot(QPersistentModelIndex index);
 virtual   void paint(QPainter* painter, const QRectF& exposedRect);
};
#endif
