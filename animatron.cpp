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
#include <QtDBus>
#include <QPainter>
#include <QFontMetrics>

        Plasma::Wallpaper* instance;

       Animatron:: Animatron(QObject* parent, const QVariantList& args)
       :Plasma::Wallpaper(parent, args)
{
       fprintf(stderr, "Plugin starting...\n");

       scenedt = 0.0f;
       sceners = true;
       sceneupdate = false;
       status = Ena && Act;
       ready = false;

       pConfigWidget = NULL;
       pTimer = NULL;

       connect(Ena.ptr(), SIGNAL(set(bool)), this, SLOT(hard_set(bool)));
       connect(Act.ptr(), SIGNAL(set(bool)), this, SLOT(soft_set(bool)));

#ifdef ENABLE_DBUS
       QDBusConnection::sessionBus().registerService(DBUS_DOM);
#endif
       instance = this;
}

       Animatron:: ~Animatron()
{
#ifdef ENABLE_DBUS
       QDBusConnection::sessionBus().unregisterService(DBUS_DOM);
#endif
       fprintf(stderr, "Plugin finished.\n");
}

void   Animatron:: init(const KConfigGroup& Config)
{
       fprintf(stderr, "Plugin loading...\n");

       mGlobalConfig.Font = Config.readEntry("font", mGlobalConfig.Font);
       mGlobalConfig.Color = Config.readEntry("color", mGlobalConfig.Color);
       mGlobalConfig.BackgroundColor = Config.readEntry("bgcolor", mGlobalConfig.BackgroundColor);
       mGlobalConfig.Refresh = Config.readEntry("refresh", mGlobalConfig.Refresh);
       mGlobalConfig.HorizontalRezolution = Config.readEntry("hr", mGlobalConfig.HorizontalRezolution);
       mGlobalConfig.VerticalRezolution = Config.readEntry("vr", mGlobalConfig.VerticalRezolution);
       mGlobalConfig.Bins = Config.readEntry("bins", mGlobalConfig.Bins);
       mGlobalConfig.Style = Config.readEntry("style", mGlobalConfig.Style);
       mGlobalConfig.Arrangement = Config.readEntry("arrangement", mGlobalConfig.Arrangement);
       mGlobalConfig.History = Config.readEntry("history", mGlobalConfig.History);
       mGlobalConfig.Rules = Config.readEntry("rules", mGlobalConfig.Rules);

       while (mGlobalConfig.Rules.count() < 32)
       {
              mGlobalConfig.Rules.append("#");
       }

       mGlobalConfig.spit();

       mSceneConfig.width = boundingRect().width();
       mSceneConfig.height = boundingRect().height();
       mSceneConfig.rez_x = mGlobalConfig.HorizontalRezolution;
       mSceneConfig.rez_y = mGlobalConfig.VerticalRezolution;
       mSceneConfig.spacing = 0;
       mSceneConfig.ffcount = mGlobalConfig.Bins;
       mSceneConfig.ffbase = mGlobalConfig.Base;

       scenedt  = 1.0f / (float)mGlobalConfig.Refresh;
       sceners  = true;

   if (!pTimer)
   {
       pTimer = new QTimer(this);
       connect(pTimer, SIGNAL(timeout()), this, SLOT(sync()));
   }

       pTimer->setInterval(1000 * scenedt);

   if (!mGlobalConfig.Style.isEmpty())
   {
       if (mGlobalConfig.Arrangement)
           mStyle.load(mGlobalConfig.Style);
           else
           mStyle = QImage();
   }   else
   {
       mStyle = QImage();
   }

       Ena.setEnabled(true);
       Act.reset(mGlobalConfig.Rules);

       mScene.reset(mSceneConfig);
       mSize = QSizeF(0.0f, 0.0f);
       sheet = mStyle.rect();
       copy  = QRectF(0.0f, 0.0f, 0.0f, 0.0f);
       erase = QRectF(0.0f, 0.0f, 0.0f, 0.0f);
       pTimer->start();

       ready = true;
       sceners = true;
}

