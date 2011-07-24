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
#ifndef FIREFLIES_H
#define FIREFLIES_H
#include <cstdlib>
#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <vector>

using namespace std;

struct SceneConfig
{
           int width;
           int height;
           int rez_x;
           int rez_y;
           int spacing;
           int ffcount;
           int ffbase;
};

class Thingy
{
 public:
 static    const uint DETAIL_SKEL = 0;
 static    const uint DETAIL_FULL = 1;

 public:
 virtual   bool sync(float dt) = 0;
 virtual   void render(QPainter* dev) = 0;

 protected:
 static    QRect bounds;
 static    QRect screen;
 static    QSize cell;
 static    int   space;
 static    uint  detail;
 static    uint  base;
};

class Cell;
class Firefly;

class Scene :public Thingy
{
 public:
 static const int MAX_FIREFLIES = 64;
 static const int MAX_CELLS_X = 256;
 static const int MAX_CELLS_Y = 64;

 private:
           int width;
           int height;

           Firefly* fireflies[MAX_FIREFLIES];
           int  count;
           bool resync;
           bool enabled;

 public:
           Scene();
 virtual   ~Scene();
           void reset(SceneConfig& config);

           bool sync(float dt);
           void render(QPainter* dev);
};

class Cell :public Thingy
{
           Firefly* parent;
           QPoint position;
           QPoint evolution;
           float life;

           float lifetime;
           char* symbol;

 private:
           void initialize(int px, int py, char* const psym);
           void dispose();
           void spinup();
           void copy(char* const);
 public:
           Cell*  prev;
           Cell*  next;

 public:
           Cell(Firefly* pparent, char* const psym, float flifetime);
           Cell(const Cell&);
 virtual   ~Cell();

           void setLifetime(int nlifetime);
           Cell* spawn();
           void move();
           void explode();
           int  getX() {return position.x();}
           void setX(int value) {position.setX(value);}
           int  getY() {return position.y();}
           void setY(int value) {position.setY(value);}
           int  getXEvo() {return evolution.x();}
           void setXEvo(int value) {evolution.setX(value);}
           int  getYEvo() {return evolution.y();}
           void setYEvo(int value) {evolution.setY(value);}
           float getLife() {return life;}
           void setLife(float value) {lifetime = value; life = value;}

           bool sync(float dt);
           void render(QPainter* dev);
};

class Firefly :public Thingy
{
           Scene* scene;
           Cell* head;
           Cell* tail;
           uint  cc;

           float timer;
           bool  resync;
 public:
           Firefly(Scene* pscene);
 virtual   ~Firefly();

           void reset();
           bool sync(float dt);
           void render(QPainter* dev);

           void _notify_cell_created(Cell*);
           void _notify_cell_expired(Cell*);
};

#endif
