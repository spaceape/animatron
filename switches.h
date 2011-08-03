#ifndef switches_h
#define switches_h
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
#include <KWindowInfo>
#include <map>
#include <vector>
#include <string>
using namespace std;

#include <QtDBus>
#ifdef ENABLE_DBUS
#define DBUS_DOM "org.plasmaabuse.animatron"
#define DBUS_ORG "/"
#endif

class KillSwitch :public QObject
{
           Q_OBJECT
           QDBusConnection bus;

           bool power;
           bool enabled;

 public:
           KillSwitch();
 virtual   ~KillSwitch();

           bool getPower();
           bool setPower(bool);
           bool getEnabled();
           void setEnabled(bool);

           KillSwitch* ptr() {return this;}
           operator bool() {return power;}
           bool operator=(bool rhs) {return setPower(rhs);}

 public slots:
           bool setSuspended(bool);
           bool isSuspended();
           bool freeze();
           bool unfreeze();
 signals:
           void set(bool);
};

class DesktopSwitch :public QObject
{
           Q_OBJECT

           QStringList pMatchList;
 typedef   map<WId, KWindowInfo*> m_windows_t;
 typedef   m_windows_t::iterator m_windows_i;

           m_windows_t  windows;

           bool enabled;
           bool rehash;
           bool suspendall;
           bool suspendcurrent;

           void hash();
           void clear();
           bool query(int pdesktop, char* const prules);
           bool sync();

 private slots:
           void deskNameChanged();
           void deskCountChanged();
           void deskSwitched(int);

           void wndAdded(WId);
           void wndRemoved(WId);
           void wndActiveChanged(WId);

 public:
           DesktopSwitch();
 virtual   ~DesktopSwitch();

           void reset(QStringList& config);
           bool getEnabled();
           void setEnabled(bool);

           DesktopSwitch* ptr() {return this;}
           operator bool() {return suspendall | suspendcurrent;}
 signals:
           void set(bool);
};
#endif
