#ifndef blacklist_h
#define blacklist_h
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
#include <QObject>
#include <QWidget>
#include <QStringList>
#include <QTimer>
#include <QtDBus>
#include <KWindowInfo>
#include <map>
#include <vector>
#include <string>
using namespace std;

class DesktopList :public QWidget
{
           Q_OBJECT

           QImage imgDesktop;
           QImage imgDeskAll;
           QFont mDesktopFont;
           float mItemWidth;
           float mSlideOffset;
           float mSlideWidth;
           QRectF mItemRect;

 typedef vector<string> v_desktop_t;
 typedef v_desktop_t::iterator v_desktop_i;

           v_desktop_t items;

           bool   minside;
           QPointF mpointer;
           QPointF mpivot;

 typedef map<int, float> m_timer_t;
 typedef m_timer_t::iterator m_timer_i;

           m_timer_t timers;

 static const float dt = 0.05;

           QTimer* pTimer;
           bool resync;

           int  index;

 private slots:
           void deskNameChanged();
           void deskCountChanged();
           void sync();

 protected:
           void updateitems();
           void updateinterface();
           void updatepivot();
           int  traceitem(QPointF);

           void enterEvent(QEvent*);
           void leaveEvent(QEvent*);
           void mouseMoveEvent(QMouseEvent*);
           void mousePressEvent(QMouseEvent*);
           void mouseReleaseEvent(QMouseEvent*);
           void resizeEvent(QResizeEvent*);
           void paintEvent(QPaintEvent*);

 public:
          DesktopList(QWidget* parent);
 virtual  ~DesktopList();

           int getIndex();
           void shout();

 public slots:
 signals:
           void IndexChanged(int);
};

#endif
