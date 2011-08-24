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
#include <cstdio>
#include <cmath>
#include "fireflies.h"
#include "tweak.h"
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       const uint Thingy::DETAIL_SKEL;
       const uint Thingy::DETAIL_FULL;

       const uint Thingy::ENA_ALL;
       const uint Thingy::ENA_SPAWNING;

       QRect Thingy::bounds;
       QRect Thingy::screen;
       QSize Thingy::cell;
       int  Thingy::space;
       uint  Thingy::detail;
       uint  Thingy::base;
       uint  Thingy::lock = Thingy::ENA_ALL;


// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       const int Scene::MAX_FIREFLIES;
       const int Scene::MAX_CELLS_X;
       const int Scene::MAX_CELLS_Y;

       Scene::Scene()
{
       width = 0;
       height = 0;
       screen.setWidth(64);
       screen.setHeight(48);

       count = 0;
       resync = false;
       enabled = false;

       lock = ENA_ALL;
       seed();
}

       Scene::~Scene()
{
       int x;

       for (x = 0; x != count; ++x)
       {
            delete fireflies[x];
       }
}

void   Scene::reset(SceneConfig& config)
{
       width = config.width;
       height = config.height;
       screen.setWidth(config.rez_x);
       screen.setHeight(config.rez_y);
       space = 1;
       base  = config.ffbase;
     //lock  = ENA_ALL;

       cell.setWidth(width / screen.width() - space);
       cell.setHeight(height / screen.height() - space);

       int dx = width  % screen.width();
       int dy = height % screen.height();

       bounds.setLeft(dx / 2);
       bounds.setTop(dy / 2);
       bounds.setWidth(cell.width() * screen.width());
       bounds.setHeight(cell.height() * screen.height());

       int x = 0;

       while(x != count)
       {
             fireflies[x++]->reset();
       }

       while(count < config.ffcount)
       {
             fireflies[count++] = new Firefly(this);
       }

       while(count > config.ffcount)
       {
             delete fireflies[--count];
       }

   if (cell.width() >= 5 && cell.height() >= 7)
       detail = DETAIL_FULL;
       else
       detail = DETAIL_SKEL;

       enabled = width >= screen.width() && height >= screen.height();
       resync = enabled;
}

uint   Scene::getLockFlags()
{
       return lock;
}

void   Scene::setLockFlags(uint ulock)
{
       lock = ulock;
}


bool   Scene::getActive()
{
       return lock != 0;
}

void   Scene::setActive(bool uactive)
{
   if (uactive)
       lock = ENA_ALL;
       else
       lock = 0;
}

bool   Scene::sync(float dt)
{
   if (enabled)
   {
       int x = 0;

       while(x != count)
       {
             resync |= fireflies[x++]->sync(dt);
       }
   }

       return resync;
}

