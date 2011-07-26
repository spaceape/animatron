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

       Animatron:: Animatron(QObject* parent, const QVariantList& args)
       :Plasma::Wallpaper(parent, args)
{
       fprintf(stderr, "Plugin starting...\n");

       scenedt = 0.04f;
       sceners = false;
       sceneupdate = false;

       pTimer = NULL;
       pOpenDialog = NULL;
       pBrowseDialog = NULL;
}


       Animatron:: ~Animatron()
{
       fprintf(stderr, "Plugin finished.\n");
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

     //crefresh = config.readEntry("refresh", 25);
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
       config.writeEntry("refresh", crefresh);
       config.writeEntry("style", cstyle);
}

QWidget* Animatron::createConfigurationInterface(QWidget* parent)
{
       QWidget* cw = new QWidget(parent);
       mConfigUi.setupUi(cw);

       mConfigUi.FontChooser->setFont(ctextfont);
       mConfigUi.ColorBtn->setColor(ctextcolor);
     //mConfigUi.RefreshCombo->setValue(crefresh);
       mConfigUi.WidthSpinner->setValue(mSceneConfig.rez_x);
       mConfigUi.HeightSpinner->setValue(mSceneConfig.rez_y);
       mConfigUi.CountSpinner->setValue(mSceneConfig.ffcount);
       mConfigUi.BaseCombo->setEditText(QString::number(mSceneConfig.ffbase));

       connect(mConfigUi.FontChooser, SIGNAL(fontSelected(QFont)), this, SLOT(modified()));
       connect(mConfigUi.ColorBtn, SIGNAL(changed(QColor)), this, SLOT(modified()));
       connect(mConfigUi.RefreshCombo, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       connect(mConfigUi.WidthSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       connect(mConfigUi.HeightSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       connect(mConfigUi.CountSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       connect(mConfigUi.BaseCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));
       connect(mConfigUi.OpenWallBtn, SIGNAL(clicked()), this, SLOT(launchOpenWall()));
       connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
       return cw;
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

       emit settingsChanged(true);
       sceners |= true;
}

void   Animatron::launchOpenWall()
{
   if (!pOpenDialog)
   {
       pOpenDialog = new KFileDialog(KUrl(), QString::fromAscii( "*.png *.jpeg *.jpg *.xcf *.svg *.svgz *.bmp" ), NULL);
       pOpenDialog->setOperationMode(KFileDialog::Opening);
       pOpenDialog->setInlinePreviewShown(true);
       pOpenDialog->setCaption(QString::fromAscii("Select wallpaper image..."));
       pOpenDialog->setModal(false);
       connect(pOpenDialog, SIGNAL(okClicked()), this, SLOT(dialogOpenWallOkay()));
       connect(pOpenDialog, SIGNAL(destroyed(QObject*)), this, SLOT(dialogOpenWallDone()));
   }

       pOpenDialog->show();
       pOpenDialog->raise();
       pOpenDialog->activateWindow();
}

void   Animatron::dialogOpenWallOkay()
{
   if (!pOpenDialog)
       return;

       cstyle = pOpenDialog->selectedFile();

   if (mStyle.load(cstyle))
       fprintf(stderr, "Image loaded successfuly.\n");
       else
       fprintf(stderr, "Failed to load image\n");

       emit settingsChanged(true);
}

void   Animatron::dialogOpenWallDone()
{

}

void   Animatron::launchBrowseWall()
{
}

void   Animatron::dialogBrowseWallOkay()
{

}

void   Animatron::dialogBrowseWallDone()
{

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

     //blit the background (saves all the per-pixel-products that blending does)
       painter->setCompositionMode(QPainter::CompositionMode_Source);

   if (mStyle.width() < boundingRect().width() || mStyle.height() < boundingRect().height())
       painter->fillRect(exposedRect, Qt::black);

   if (!mStyle.isNull())
       painter->drawImage((boundingRect().width() - mStyle.width()) / 2, (boundingRect().height() - mStyle.height()) / 2, mStyle);

       painter->setPen(ctextcolor);
       painter->setFont(ctextfont);
       mScene.render(painter);

       sceners = false;
}

#include "animatron.moc"
