#ifndef config_h
#define config_h
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
#include <QLocale>
#include <QVariant>

#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>

#include <QAction>
#include <QLabel>
#include <QHeaderView>
#include <QButtonGroup>
#include <QSpacerItem>

#include "knuminput.h"
#include "kcombobox.h"
#include "kcolorbutton.h"
#include "kfontrequester.h"
#include "ktabwidget.h"

#include <Plasma/Wallpaper>
#include <Plasma/Package>

#include "ui_backgroundpage.h"
#include "ui_functionpage.h"

class BackgroundListModel;
class KFileDialog;
namespace KNS3 {
class DownloadDialog;
}

class DesktopList;

struct GlobalConfig
{
          QFont Font;
          QColor Color;
          int   Refresh;
          int   HorizontalRezolution;
          int   VerticalRezolution;
          int   Bins;
          int   Base;
          QString Style;
          int   Arrangement;
          QStringList History;
          QStringList Rules;

 public:
          GlobalConfig();
          GlobalConfig(KConfigGroup&);
          GlobalConfig(const GlobalConfig&);
 virtual  ~GlobalConfig();
          void  spit();
};

class ConfigWidget :public QWidget
{
           Q_OBJECT
           GlobalConfig& Settings;

           QGridLayout *gridLayout;
           QLabel *label_1;
           QHBoxLayout *FontBar;
           KFontRequester *FontChooser;
           QLabel *label_2;
           QHBoxLayout *ColorBar;
           KColorButton *ColorBtn;
           QSpacerItem *Spacer1;
           QLabel *label_3;
           QHBoxLayout *RefreshBar;
           KIntSpinBox *RefreshCombo;
           QSpacerItem *Spacer3;
           QLabel *label_4;
           QHBoxLayout *SizeBar;
           KIntSpinBox *WidthSpinner;
           KIntSpinBox *HeightSpinner;
           QLabel *label_5;
           KIntSpinBox *CountSpinner;
           QSpacerItem *Spacer2;
           QLabel *label_6;
           QHBoxLayout *BaseBar;
           KComboBox *BaseCombo;
           QSpacerItem *Spacer4;
           KTabWidget *SettingsGroup;
           QWidget *BackgroundSettings;
           QWidget *FunctionalitySettings;

           QRect mDesktopGeometry;

           BackgroundListModel* pImageListModel;
           KFileDialog* pOpenDialog;
           KNS3::DownloadDialog* pBrowseDialog;
           Ui_BackgroundPage BackgroundPage;

           DesktopList* pDesktopList;
           Ui_FunctionPage FunctionalityPage;

 private slots:
           void imagesAdd();
           void openDialogOkay();
           void openDialogDone();
           void imagesBrowse();
           void browseDialogOkay();
           void browseDialogDone();
           void imagesRemove(QString);
           void imageChanged(const QModelIndex&);
           void imageChanged(QString);
           void desktopSelected(int);
           void desktopRuleCompleted(QString);
           void _async_start();
           void modified();

 signals:
           void apply(bool);

 public:
           ConfigWidget(QWidget* parent, GlobalConfig& settings);
 virtual   ~ConfigWidget();

            QFont getFont();
            void setFont(QFont value);
            QColor getColor();
            void setColor(QColor);
            int  getRefresh();
            void setRefresh(int);
            int  getGridX();
            void setGridX(int);
            int  getGridY();
            void setGridY(int);
            int  getBins();
            void setBins(int);
            int  getBase();
            void setBase(int);

            void setDesktopGeometry(QRect);

            void updateScreenshot(QPersistentModelIndex index);
};

extern Plasma::Wallpaper* instance;

#endif