void   Animatron:: save(KConfigGroup& Config)
{
       fprintf(stderr, "Plugin saving...\n");
       pTimer->stop();

       Config.writeEntry("font", mGlobalConfig.Font);
       Config.writeEntry("color", mGlobalConfig.Color);
       Config.writeEntry("bgcolor", mGlobalConfig.BackgroundColor);
       Config.writeEntry("refresh", mGlobalConfig.Refresh);
       Config.writeEntry("hr", mGlobalConfig.HorizontalRezolution);
       Config.writeEntry("vr", mGlobalConfig.VerticalRezolution);
       Config.writeEntry("bins", mGlobalConfig.Bins);
       Config.writeEntry("style", mGlobalConfig.Style);
       Config.writeEntry("arrangement", mGlobalConfig.Arrangement);
       Config.writeEntry("history", mGlobalConfig.History);
       Config.writeEntry("rules", mGlobalConfig.Rules);

       mGlobalConfig.spit();
       ready = false;
}

QWidget* Animatron::createConfigurationInterface(QWidget* parent)
{
       pConfigWidget = new ConfigWidget(parent, mGlobalConfig);
     //connect(parent, SIGNAL(destroyed(QObject*)), this, SLOT(disposeConfigurationInterface()));

       return pConfigWidget;
}

void   Animatron::updateScreenshot(QPersistentModelIndex index)
{
   if (pConfigWidget)
   {
       pConfigWidget->updateScreenshot(index);
   }
}

void   Animatron::hard_set(bool value)
{
       fprintf(stderr, "Hard b=%d\n", value);
       Act.setEnabled(value);
       mScene.setActive(value);
}

void   Animatron::soft_set(bool value)
{
       fprintf(stderr, "Soft b=%d\n", value);
       mScene.setActive(value);
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
       bool  resize = mSize != boundingRect().size();
       bool  have_image = mGlobalConfig.Arrangement != 0.0f && mStyle.isNull() == false;
       bool  need_erase = !have_image;

   if (resize)
   {
       mSize = boundingRect().size();
       mSceneConfig.width = mSize.width();
       mSceneConfig.height = mSize.height();
       mScene.reset(mSceneConfig);

       sheet.moveCenter(boundingRect().center());
   }

       copy = exposedRect;
       erase = exposedRect;

       QRectF src;

   if (have_image)
   {
       copy &= sheet;

       if (copy.width() == erase.width())
       {
           if (copy.top() > erase.top())
           {
               painter->fillRect(erase.left(), erase.top(), erase.width(), copy.top() - erase.top(), mGlobalConfig.BackgroundColor);
           }

           if (copy.bottom() < erase.bottom())
           {
               painter->fillRect(erase.left(), copy.bottom(), erase.width(), erase.bottom() - copy.bottom(), mGlobalConfig.BackgroundColor);
           }
       }   else
       {
           need_erase = true;
       }

       if (copy.height() == erase.height())
       {
           if (copy.left() > erase.left())
           {
               painter->fillRect(erase.left(), erase.top(), copy.left() - erase.left(), erase.height(), mGlobalConfig.BackgroundColor);
           }

           if (copy.right() < erase.right())
           {
               painter->fillRect(copy.right(), erase.top(), erase.right() - copy.right(), erase.height(), mGlobalConfig.BackgroundColor);
           }
       }   else
       {
           need_erase = true;
       }

       src = copy.translated(-sheet.left(), -sheet.top());

       have_image  = copy.width() * copy.height() != 0.0f;
     //need_erase &= erase.width() * erase.height() != 0.0f;
   }

   if (need_erase)
   {
       painter->fillRect(erase, mGlobalConfig.BackgroundColor);
   }

   if (have_image)
   {
       painter->drawImage(copy, mStyle, src);
   }

   if (sceners)
   {
       painter->setPen(mGlobalConfig.Color);
       painter->setFont(mGlobalConfig.Font);
       mScene.render(painter);
   }

       sceners = false;
}

K_EXPORT_PLASMA_WALLPAPER(animatron, Animatron)
#include "animatron.moc"
