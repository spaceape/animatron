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
#include "config.h"
#include "imagelist.h"
#include "blacklist.h"
#include "animatron.h"
#include <QTimer>
#include <QFileInfo>
#include <kfiledialog.h>
#include <knewstuff3/downloaddialog.h>
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
       GlobalConfig::GlobalConfig()
{
       Font = QFont("Courier New", 8);
       Color = QColor::fromRgb(0, 255, 0);
       BackgroundColor = QColor::fromRgb(0, 0, 0);
       Refresh = 25;
       HorizontalRezolution = 64;
       VerticalRezolution = 48;
       Bins = 8;
       Base = 16;
       Style = "";
       Arrangement = (int)Plasma::Wallpaper::CenteredResize;
}

       GlobalConfig::GlobalConfig(KConfigGroup& src)
{
}

       GlobalConfig::GlobalConfig(const GlobalConfig& src)
{
       Font = src.Font;
       Color = src.Color;
       BackgroundColor = src.BackgroundColor;
       Refresh = src.Refresh;
       HorizontalRezolution = src.HorizontalRezolution;
       VerticalRezolution = src.VerticalRezolution;
       Bins = src.Bins;
       Base = src.Base;
       Style = src.Style;
       Arrangement = src.Arrangement;

       History = src.History;
       Rules = src.Rules;
}

       GlobalConfig::~GlobalConfig()
{
}

