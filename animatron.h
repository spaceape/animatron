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
#include <QtDBus>
#include <Plasma/Wallpaper>
#include <Plasma/Package>

#include "fireflies.h"
#include "ui_config.h"
#include "backgroundlist.h"
using namespace std;
using namespace Ui;


class QSizeF;
class KFileDialog;
namespace KNS3 {
class DownloadDialog;
}

#define DBUS_DOM "org.plasmaabuse.animatron"
#define DBUS_ORG "/"

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
           int      carrange;
           QStringList cuserstyles;
           int      crefresh;
           QSizeF   msize;

           float    scenedt;
           bool     sceners;
           bool     sceneupdate;

           bool     active;
           bool     enabled;
           bool     ready;

           Scene    mScene;
           SceneConfig mSceneConfig;
           QImage   mStyle;
           config   mConfigUi;

           QTimer*  pTimer;
           BackgroundListModel* pImageListModel;
           KFileDialog* pOpenDialog;
           KNS3::DownloadDialog* pBrowseDialog;

 signals:
           void SettingsChanged(bool);

 private slots:
           void wallInsertItem();
           void wallInsertDialogOkay();
           void wallInsertDialogDone();
           void wallBrowseItem();
           void wallBrowseDialogOkay();
           void wallBrowseDialogDone();
           void wallRemoveItem(QString);
           void wallFetchList();
           void wallIndexChanged(const QModelIndex&);
           void wallImageChanged(QString);
           void wallArrangementChanged(int);
           void modified();

           void disposeConfigurationInterface();
           void suspend(bool); //hardlock

           void sync();

 protected:
 virtual   void init(const KConfigGroup& config);

 public:
           Animatron(QObject* parent, const QVariantList& args);
           ~Animatron();
           void updateScreenshot(QPersistentModelIndex index);

 virtual   void save(KConfigGroup& config);
 virtual   QWidget* createConfigurationInterface(QWidget* parent);
 virtual   void paint(QPainter* painter, const QRectF& exposedRect);

 signals:
           void statusupdate(bool);

 public slots:
           void freeze();
           void unfreeze();
           bool getstatus();
};

#endif