void   Scene::render(QPainter* dev)
{
   if (enabled)
   {
       //dev->save();

       int x = 0;

       while(x != count)
       {
             fireflies[x++]->render(dev);
       }

       //dev->restore();
   }

       resync = false;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       Cell::Cell(Firefly* pparent, char* const psym, float flifetime)
       :Thingy()
{
       parent = pparent;
       initialize(0, 0, psym);
       lifetime = flifetime;
       life = flifetime;
}

       Cell::Cell(const Cell& psrc)
       :Thingy()
{
       parent = psrc.parent;
       initialize(psrc.position.x(), psrc.position.y(), psrc.symbol);
       lifetime = psrc.lifetime;
       life = psrc.life;
}

       Cell::~Cell()
{
}

void   Cell::initialize(int px, int py, char* const psym)
{
       position.setX(px);
       position.setY(py);
       evolution.setX(0);
       evolution.setY(0);
       life = 0.0f;
       lifetime = 0.0f;

       symbol   = strdup(" ");
       resync   = true;

   if (psym)
       copy(psym);
       else
       spinup();

       prev  = NULL;
       next  = NULL;

       parent->_notify_cell_created(this);
}

void   Cell::dispose()
{
       free(symbol);
}

void   Cell::spinup()
{
       int   k = rand() % base;

  if  (k < 0x0a)
  {
       symbol[0] = 0x30 + k;
  }    else
  {
       if (k < 0x2a)
       {
           symbol[0] = 0x37 + k;
       }   else
       {
           symbol[0] = 0x43 + k;
       }
  }
}

void   Cell::copy(char* const src)
{
       symbol[0] = src[0];
}

void   Cell::move()
{
   if (!evolution.isNull())
   {   spawn();
       spinup();
       position += evolution;
       life = lifetime;
   }
}

Cell* Cell::spawn()
{
       Cell* ret = new Cell(*this);
       return ret;
}

void   Cell::explode()
{
       Cell* c1 = spawn(); c1->setXEvo(+1.0f); c1->setYEvo(0.0f); c1->setLife(CELL_EXPLODE_LIFETIME);
       Cell* c2 = spawn(); c2->setXEvo(-1.0f); c2->setYEvo(0.0f); c2->setLife(CELL_EXPLODE_LIFETIME);
       Cell* c3 = spawn(); c3->setXEvo(0.0f); c3->setYEvo(+1.0f); c3->setLife(CELL_EXPLODE_LIFETIME);
       Cell* c4 = spawn(); c4->setXEvo(0.0f); c4->setYEvo(-1.0f); c4->setLife(CELL_EXPLODE_LIFETIME);
}

bool   Cell::sync(float dt)
{
       resync = false;
       life -= dt;

   if (life <= 0.0f)
   {
       parent->_notify_cell_expired(this);

       return  resync;
   }

   if (screen.contains(position))
   {
       move();
       resync = true;
   }

       return resync;
}

void   Cell::render(QPainter* dev)
{
   if (resync)
   {
       QRect r(bounds.left() + position.x() * (cell.width() + space),
               bounds.top() + position.y() * (cell.height() + space),
               cell.width(),
               cell.height()
       );

       dev->setOpacity(life / lifetime);

       switch(detail)
       {
         case DETAIL_FULL:
         {
              dev->drawText(r, Qt::AlignCenter, symbol, &r);
         }
              break;

         case DETAIL_SKEL:
         {
              dev->drawRect(r);
         }
              break;

       }

       resync = false;
   }
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

       Firefly::Firefly(Scene* pscene)
{
       scene  = pscene;
       head   = NULL;
       tail   = NULL;
       cc     = 0;

       timer  = 0.0f;
       resync = true;

       reset();
}

       Firefly::~Firefly()
{
       reset();
}

void   Firefly::reset()
{
       Cell* cnt = tail;

       while(cnt)
       {
            _notify_cell_expired(cnt);
             cnt = cnt->prev;
       }

       resync = true;
}

bool   Firefly::sync(float dt)
{
   if (lock & ENA_SPAWNING)
   {
       if (timer == 0.0f)
       {
           Cell* c1 = new Cell(this, NULL, CELL_G_LIFETIME);
                 c1->setXEvo(CELL_X_EVO);
                 c1->setYEvo(CELL_Y_EVO);

                 c1->setX(rndint(screen.width() / 4, screen.width() / 2, -screen.width() / 4));
                 c1->setY(rndint(screen.height()));
       }

           timer  += dt;

       if (timer >= TIME_RECYCLE)
       {
           if (head)
           {
               head->explode();
           }

           timer  = 0.0f;
       }
   }

       Cell *current = head, *next;

       while(current)
       {
             next    = current->next;
             resync |= current->sync(dt);
             current = next;
       }

       return resync;
}

void   Firefly::render(QPainter* dev)
{
       Cell* current = head;

       while(current)
       {
             current->render(dev);
             current = current->next;
       }

       resync = false;
}

void   Firefly::_notify_cell_created(Cell* pcell)
{
   if (!head)
       head = pcell;

   if (tail)
       tail->next = pcell;

       pcell->prev = tail;

       tail = pcell;

     ++cc;
}

void   Firefly::_notify_cell_expired(Cell* pcell)
{
   if (pcell->prev)
       pcell->prev->next = pcell->next;
       else
       head = pcell->next;

   if (pcell->next)
       pcell->next->prev = pcell->prev;
       else
       tail = pcell->prev;

       delete pcell;

     --cc;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
