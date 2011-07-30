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
#include "animatron.h"
#include <QtGui/QPainter>
#include <QtGui/QFontMetrics>
#include <kfiledialog.h>
#include <knewstuff3/downloaddialog.h>

       Animatron:: Animatron(QObject* parent, const QVariantList& args)
       :Plasma::Wallpaper(parent, args),
        bus(QDBusConnection::sessionBus())
{
       fprintf(stderr, "Plugin starting...\n");

       scenedt = 0.0f;
       sceners = false;
       sceneupdate = false;
       active = false;
       enabled = false;
       ready = false;

       pTimer = NULL;
       pImageListModel = NULL;
       pOpenDialog = NULL;
       pBrowseDialog = NULL;

   if (bus.registerService(DBUS_DOM))
   {
       bus.registerObject(DBUS_ORG, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
       bus.connect("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "ActiveChanged", this, SLOT( suspend(bool) ));
     //bus.connect("org.freedesktop.PowerManagement", "/org/freedesktop/PowerManagement", "org.freedesktop.PowerManagement", "PowerSaveStatusChanged", this, SLOT( suspend(bool) ));
       fprintf(stderr, "Connected to DBUS\n");
   }

       unfreeze();
}


       Animatron:: ~Animatron()
{
       bus.unregisterObject(DBUS_ORG);
       bus.unregisterService(DBUS_DOM);
       fprintf(stderr, "Plugin finished.\n");
}

void   Animatron::updateScreenshot(QPersistentModelIndex index)
{
       mConfigUi.ImageList->update(index);
}

void   Animatron:: init(const KConfigGroup& config)
{
       fprintf(stderr, "Plugin loading...\n");
       mSceneConfig.width = boundingRect().width();
       mSceneConfig.height = boundingRect().height();
       mSceneConfig.rez_x = config.readEntry("xrez", 64);
       mSceneConfig.rez_y = config.readEntry("yrez", 48);
       mSceneConfig.spacing = 0;
       mSceneConfig.ffcount = config.readEntry("count", 8);
       mSceneConfig.ffbase = config.readEntry("base", 16);
       ctextcolor = config.readEntry("color", QColor::fromRgb(24, 255, 0));
       ctextfont = config.readEntry("font", QFont("Courier", 8));
       cstyle = config.readEntry("style");
       carrange = config.readEntry("arrange", -1);
       cuserstyles = config.readEntry("userstyles", QStringList());
       crefresh = 25;

       scenedt  = 1.0f / (float)crefresh;
       sceners  = true;

   if (!pTimer)
   {
       pTimer = new QTimer(this);
       connect(pTimer, SIGNAL(timeout()), this, SLOT(sync()));
   }

   if (!cstyle.isEmpty())
   {
       mStyle.load(cstyle);
   }

       mScene.reset(mSceneConfig);
       pTimer->setInterval(1000 * scenedt);
       pTimer->start();

#ifdef DEBUG
       fprintf(stderr, "style=`%s` arrange=%d\n", cstyle.toAscii().data(), carrange);
#endif

       ready = true;
       emit SettingsChanged(false);
       emit update(boundingRect());
}

void   Animatron:: save(KConfigGroup& config)
{
       fprintf(stderr, "Plugin saving...\n");
       pTimer->stop();

       config.writeEntry("xrex", mSceneConfig.rez_x);
       config.writeEntry("yrez", mSceneConfig.rez_y);
       config.writeEntry("count", mSceneConfig.ffcount);
       config.writeEntry("base", mSceneConfig.ffbase);
       config.writeEntry("font", ctextfont);
       config.writeEntry("color", ctextcolor);
       config.writeEntry("style", cstyle);
       config.writeEntry("arrange", carrange);
       config.writeEntry("userstyles", cuserstyles);

       ready = false;
}

QWidget* Animatron::createConfigurationInterface(QWidget* parent)
{
       QWidget* widget = new QWidget(parent);
       mConfigUi.setupUi(widget);
       connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(disposeConfigurationInterface()));

       int  x;

       mConfigUi.FontChooser->setFont(ctextfont);
       connect(mConfigUi.FontChooser, SIGNAL(fontSelected(QFont)), this, SLOT(modified()));

       mConfigUi.ColorBtn->setColor(ctextcolor);
       connect(mConfigUi.ColorBtn, SIGNAL(changed(QColor)), this, SLOT(modified()));

     //mConfigUi.RefreshCombo->setValue(crefresh);
     //connect(mConfigUi.RefreshCombo, SIGNAL(valueChanged(int)), this, SLOT(modified()));

       mConfigUi.WidthSpinner->setValue(mSceneConfig.rez_x);
       connect(mConfigUi.WidthSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));

       mConfigUi.HeightSpinner->setValue(mSceneConfig.rez_y);
       connect(mConfigUi.HeightSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));

       mConfigUi.CountSpinner->setValue(mSceneConfig.ffcount);
       connect(mConfigUi.CountSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));

       mConfigUi.BaseCombo->setEditText(QString::number(mSceneConfig.ffbase));

       for (x = 0; x < mConfigUi.BaseCombo->count(); ++x) 
       {
            if (mSceneConfig.ffbase == mConfigUi.BaseCombo->itemText(x).toInt())
            {
                mConfigUi.BaseCombo->setCurrentIndex(x);
                break;
            }
       }
       connect(mConfigUi.BaseCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));

       pImageListModel = new BackgroundListModel(this, widget);
       pImageListModel->setResizeMethod((ResizeMethod)carrange);
       pImageListModel->setWallpaperSize(boundingRect().size().toSize());
       pImageListModel->reload(cuserstyles);

       QTimer::singleShot(250, this, SLOT(wallFetchList()));
       mConfigUi.ImageList->setItemDelegate(new BackgroundDelegate(mConfigUi.ImageList));