void   GlobalConfig::spit()
{
       printf("font=[%s, %d]\n", Font.family().toAscii().data(), Font.pixelSize());
       printf("color=[%.2x %.2x %.2x]\n", Color.red(), Color.green(), Color.blue());
       printf("bgcolor=[%.2x %.2x %.2x]\n", BackgroundColor.red(), BackgroundColor.green(), BackgroundColor.blue());
       printf("refresh=%d\n", Refresh);
       printf("res=[%dx%d]\n", HorizontalRezolution, VerticalRezolution);
       printf("bins=%d\n", Bins);
       printf("base=%d\n", Base);
       printf("style=%s\n", Style.toAscii().data());
       printf("arrangement=%d\n", Arrangement);
       printf("history size=%d\n", History.count());
       printf("rules size=%d\n", Rules.count());
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

       ConfigWidget::ConfigWidget(QWidget* parent, GlobalConfig& settings, Animatron* plugin)
       :QWidget(parent),
        Settings(settings),
        Plugin(plugin)
{
       this->resize(784, 644);
       gridLayout = new QGridLayout(this);
       gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
       label_1 = new QLabel(this);
       label_1->setText("Font:");

       gridLayout->addWidget(label_1, 0, 0, 1, 1);

       FontBar = new QHBoxLayout();
       FontBar->setObjectName(QString::fromUtf8("FontBar"));
       FontChooser = new KFontRequester(this);
       FontChooser->setObjectName(QString::fromUtf8("FontChooser"));
       FontChooser->setFont(Settings.Font);
       connect(FontChooser, SIGNAL(fontSelected(QFont)), this, SLOT(modified()));
       FontBar->addWidget(FontChooser);

       gridLayout->addLayout(FontBar, 0, 1, 1, 1);

       label_2 = new QLabel(this);
       label_2->setText("Color:");

       gridLayout->addWidget(label_2, 1, 0, 1, 1);

       ColorBar = new QHBoxLayout();
       ColorBar->setObjectName(QString::fromUtf8("ColorBar"));
       ColorBtn = new KColorButton(this);
       ColorBtn->setObjectName(QString::fromUtf8("ColorBtn"));
       ColorBtn->setMinimumSize(QSize(64, 0));
       ColorBtn->setColor(Settings.Color);
       ColorBtn->setDefaultColor(QColor(0, 255, 0));
       connect(ColorBtn, SIGNAL(changed(QColor)), this, SLOT(modified()));
       ColorBar->addWidget(ColorBtn);

       Spacer1 = new QSpacerItem(187, 17, QSizePolicy::Expanding, QSizePolicy::Minimum);

       ColorBar->addItem(Spacer1);


       gridLayout->addLayout(ColorBar, 1, 1, 1, 1);

       label_3 = new QLabel(this);
       label_3->setText("Sync:");

       gridLayout->addWidget(label_3, 2, 0, 1, 1);

       RefreshBar = new QHBoxLayout();
       RefreshBar->setObjectName(QString::fromUtf8("RefreshBar"));
       RefreshCombo = new KIntSpinBox(this);
       RefreshCombo->setObjectName(QString::fromUtf8("RefreshCombo"));
       RefreshCombo->setEnabled(false);
       RefreshCombo->setMinimumSize(QSize(64, 0));
       RefreshCombo->setMinimum(10);
       RefreshCombo->setMaximum(50);
       RefreshCombo->setSingleStep(5);
       RefreshCombo->setValue(Settings.Refresh);
       //connect(RefreshCombo, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       RefreshBar->addWidget(RefreshCombo);

       Spacer3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

       RefreshBar->addItem(Spacer3);


       gridLayout->addLayout(RefreshBar, 2, 1, 1, 1);

       label_4 = new QLabel(this);
       label_4->setText("Grid:");

       gridLayout->addWidget(label_4, 3, 0, 1, 1);

       SizeBar = new QHBoxLayout();
       SizeBar->setObjectName(QString::fromUtf8("SizeBar"));
       WidthSpinner = new KIntSpinBox(this);
       WidthSpinner->setObjectName(QString::fromUtf8("WidthSpinner"));
       WidthSpinner->setMinimumSize(QSize(64, 0));
       WidthSpinner->setMinimum(32);
       WidthSpinner->setMaximum(256);
       WidthSpinner->setSingleStep(4);
       WidthSpinner->setValue(Settings.HorizontalRezolution);
       connect(WidthSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       SizeBar->addWidget(WidthSpinner);

       HeightSpinner = new KIntSpinBox(this);
       HeightSpinner->setObjectName(QString::fromUtf8("HeightSpinner"));
       HeightSpinner->setMinimumSize(QSize(64, 0));
       HeightSpinner->setMinimum(24);
       HeightSpinner->setMaximum(256);
       HeightSpinner->setSingleStep(4);
       HeightSpinner->setValue(Settings.VerticalRezolution);
       connect(HeightSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       SizeBar->addWidget(HeightSpinner);

       label_5 = new QLabel(this);
       label_5->setText("Bins:");

       SizeBar->addWidget(label_5);

       CountSpinner = new KIntSpinBox(this);
       CountSpinner->setObjectName(QString::fromUtf8("CountSpinner"));
       CountSpinner->setMinimumSize(QSize(64, 0));
       CountSpinner->setMinimum(4);
       CountSpinner->setMaximum(64);
       CountSpinner->setSingleStep(2);
       CountSpinner->setValue(Settings.Bins);
       connect(CountSpinner, SIGNAL(valueChanged(int)), this, SLOT(modified()));
       SizeBar->addWidget(CountSpinner);

       Spacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

       SizeBar->addItem(Spacer2);


       gridLayout->addLayout(SizeBar, 3, 1, 1, 1);

       label_6 = new QLabel(this);
       label_6->setText("Base:");

       gridLayout->addWidget(label_6, 4, 0, 1, 1);

       BaseBar = new QHBoxLayout();
       BaseBar->setObjectName(QString::fromUtf8("BaseBar"));
       BaseCombo = new KComboBox(this);
       BaseCombo->setObjectName(QString::fromUtf8("BaseCombo"));
       BaseCombo->setMinimumSize(QSize(64, 0));
       BaseCombo->setAutoCompletion(false);
       BaseCombo->addItem("10");
       BaseCombo->addItem("16");
       BaseCombo->addItem("36");
       BaseCombo->addItem("62");

       int x;
       for (x = 0; x < BaseCombo->count(); ++x) 
       {
            if (Settings.Base == BaseCombo->itemText(x).toInt())
            {
                 BaseCombo->setCurrentIndex(x);
                 break;
            }
       }
       connect(BaseCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));
       BaseBar->addWidget(BaseCombo);

       Spacer4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

       BaseBar->addItem(Spacer4);


       gridLayout->addLayout(BaseBar, 4, 1, 1, 1);

       SettingsGroup = new KTabWidget(this);
       SettingsGroup->setObjectName(QString::fromUtf8("SettingsGroup"));
       SettingsGroup->setTabPosition(QTabWidget::West);

  //setup BackgroundPage
       BackgroundSettings = new QWidget();
       BackgroundPage.setupUi(BackgroundSettings);
       pImageListModel = new BackgroundListModel(instance, this);
       pImageListModel->setResizeMethod((Plasma::Wallpaper::ResizeMethod)Settings.Arrangement);
       pImageListModel->setWallpaperSize(QSize(1024, 768));
       pImageListModel->reload(Settings.History);

       QTimer::singleShot(250, this, SLOT(_async_start()));
       BackgroundPage.ImageList->setItemDelegate(new BackgroundDelegate(BackgroundPage.ImageList));

       BackgroundPage.ImageList->setMinimumWidth(
       (BackgroundDelegate::SCREENSHOT_SIZE + BackgroundDelegate::MARGIN * 2 + BackgroundDelegate::BLUR_INCREMENT) * 3 +
         BackgroundPage.ImageList->spacing() * 4 + 
         QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 
         QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2 + 7
       );

       BackgroundPage.ImageList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
       connect(BackgroundPage.ImageList, SIGNAL(clicked(const QModelIndex&)), this, SLOT(imageSelected()));

       BackgroundPage.ImageList->setSpacing(2);
       BackgroundPage.ImageList->setViewMode(QListView::IconMode);

       RemoveButtonManager* rbmanager = new RemoveButtonManager(BackgroundPage.ImageList, &Settings.History);
       connect(rbmanager, SIGNAL(removeClicked(QString)), this, SLOT(imagesRemove(QString)));

       BackgroundPage.ArrangementCombo->addItem("No Background", 0);
       //BackgroundPage.ArrangementCombo->addItem("Scaled & Cropped", Plasma::Wallpaper::ScaledAndCroppedResize);
       //BackgroundPage.ArrangementCombo->addItem("Scaled", Plasma::Wallpaper::ScaledResize);
       //BackgroundPage.ArrangementCombo->addItem("Scaled, keep proportions", Plasma::Wallpaper::MaxpectResize);
       BackgroundPage.ArrangementCombo->addItem("Centered", Plasma::Wallpaper::CenteredResize);
       //BackgroundPage.ArrangementCombo->addItem("Tiled", Plasma::Wallpaper::TiledResize);
       //BackgroundPage.ArrangementCombo->addItem("Center Tiled", Plasma::Wallpaper::CenterTiledResize);

       for (x = 0; x < BackgroundPage.ArrangementCombo->count(); ++x) 
       {
            if (Settings.Arrangement == BackgroundPage.ArrangementCombo->itemData(x).value<int>())
            {
                BackgroundPage.ArrangementCombo->setCurrentIndex(x);
                break;
            }
       }
       connect(BackgroundPage.ArrangementCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(modified()));

       BackgroundPage.OpenWallBtn->setIcon(KIcon("document-open"));
       connect(BackgroundPage.OpenWallBtn, SIGNAL(clicked()), this, SLOT(imagesAdd()));
       pOpenDialog = NULL;

       BackgroundPage.GetNewWallBtn->setIcon(KIcon("get-hot-new-stuff"));
       connect(BackgroundPage.GetNewWallBtn, SIGNAL(clicked()), this, SLOT(imagesBrowse()));
       pBrowseDialog = NULL;

       BackgroundSettings->setObjectName(QString::fromUtf8("BackgroundSettings"));
       SettingsGroup->addTab(BackgroundSettings, QString());
       SettingsGroup->setTabText(0, "Background");

 //setup FunctionalityPage
       FunctionalitySettings = new QWidget();
       FunctionalityPage.setupUi(FunctionalitySettings);

       connect(FunctionalityPage.RuleEdit, SIGNAL(textChanged(QString)), this, SLOT(desktopRuleCompleted(QString)));

       pDesktopList = new DesktopList(FunctionalityPage.DesktopWidget);
       FunctionalityPage.DesktopLayout->addWidget(pDesktopList);
       connect(pDesktopList, SIGNAL(IndexChanged(int)), this, SLOT(desktopSelected(int)));
       pDesktopList->shout();

       FunctionalityPage.PrevDesktopBtn->setIcon(KIcon("arrow-left"));
       connect(FunctionalityPage.PrevDesktopBtn, SIGNAL(clicked(bool)), pDesktopList, SLOT(prev()));

       FunctionalityPage.NextDesktopBtn->setIcon(KIcon("arrow-right"));
       connect(FunctionalityPage.NextDesktopBtn, SIGNAL(clicked(bool)), pDesktopList, SLOT(next()));

       pAddRuleMenu = new AddRuleMenu(pDesktopList);
       FunctionalityPage.AddRuleBtn->setIcon(KIcon("list-add"));
       FunctionalityPage.AddRuleBtn->setMenu(pAddRuleMenu);
       connect(pAddRuleMenu, SIGNAL(perform(QString)), this, SLOT(desktopRuleAdd(QString)));

       pRemRuleMenu = new RemRuleMenu(pDesktopList);
       FunctionalityPage.RemRuleBtn->setIcon(KIcon("list-remove"));
       FunctionalityPage.RemRuleBtn->setMenu(pRemRuleMenu);
       connect(pRemRuleMenu, SIGNAL(perform(QString)), this, SLOT(destopRuleRemove(QString)));

       FunctionalitySettings->setObjectName(QString::fromUtf8("FunctionalitySettings"));
       SettingsGroup->addTab(FunctionalitySettings, QString());
       SettingsGroup->setTabText(1, "Quantum Lock");

       gridLayout->addWidget(SettingsGroup, 5, 0, 1, 2);

       SettingsGroup->setCurrentIndex(0);

       connect(this, SIGNAL(apply(bool)), parent, SLOT(settingsChanged(bool)));
       emit apply(false);
}

       ConfigWidget::~ConfigWidget()
{
}

void   ConfigWidget::imagesAdd()
{
   if (!pOpenDialog)
   {
       pOpenDialog = new KFileDialog(KUrl(), QString::fromAscii( "*.png *.jpeg *.jpg *.xcf *.svg *.svgz *.bmp" ), NULL);
       pOpenDialog->setOperationMode(KFileDialog::Opening);
       pOpenDialog->setInlinePreviewShown(true);
       pOpenDialog->setCaption(QString::fromAscii("Select wallpaper image..."));
       pOpenDialog->setModal(false);
       connect(pOpenDialog, SIGNAL(okClicked()), this, SLOT(openDialogOkay()));
       connect(pOpenDialog, SIGNAL(destroyed(QObject*)), this, SLOT(openDialogDone()));
   }

       pOpenDialog->show();
       pOpenDialog->raise();
       pOpenDialog->activateWindow();
}

void   ConfigWidget::openDialogOkay()
{
       Q_ASSERT(pImageListModel);

   const QFileInfo info(pOpenDialog->selectedFile());
   const QString   pathname = info.canonicalFilePath();

   if (!pathname.isEmpty())
   {
            QModelIndex   index;

        if (!pImageListModel->contains(pathname))
        {
            pImageListModel->addBackground(pathname);
            Settings.History.append(pathname);
        }

            index = pImageListModel->indexOf(pathname);

        if (index.isValid())
        {
            BackgroundPage.ImageList->setCurrentIndex(index);
            imageSelected();
        }
   }
}

void   ConfigWidget::openDialogDone()
{
}

void   ConfigWidget::imagesBrowse()
{
   if (!pBrowseDialog)
   {
        pBrowseDialog = new KNS3::DownloadDialog("animatron.knsrc", NULL);
        connect(pBrowseDialog, SIGNAL(accepted()), SLOT(browseDialogOkay()));
        connect(pBrowseDialog, SIGNAL(destroyed()), SLOT(browseDialogDone()));
   }

        pBrowseDialog->show();
}

void   ConfigWidget::browseDialogOkay()
{
       pImageListModel->reload();
       pImageListModel->reload(Settings.History);

       imageSelected();
}

void   ConfigWidget::browseDialogDone()
{

}

void   ConfigWidget::imagesRemove(QString pathname)
{
   int index = Settings.History.indexOf(pathname);

       printf("Removing image `%s`@%d\n", pathname.toAscii().data(), index);

   if (index >= 0)
   {
        Settings.History.removeAt(index);
        pImageListModel->reload(Settings.History);
        emit apply(true);
   }
}

void   ConfigWidget::imageChanged(const QModelIndex& index)
{
    if (index.row() >= 0)
    {
        Plasma::Package* package = pImageListModel->package(index.row());

        if (package->structure()->contentsPrefix().isEmpty())
            imageChanged(package->filePath("preferred"));
            else
            imageChanged(package->path());
    }
}

void   ConfigWidget::imageChanged(QString pathname)
{
       QString Style;

   if (pathname != Settings.Style)
   {
       if (QDir::isAbsolutePath(pathname))
       {
           Plasma::Package package(pathname, Plugin->packageStructure(Plugin));
           Style = package.filePath("preferred");

        if (Style.isEmpty() || !QFile::exists(Style))
        {
            Style = Settings.Style;
        }
    } else 
    {
      Style = pathname;
    }

       Settings.Style = Style;
       emit apply(true);
   }

       fprintf(stderr, "Changed background image to `%s`...\n", Style.toAscii().data());
}

void   ConfigWidget::imageSelected()
{
   if (BackgroundPage.ArrangementCombo->currentIndex() == 0)
   {
       BackgroundPage.ArrangementCombo->setCurrentIndex(1);
   }

       modified();
}

void   ConfigWidget::desktopSelected(int index)
{
       while (Settings.Rules.count() <= index)
       {
              Settings.Rules.append("#");
       }

       FunctionalityPage.RuleEdit->setText(Settings.Rules.at(index));
}

void   ConfigWidget::desktopRuleCompleted(QString text)
{
       Settings.Rules[pDesktopList->getIndex()] = text;
       emit apply(true);
}

// NOTE:
// desktopRule-- methods are the lame substitute of a proper validator

void   ConfigWidget::desktopRuleAdd(QString text)
{
       QString rule = FunctionalityPage.RuleEdit->text();

       if (rule.length())
       {
           if (*rule.end() != ' ')
               rule.push_back(' ');
       }

       rule += text;

       FunctionalityPage.RuleEdit->setText(rule);
}

void   ConfigWidget::desktopRuleRemove(QString )
{
}

void   ConfigWidget::_async_start()
{
       BackgroundPage.ImageList->setModel(pImageListModel);
       connect(BackgroundPage.ImageList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
               this, SLOT(imageChanged(const QModelIndex&)));

       QModelIndex index = pImageListModel->indexOf(Settings.Style);

   if  (index.isValid())
   {
        BackgroundPage.ImageList->setCurrentIndex(index);
   }
}

void   ConfigWidget::modified()
{
       Settings.Font = FontChooser->font();
       Settings.Color = ColorBtn->color();
     //Settings.Refresh = RefreshCombo->value();
       Settings.HorizontalRezolution = WidthSpinner->value();
       Settings.VerticalRezolution = HeightSpinner->value();
       Settings.Bins = CountSpinner->value();
       Settings.Base = BaseCombo->currentText().toInt();

       Settings.Arrangement = BackgroundPage.ArrangementCombo->itemData(
                BackgroundPage.ArrangementCombo->currentIndex()
       ).value<int>();

       emit apply(true);
}

QFont ConfigWidget::getFont()
{
       return FontChooser->font();
}

void   ConfigWidget::setFont(QFont value)
{
       FontChooser->setFont(value);
}

QColor ConfigWidget::getColor()
{
       return ColorBtn->color();
}

void   ConfigWidget::setColor(QColor value)
{
       ColorBtn->setColor(value);
}

int    ConfigWidget::getRefresh()
{
       return 0;
}

void   ConfigWidget::setRefresh(int value)
{
}

int    ConfigWidget::getGridX()
{
       return WidthSpinner->value();
}

void   ConfigWidget::setGridX(int value)
{
       WidthSpinner->setValue(value);
}

int    ConfigWidget::getGridY()
{
       return HeightSpinner->value();
}

void   ConfigWidget::setGridY(int value)
{
       HeightSpinner->setValue(value);
}

int    ConfigWidget::getBins()
{
       return CountSpinner->value();
}

void   ConfigWidget::setBins(int value)
{
       CountSpinner->setValue(value);
}

int    ConfigWidget::getBase()
{
       return BaseCombo->currentText().toInt();
}

void   ConfigWidget::setBase(int value)
{
       int x;
       for (x = 0; x < BaseCombo->count(); ++x) 
       {
            if (value == BaseCombo->itemText(x).toInt())
            {
                BaseCombo->setCurrentIndex(x);
                break;
            }
       }
}

void   ConfigWidget::updateScreenshot(QPersistentModelIndex index)
{
       BackgroundPage.ImageList->update(index);
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
