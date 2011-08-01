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
#include "switches.h"

       KillSwitch::KillSwitch()
       :bus(QDBusConnection::sessionBus())
{
       power = true;
       enabled = false;

       bus.registerObject("/KillSwitch", this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
       bus.connect("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "ActiveChanged", this, SLOT(remote_status_update(bool)));
       bus.connect("org.freedesktop.PowerManagement", "/org/freedesktop/PowerManagement", "org.freedesktop.PowerManagement", "PowerSaveStatusChanged", this, SLOT(remote_status_update(bool)));

       fprintf(stderr, "Connected to DBUS\n");
       setPower(true);
}

       KillSwitch::~KillSwitch()
{
       bus.disconnect("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver", "ActiveChanged", this, SLOT(enable(bool)));
       bus.disconnect("org.freedesktop.PowerManagement", "/org/freedesktop/PowerManagement", "org.freedesktop.PowerManagement", "PowerSaveStatusChanged", this, SLOT( suspend(bool) ));
}

bool   KillSwitch::setSuspended(bool value)
{
       return setPower(!value) != value;
}

bool   KillSwitch::freeze()
{
       return setPower(false) == false;
}

bool   KillSwitch::unfreeze()
{
       return setPower(true);
}

bool   KillSwitch::getPower()
{
       return power;
}

bool   KillSwitch::setPower(bool value)
{
   if (enabled)
   {
       if (power != value)
       {
           if (power = value)
               fprintf(stderr, "UNFROZEN\n");
               else
               fprintf(stderr, "FROZEN\n");

               emit set(power);
       }
   }

       return power;
}

bool   KillSwitch::getEnabled()
{
       return enabled;
}

void   KillSwitch::setEnabled(bool value)
{
   if (enabled != value)
   {
       if (enabled = value)
       {
           if (power)
           {
               emit set(true);
           }

           fprintf(stderr, "KS online\n");
       }   else
       {
           if (power)
           {
               emit set(false);
           }

           fprintf(stderr, "KS offline\n");
       }
   }
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       DesktopSwitch::DesktopSwitch()
{
       enabled = false;
       rehash  = true;
       suspendall = false;
       suspendcurrent = false;
}

       DesktopSwitch::~DesktopSwitch()
{
       clear();
}

void   DesktopSwitch::hash()
{
   if (rehash)
   {
       QList<WId> list = KWindowSystem::windows();
       clear();

       int  x;
       WId  id;
       KWindowInfo* info;

       for (x = 0; x != list.count(); ++x)
       {
            id   = list.at(x);
            info = new KWindowInfo(id, NET::WMDesktop, NET::WM2WindowClass);

            windows[id] = info;
       }

       rehash = !enabled;
   }
}

void   DesktopSwitch::clear()
{
       m_windows_i it = windows.begin();

       while (it != windows.end())
       {
              delete it->second;
            ++it;
       }

       windows.clear();
       rehash = true;
}

/* application match query
 * <blah> -> true if exact match (yes, that means neither globbing, nor regexps work);
 *           multiple terms are separated by ';';
 *           spaces are ignored;
 * # or blank -> always false; everything following '#' is ignored
 * * -> always true; everything following '*' is ignored
 * ?[<count>] -> true if number of windows on <desktop> >= <count>
 * 
 * multiple terms are separated by ';'
 * <desktop> == 0 -> matches on all desktops
*/

bool   DesktopSwitch::query(int desktop, char* const pattern)
{
       int e = 0;
       int mode_count = 0;
     //int mode_match

       hash();

       bool ret = false;
       char* rule;
       char *sequence_p = strdup(pattern), *sequence = sequence_p;

       while(rule = strtok(sequence, " ;"))
       {
             switch(rule[0])
             {
               case 0: return false;
               case '#': return false;
               case '*': return true;
               case '?': mode_count = 1 + strtoul(rule + 1, NULL, 0);
             }

             int  hits = 0;
             int  x;

             if  (mode_count)
             {
                  mode_count -= 1;
             }

             m_windows_i it = windows.begin();

             while (it != windows.end())
             {
                if (!desktop || it->second->desktop() == desktop)
                {
                   ++hits;

                     if (mode_count)
                     {
                         if (hits >= mode_count)
                         {   ret = true;
                             break;
                         }
                     }   else
                     {
                         if (strcasecmp(it->second->windowClassName().data(), rule) == 0)
                         {
                             ret = true;
                             break;
                         }
                     }
                }

                   ++it;
             }

             if (ret)
             {
                 break;
             }

             sequence = NULL;
       }

       free(sequence_p);
       return ret;
}

bool   DesktopSwitch::sync()
{
       int k = pMatchList.count();

       while(k < KWindowSystem::numberOfDesktops() + 1)
       {
             pMatchList.append("#");
           ++k;
       }

       suspendall = query(0,  pMatchList.at(0).toAscii().data());

       suspendcurrent = suspendall ? false : query(KWindowSystem::currentDesktop(), pMatchList.at(KWindowSystem::currentDesktop()).toAscii().data());

       return suspendall || suspendcurrent;
}

void   DesktopSwitch::deskNameChanged()
{
}

void   DesktopSwitch::deskCountChanged()
{
       emit set( !sync() );
}

void   DesktopSwitch::deskSwitched(int index)
{
   if (!suspendall)
   {
       emit set( !(suspendcurrent = query(index, pMatchList.at(index).toAscii().data())) );
   }
}

void   DesktopSwitch::wndAdded(WId wid)
{
       windows[wid] = new KWindowInfo(wid, NET::WMDesktop, NET::WM2WindowClass);

   if (!suspendall)
   {
       emit set( !(suspendall = query(0,  pMatchList.at(0).toAscii().data())) );

       deskSwitched(KWindowSystem::currentDesktop());
   }
}

void   DesktopSwitch::wndRemoved(WId wid)
{
       windows.erase(wid);

       emit set( !sync() );
}

void   DesktopSwitch::wndActiveChanged(WId wid)
{
}

void   DesktopSwitch::reset(QStringList& config)
{
       pMatchList = config;

   if (enabled)
   {
       emit set( !sync() );
   }
}

bool   DesktopSwitch::getEnabled()
{
       return enabled;
}

void   DesktopSwitch::setEnabled(bool value)
{
   if (enabled != value)
   {
       if (enabled = value)
       {
           emit set( !sync() );

           connect(KWindowSystem::self(), SIGNAL(desktopNamesChanged()), this, SLOT(deskNameChanged()));
           connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)), this, SLOT(deskCountChanged()));
           connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(deskSwitched(int)));
           connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(wndAdded(WId)));
           connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(wndRemoved(WId)));
           connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(wndActiveChanged(WId)));
           fprintf(stderr, "DS online\n");
       }   else
       {
           disconnect(this, SLOT(wndActiveChanged(WId)));
           disconnect(this, SLOT(wndRemoved(WId)));
           disconnect(this, SLOT(wndAdded(WId)));
           disconnect(this, SLOT(deskSwitched(int)));
           disconnect(this, SLOT(deskCountChanged()));
           disconnect(this, SLOT(deskNameChanged()));

           clear();
           fprintf(stderr, "DS offline\n");
         //suspendall = false;
         //suspendcurrent = false;
       }
   }
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
#include "switches.moc"