//        mConfigUi.ImageList->setMinimumWidth(
//           (BackgroundDelegate::SCREENSHOT_SIZE + BackgroundDelegate::MARGIN * 2 + BackgroundDelegate::BLUR_INCREMENT) * 3 +
//            mConfigUi.ImageList->spacing() * 4 + 
//            QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 
//            QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2 + 7
//        );

       mConfigUi.ImageList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
       connect(mConfigUi.ImageList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(modified()));

       mConfigUi.ImageList->setSpacing(2);
       mConfigUi.ImageList->setViewMode(QListView::IconMode);

       RemoveButtonManager* rbmanager = new RemoveButtonManager(mConfigUi.ImageList, &cuserstyles);
       connect(rbmanager, SIGNAL(removeClicked(QString)), this, SLOT(wallRemoveItem(QString)));

       //mConfigUi.ArrangementCombo->addItem(i18n("Scaled & Cropped"), ScaledAndCroppedResize);
       //mConfigUi.ArrangementCombo->addItem(i18n("Scaled"), ScaledResize);
       //mConfigUi.ArrangementCombo->addItem(i18n("Scaled, keep proportions"), MaxpectResize);
       mConfigUi.ArrangementCombo->addItem(i18n("Centered"), CenteredResize);
       //mConfigUi.ArrangementCombo->addItem(i18n("Tiled"), TiledResize);
       //mConfigUi.ArrangementCombo->addItem(i18n("Center Tiled"), CenterTiledResize);
       mConfigUi.ArrangementCombo->addItem(i18n("No Background"), 0);

       for (x = 0; x < mConfigUi.ArrangementCombo->count(); ++x) 
       {
            if (carrange == mConfigUi.ArrangementCombo->itemData(x).value<int>())
            {
                mConfigUi.ArrangementCombo->setCurrentIndex(x);
                break;
            }
       }
       connect(mConfigUi.ArrangementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(wallArrangementChanged(int)));

       mConfigUi.OpenWallBtn->setIcon(KIcon("document-open"));
       connect(mConfigUi.OpenWallBtn, SIGNAL(clicked()), this, SLOT(wallInsertItem()));

       mConfigUi.GetNewWallBtn->setIcon(KIcon("get-hot-new-stuff"));
       connect(mConfigUi.GetNewWallBtn, SIGNAL(clicked()), this, SLOT(wallBrowseItem()));

       connect(this, SIGNAL(SettingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
       return widget;
}

void   Animatron::disposeConfigurationInterface()
{
   if (pOpenDialog)
   {
       delete pOpenDialog;
       pOpenDialog = NULL;
   }

   if (pBrowseDialog)
   {
       delete pBrowseDialog;
       pBrowseDialog = NULL;
   }

       pImageListModel = NULL;
}

void   Animatron::wallInsertItem()
{
   if (!pOpenDialog)
   {
       pOpenDialog = new KFileDialog(KUrl(), QString::fromAscii( "*.png *.jpeg *.jpg *.xcf *.svg *.svgz *.bmp" ), NULL);
       pOpenDialog->setOperationMode(KFileDialog::Opening);
       pOpenDialog->setInlinePreviewShown(true);
       pOpenDialog->setCaption(QString::fromAscii("Select wallpaper image..."));
       pOpenDialog->setModal(false);
       connect(pOpenDialog, SIGNAL(okClicked()), this, SLOT(wallInsertDialogOkay()));
       connect(pOpenDialog, SIGNAL(destroyed(QObject*)), this, SLOT(wallInsertDialogDone()));
   }

       pOpenDialog->show();
       pOpenDialog->raise();
       pOpenDialog->activateWindow();
}

void   Animatron::wallInsertDialogOkay()
{
       Q_ASSERT(pImageListModel);

   const QFileInfo info(pOpenDialog->selectedFile());
   const QString   pathname = info.canonicalFilePath();

   if (!pathname.isEmpty())
   {
            QModelIndex   index;

        if (!pImageListModel->contains(pathname))
        {
            pImageListModel->addBackground(pathname);
            cuserstyles << pathname;
        }

            index = pImageListModel->indexOf(pathname);

        if (index.isValid())
        {
            mConfigUi.ImageList->setCurrentIndex(index);
        }
   }

        wallImageChanged(pathname);
}

void   Animatron::wallInsertDialogDone()
{
}

void   Animatron::wallBrowseItem()
{
   if (!pBrowseDialog)
   {
        pBrowseDialog = new KNS3::DownloadDialog("animatron.knsrc", NULL);
        connect(pBrowseDialog, SIGNAL(accepted()), SLOT(wallBrowseDialogOkay()));
        connect(pBrowseDialog, SIGNAL(destroyed()), SLOT(wallBrowseDialogDone()));
   }

        pBrowseDialog->show();
}

void   Animatron::wallBrowseDialogOkay()
{
       pImageListModel->reload();
       emit SettingsChanged(true);
}

void   Animatron::wallBrowseDialogDone()
{
}

void   Animatron::wallRemoveItem(QString pathname)
{
   int index = cuserstyles.indexOf(pathname);

#ifdef DEBUG
       printf("Removing image `%s`@%d\n", pathname.toAscii().data(), index);
#endif

   if (index >= 0)
   {    cuserstyles.removeAt(index);
        pImageListModel->reload(cuserstyles);
        emit SettingsChanged(true);
   }
}

void   Animatron::wallFetchList()
{
       mConfigUi.ImageList->setModel(pImageListModel);
       connect(mConfigUi.ImageList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
               this, SLOT(wallIndexChanged(const QModelIndex&)));

       QModelIndex index = pImageListModel->indexOf(cstyle);

   if  (index.isValid())
   {
        mConfigUi.ImageList->setCurrentIndex(index);
   }
}

void   Animatron::wallIndexChanged(const QModelIndex& index)
{
    if (index.row() >= 0)
    {
        Plasma::Package* package = pImageListModel->package(index.row());

        if (package->structure()->contentsPrefixPaths().isEmpty())
            wallImageChanged(package->filePath("preferred"));
            else
            wallImageChanged(package->path());
    }
}

void   Animatron::wallImageChanged(QString pathname)
{
   if (pathname != cstyle)
   {
       cstyle = pathname;
       emit SettingsChanged(true);
   }

#ifdef DEBUG
       fprintf(stderr, "Changed background image to `%s`...\n", pathname.toAscii().data());
#endif
}

void   Animatron::wallArrangementChanged(int value)
{
       carrange = mConfigUi.ArrangementCombo->itemData(value).value<int>();
#ifdef DEBUG
       fprintf(stderr, "Arrangement changed to `%s`, value==%d...\n", mConfigUi.ArrangementCombo->itemText(value).toAscii().data(), carrange);
#endif
}

void   Animatron::modified()
{
       ctextfont = mConfigUi.FontChooser->font();
       ctextcolor = mConfigUi.ColorBtn->color();
     //crefresh = mConfigUi.RefreshCombo->text().toInt();

       mSceneConfig.rez_x = mConfigUi.WidthSpinner->value();
       mSceneConfig.rez_y = mConfigUi.HeightSpinner->value();
       mSceneConfig.ffcount = mConfigUi.CountSpinner->value();
       mSceneConfig.ffbase = mConfigUi.BaseCombo->currentText().toInt();

       emit SettingsChanged(true);
}

void   Animatron::suspend(bool value)
{
   if (value)
       freeze();
       else
       unfreeze();
}

void   Animatron::freeze()
{
   if (enabled)
   {
#ifdef DEBUG
       printf("FROZEN\n");
#endif

       mScene.setLockFlags(0);
       emit statusupdate(enabled = false);
   }
}

void   Animatron::unfreeze()
{
   if (!enabled)
   {
#ifdef DEBUG
       printf("UNFROZEN\n");
#endif

       mScene.setLockFlags(Scene::ENA_ALL);
       emit statusupdate(enabled = true);
   }
}

bool   Animatron::getstatus()
{
       return enabled;
}

void   Animatron::sync()
{
  if  (sceners |= mScene.sync(scenedt))
  {
       emit( update(boundingRect()) );
  }
}

void   Animatron:: paint(QPainter* painter, const QRectF& exposedRect)
{
   if (msize != boundingRect().size())
   {
       msize  = boundingRect().size();
       mSceneConfig.width = msize.width();
       mSceneConfig.height = msize.height();
       mScene.reset(mSceneConfig);
   }

   if (sceners)
   {
        //blit the background (saves all the per-pixel-products that blending does)
          painter->setCompositionMode(QPainter::CompositionMode_Source);

       if (!carrange || mStyle.width() < boundingRect().width() || mStyle.height() < boundingRect().height())
           painter->fillRect(exposedRect, Qt::black);

       if (carrange)
           if (!mStyle.isNull())
               painter->drawImage((boundingRect().width() - mStyle.width()) / 2, (boundingRect().height() - mStyle.height()) / 2, mStyle);

           painter->setPen(ctextcolor);
           painter->setFont(ctextfont);
           mScene.render(painter);

           sceners = false;
   }
}

K_EXPORT_PLASMA_WALLPAPER(animatron, Animatron)
#include "animatron.moc"
