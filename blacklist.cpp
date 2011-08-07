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
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <QPainter>
#include <qevent.h>
#include <KWindowSystem>
#include <KIcon>
#include "blacklist.h"
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

       const float DesktopList::dt;

       DesktopList::DesktopList(QWidget* parent)
       :QWidget(parent)
{
       imgDesktop.load(":/desktop.png");
       imgDeskAll.load(":/desktop-all.png");
       imgLogo.load(":/logo.png");
       mDesktopFont = QFont("DejaVu Sans", 8);
       setMinimumHeight(128);
       setMaximumHeight(128);

       mItemWidth = imgDesktop.width();
       mItemRect = QRectF(0.0f, 0.0f, mItemWidth, height() - 24.0f);

     //mSlideRect = QRectF(0.0f, 0.0f, 0.0f, 0.0f);

       pTimer = new QTimer(this);
       pTimer->setInterval(dt * 1000);
       connect(pTimer, SIGNAL(timeout()), this, SLOT(sync()));

       connect(KWindowSystem::self(), SIGNAL(desktopNamesChanged()), this, SLOT(deskNameChanged()));
       connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), this, SLOT(deskCountChanged()));

       minside = false;
       mpointer = QPointF(0.0f, 0.0f);
       mpivot = QPointF(0.0f, 0.0f);

       index = 0;
       updateitems();
       pTimer->start();

       setMouseTracking(true);
}

       DesktopList::~DesktopList()
{
}

int    DesktopList::getIndex()
{
       return index;
}

void   DesktopList::setIndex(int x)
{
   if (x < 0)
       return;

   if (x < items.size())
   {
       index = x;

       updateinterface();
       emit IndexChanged(index);
   }
}

void   DesktopList::shout()
{
       emit IndexChanged(index);
}

void   DesktopList::prev()
{
       setIndex(index - 1);
}

void   DesktopList::next()
{
       setIndex(index + 1);
}

void   DesktopList::deskNameChanged()
{
       updateitems();
}

void   DesktopList::deskCountChanged()
{
       updateitems();
}

void   DesktopList::updateitems()
{
       int   x;

       items.clear();

       if (index > KWindowSystem::numberOfDesktops())
           index = KWindowSystem::NumberOfDesktops;

       if (KWindowSystem::numberOfDesktops() > 1)
       {
           items.push_back("All Desktops");
       }

       for (x = 1; x <= KWindowSystem::numberOfDesktops(); ++x)
       {
            items.push_back(KWindowSystem::desktopName(x).toAscii().data());
       }

           mSlideWidth = items.size() * mItemWidth;

           updateinterface();
}

void   DesktopList::updateinterface()
{

   if (mSlideWidth < width())
       mpivot = QPointF((width() - mSlideWidth) / 2.0f, 0.0f);
       else
       mpivot = QPointF(width() / 2.0f - (index + 0.5f) * mItemWidth, 0.0f);

       resync = true;
}

int    DesktopList::traceitem(QPointF point)
{
       int index = floor((point.x() - mSlideOffset) / mItemWidth);

   if (index >= 0)
   {
       if (index >= items.size())
           index = -1;
   }   else
   {
       index = -1;
   }

       return index;
}

void   DesktopList::sync()
{
   if (minside)
   {
       int index = traceitem(mpointer);

       if (index >= 0)
       {
           timers[index] = 1.0f;
       }
   }

   float dx = mSlideOffset - mpivot.x();
   float p;

   if (dx != 0.0f)
   {
       mSlideOffset -= dx / 2.0f;
       resync = true;

       if (mSlideWidth > width())
       {
           if (mSlideOffset > 0.0f)
               mSlideOffset = 0.0f;

           if ((mSlideOffset + mSlideWidth) < width())
               mSlideOffset = width() - mSlideWidth;
       }
   }

       m_timer_i it = timers.begin();

       while (it != timers.end())
       {
          if (it->second > 0.0f)
          {   it->second -= dt;
              resync = true;
          }   else
          {
              it->second = 0.0f;
          }

           ++it;
       }

   if (resync)
   {
       update();
   }
}

void   DesktopList::enterEvent(QEvent* ev)
{
       minside = true;
}

void   DesktopList::leaveEvent(QEvent* ev)
{
       minside = false;
}

void   DesktopList::mouseMoveEvent(QMouseEvent* ev)
{
       mpointer = ev->pos();
}

void   DesktopList::mousePressEvent(QMouseEvent* ev)
{
   if (ev->buttons() & Qt::LeftButton)
   {
       int trace = traceitem(ev->pos());

       if (trace >= 0)
       {
           if (index != trace)
           {
               setIndex(trace);
           }
       }
   }
}

void   DesktopList::mouseReleaseEvent(QMouseEvent* )
{
}

void   DesktopList::resizeEvent(QResizeEvent* )
{
       updateinterface();
}

