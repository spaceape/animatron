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
#include "blacklist.h"
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

       const float DesktopList::dt;

       DesktopList::DesktopList(QWidget* parent)
       :QWidget(parent)
{
       mDesktopImage.load(":/desktop.png");
       mDesktopFont = QFont("DejaVu Sans", 8);
       setMinimumHeight(128);
       setMaximumHeight(128);

       mItemSize = QSize(mDesktopImage.width() + 2, height());
       mItemRect = QRect(1, 0, mDesktopImage.width(), height() - 24);

       mBoardSize = QSizeF(0.0f, 0.0f);
       mBoardOffset = QPointF(0.0f, 0.0f);

       pTimer = new QTimer(this);
       pTimer->setInterval(dt * 1000);
       connect(pTimer, SIGNAL(timeout()), this, SLOT(sync()));

       connect(KWindowSystem::self(), SIGNAL(desktopNamesChanged()), this, SLOT(deskNameChanged()));
       connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), this, SLOT(deskCountChanged()));

       minside = false;
       mpointer = QPoint(0, 0);

       index = 0;
       updateitems();
       pTimer->start();

       setMouseTracking(true);
       fprintf(stderr, "DesktopList: size=(%d, %d) | itemsize=(%d, %d)", width(), height(), mItemSize.width(), mItemSize.height());
}

       DesktopList::~DesktopList()
{
}

int    DesktopList::getIndex()
{
       return index;
}

void   DesktopList::shout()
{
       emit IndexChanged(index);
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

       items.push_back("All");

       for (x = 1; x <= KWindowSystem::numberOfDesktops(); ++x)
       {
            items.push_back(KWindowSystem::desktopName(x).toAscii().data());
       }

       updateinterface();
}

void   DesktopList::updateinterface()
{
       mBoardSize = QSizeF(mItemSize.width() * items.size(), 0.0f);

   if (mBoardSize.width() < width())
   {
       mBoardOffset = QPointF((width() - mBoardSize.width()) / 2.0f, 0.0f);
   }

       resync = true;
}

int    DesktopList::traceitem(QPoint point)
{
       int index = floor( (point.x() - mBoardOffset.x()) / mItemSize.width() );

   if (index > 0)
       if (index > items.size())
           index = -1;

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
               index = trace;
               resync = true;

               emit IndexChanged(index);
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

       dev.setPen(Qt::white);
       dev.drawText(QRect(4, height() - 24, width() - 8, 24), Qt::AlignVCenter, items[index].c_str());

       int ix = 0;
       QRect ir = mItemRect;

       ir.moveTo(mBoardOffset.x(), 0);

       dev.setFont(mDesktopFont);
       dev.setPen(Qt::white);

       while (ix < items.size())
       {
          if (ir.right() > 0.0f)
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
              dev.drawImage(ir.x(), 8, mDesktopImage);
              dev.drawText(ir, Qt::AlignCenter, QString().setNum(ix));
              ir.translate(mItemSize.width(), 0);
          }

          if (ir.left() > width())
          {
              break;
          }

            ++ix;
       }

       resync = false;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
#include "blacklist.moc"
