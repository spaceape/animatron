#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <QApplication>
#include <QStringList>
#include <KWindowInfo>
#include <KWindowSystem>
#include <map>
using  namespace std;

void   fetch();
bool   query(int pdesktop, char* const prules);

typedef map<WId, KWindowInfo*> m_windows_t;
typedef m_windows_t::iterator m_windows_i;

    m_windows_t  windows;

int main(int argc, char** argv)
{
    QApplication Application(argc, argv);

    if (argc < 2)
    {
        printf("Usage: %s <query>\n", argv[0]);
        return 0;
    }

    char* pattern = argv[1];
  //char* pattern = "kwritew ; ?55";

    fetch();
    printf("Test [%s]@all: %d\n", pattern, query(0, pattern));
    return 0;
}

void   fetch()
{
       QList<WId> list = KWindowSystem::windows();

       int  x;
       WId  id;
       KWindowInfo* info;

       for (x = 0; x != list.count(); ++x)
       {
            id   = list.at(x);
            info = new KWindowInfo(id, NET::WMDesktop, NET::WM2WindowClass);

            windows[id] = info;

            printf("A %s | %d\n", info->windowClassName().data(), info->desktop());
       }
}

bool   query(int desktop, char* const pattern)
{
       int e = 0;
       int mode_count = 0;
     //int mode_match

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

                     printf(">%.4d %s\n", hits, it->second->windowClassName().data());

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