void   DesktopList::paintEvent(QPaintEvent*)
{
       QPainter dev(this);
       dev.setClipRect(geometry());
       dev.fillRect(geometry(), Qt::black);
       dev.drawImage(width() - 256, 0, imgLogo);

       dev.setPen(Qt::white);
       dev.drawText(QRect(4, height() - 24, width() - 8, 24), Qt::AlignVCenter, items[index].c_str());

       int ix = 0;
//       int ix = floor(-mSlideOffset / mItemWidth); //

       if (ix < 0)
       {
           ix = 0;
       }

       QRectF ir = mItemRect;

       ir.moveTo(mSlideOffset, 0);

       dev.setFont(mDesktopFont);
       dev.setPen(Qt::white);

       while (ix < items.size())
       {
          if (ix == index)
          {
              dev.fillRect(QRect(ir.left(), ir.top(), ir.width(), 4), Qt::white);
              dev.setOpacity(0.5f);
              dev.fillRect(ir, QColor::fromRgb(0, 0, 255));
              dev.drawRect(ir);
          }

              m_timer_i it = timers.find(ix);

          if (it != timers.end())
          {
              dev.setOpacity(it->second);
              dev.fillRect(QRect(ir.left(), ir.bottom() - 4, ir.width(), 4), Qt::red);
          }

              dev.setOpacity(1.0f);

          if (ix)
          {
              dev.drawImage(ir.x(), 8, imgDesktop);
              dev.drawText(ir, Qt::AlignCenter, QString().setNum(ix));
          }   else
          {
              dev.drawImage(ir.x(), 8, imgDeskAll);
             //dev.drawText(ir, Qt::AlignCenter, QString("@"));
          }

              ir.translate(mItemWidth, 0);

          if (ir.x() > width())
          {
              break;
          }

            ++ix;
       }

       resync = false;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       DesktopMenu::DesktopMenu(DesktopList* pController)
       :QMenu(NULL)
{
       controller = pController;
       connect(this, SIGNAL(aboutToShow()), this, SLOT(_onshow()));
       connect(this, SIGNAL(aboutToHide()), this, SLOT(_onhide()));
}

       DesktopMenu::~DesktopMenu()
{
}

void   DesktopMenu::_onshow()
{
       showing();
}

void   DesktopMenu::_onhide()
{
       hiding();
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       DesktopRule::DesktopRule(DesktopMenu* pMenu, QString qActionText, QIcon qActionIcon, QString qRuleText)
       :QAction(qActionIcon, qActionText, NULL)
{
       menu = pMenu;
       rule = qRuleText;

       connect(this, SIGNAL(triggered(bool)), this, SLOT(_hit()));
}

       DesktopRule::~DesktopRule()
{
       printf("Rule %p destroyed\n", this);
}


void   DesktopRule::_hit()
{
       menu->hit(rule.toAscii().data());
}


// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       AddRuleMenu::AddRuleMenu(DesktopList* pController)
       :DesktopMenu(pController)
{
}

       AddRuleMenu::~AddRuleMenu()
{
}

void   AddRuleMenu::hit(char* const text)
{
       emit perform(QString(text));
}

void   AddRuleMenu::showing()
{
       int  x;

     //dispose previous items
//        try
//        {
//            for (x = 0; x != actions().count(); ++x)
//            {
//                delete actions().at(x);
//            }
//        }
//        catch(...)
//        {
//            fprintf(stderr, "[AddRuleMenu:%p]: Algorithm `dispose previous items` crashed :(\n");
//        }

       clear();


     //populate menu
       QList<WId> list = KWindowSystem::windows();

       int  desktop = controller->getIndex();
       WId  id;
       KWindowInfo* info;

       static int window_supported_types = NET::NormalMask | NET::DialogMask | NET::OverrideMask | NET::UtilityMask |
                  NET::DesktopMask | NET::DockMask | NET::TopMenuMask | NET::SplashMask |
                  NET::ToolbarMask | NET::MenuMask;

       for (x = 0; x != list.count(); ++x)
       {
            id   = list.at(x);
            info = new KWindowInfo(id, NET::WMDesktop | NET::WMWindowType, NET::WM2WindowClass);

            if (info->windowType(window_supported_types) == NET::Normal)
                if (!desktop || info->desktop() == desktop)
                    addAction(new DesktopRule(this, info->windowClassName(), QIcon(KWindowSystem::icon(id)), info->windowClassName()));
       }

       addSeparator();
       addAction(new DesktopRule(this, "Always disabled", QIcon(), "*"));
       addAction(new DesktopRule(this, "Always enabled", QIcon(), "#"));
}

void   AddRuleMenu::hiding()
{
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       RemRuleMenu::RemRuleMenu(DesktopList* pController)
       :DesktopMenu(pController)
{
}

       RemRuleMenu::~RemRuleMenu()
{
}

void   RemRuleMenu::hit(char* const text)
{
       controller->context_message("Then maybe wait for the next release");
       emit perform(QString(text));
}


void   RemRuleMenu::showing()
{
       clear();
       addAction(new DesktopRule(this, "Too lazy to edit the rules by hand?", KIcon("emoticon"), ""));
}

void   RemRuleMenu::hiding()
{
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
#include "blacklist.moc"
