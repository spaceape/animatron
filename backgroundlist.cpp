#ifndef backgroundlist_cpp
#define backgroundlist_cpp
/*
    Copyright (c) 2011, spaceape [[ spaceape99@gmail.com ]]
    All rights reserved.

    CREDITS:
    Paolo Capriotti <p.capriotti@gmail.com>
    Peter Penz <peter.penz@gmx.at>
    Davide Bettio <davide.bettio@kdemail.net>

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
#include <QFile>
#include <QDir>
#include <QThreadPool>
#include <QUuid>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTextDocument>

#include <KDebug>
#include <KFileMetaInfo>
#include <KGlobal>
#include <KProgressDialog>
#include <KStandardDirs>
#include <KGlobalSettings>
#include <KIconEffect>
#include <KIO/PreviewJob>

#include <Plasma/Package>
#include <Plasma/PackageStructure>
#include <Plasma/PaintUtils>
#include "backgroundlist.h"
#include "animatron.h"

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
ImageSizeFinder::ImageSizeFinder(const QString &path, QObject *parent)
    : QObject(parent),
      m_path(path)
{
}

void ImageSizeFinder::run()
{
    QImage image(m_path);
    emit sizeFound(m_path, image.size());
}


BackgroundListModel::BackgroundListModel(Plasma::Wallpaper *listener, QObject *parent)
    : QAbstractListModel(parent),
      m_structureParent(listener),
      m_size(0,0),
      m_resizeMethod(Plasma::Wallpaper::ScaledResize)
{
    connect(&m_dirwatch, SIGNAL(deleted(QString)), this, SLOT(removeBackground(QString)));
    m_previewUnavailablePix.fill(Qt::transparent);
    //m_previewUnavailablePix = KIcon("unknown").pixmap(m_previewUnavailablePix.size());
}

BackgroundListModel::~BackgroundListModel()
{
    qDeleteAll(m_packages);
}

void BackgroundListModel::removeBackground(const QString &path)
{
    QModelIndex index;
    while ((index = indexOf(path)).isValid()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        Plasma::Package *package = m_packages.at(index.row());
        m_packages.removeAt(index.row());
        delete package;
        endRemoveRows();
    }
}

void BackgroundListModel::reload()
{
    reload(QStringList());
}

void BackgroundListModel::reload(const QStringList &selected)
{
    if (!m_packages.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_packages.count() - 1);
        qDeleteAll(m_packages);
        m_packages.clear();
        endRemoveRows();
    }

    if (!selected.isEmpty()) {
        processPaths(selected);
    }

    const QStringList dirs = KGlobal::dirs()->findDirs("wallpaper", "");
    kDebug() << "going looking in" << dirs;
    BackgroundFinder *finder = new BackgroundFinder(m_structureParent, dirs);
    connect(finder, SIGNAL(backgroundsFound(QStringList,QString)), this, SLOT(backgroundsFound(QStringList,QString)));
    m_findToken = finder->token();
    finder->start();
}

void BackgroundListModel::backgroundsFound(const QStringList &paths, const QString &token)
{
    if (token == m_findToken) {
        processPaths(paths);
    }
}

void BackgroundListModel::processPaths(const QStringList &paths)
{
    QList<Plasma::Package *> newPackages;
    foreach (const QString &file, paths) {
        if (!contains(file) && QFile::exists(file)) {
            Plasma::PackageStructure::Ptr structure = Plasma::Wallpaper::packageStructure(m_structureParent);
            Plasma::Package *package  = new Plasma::Package(file, structure);
            if (package->isValid()) {
                newPackages << package;
            } else {
                delete package;
            }
        }
    }

    // add new files to dirwatch
    foreach (Plasma::Package *b, newPackages) {
        if (!m_dirwatch.contains(b->path())) {
            m_dirwatch.addFile(b->path());
        }
    }

    if (!newPackages.isEmpty()) {
        const int start = rowCount();
        beginInsertRows(QModelIndex(), start, start + newPackages.size());
        m_packages.append(newPackages);
        endInsertRows();
    }
    //kDebug() << t.elapsed();
}

void BackgroundListModel::addBackground(const QString& path)
{
    if (!contains(path)) {
        if (!m_dirwatch.contains(path)) {
            m_dirwatch.addFile(path);
        }
        beginInsertRows(QModelIndex(), 0, 0);
        Plasma::PackageStructure::Ptr structure = Plasma::Wallpaper::packageStructure(m_structureParent);
        Plasma::Package *pkg = new Plasma::Package(path, structure);
        m_packages.prepend(pkg);
        endInsertRows();
    }
}

QModelIndex BackgroundListModel::indexOf(const QString &path) const
{
    for (int i = 0; i < m_packages.size(); i++) {
        // packages will end with a '/', but the path passed in may not
        QString package = m_packages[i]->path();
        if (package.at(package.length() - 1) == '/') {
            package.truncate(package.length() - 1);
        }

        if (path.startsWith(package)) {
            // FIXME: ugly hack to make a difference between local files in the same dir
            // package->path does not contain the actual file name
            if ((!m_packages[i]->structure()->contentsPrefixPaths().isEmpty()) ||
                (path == m_packages[i]->filePath("preferred"))) {
                return index(i, 0);
            }
        }
    }
    return QModelIndex();
}

bool BackgroundListModel::contains(const QString &path) const
{
    return indexOf(path).isValid();
}

int BackgroundListModel::rowCount(const QModelIndex &) const
{
    return m_packages.size();
}

QSize BackgroundListModel::bestSize(Plasma::Package *package) const
{
    if (m_sizeCache.contains(package)) {
        return m_sizeCache.value(package);
    }

    const QString image = package->filePath("preferred");
    if (image.isEmpty()) {
        return QSize();
    }

    KFileMetaInfo info(image, QString(), KFileMetaInfo::TechnicalInfo);
    QSize size(info.item("http://freedesktop.org/standards/xesam/1.0/core#width").value().toInt(),
               info.item("http://freedesktop.org/standards/xesam/1.0/core#height").value().toInt());
    //backup solution if strigi does not work
    if (size.width() == 0 || size.height() == 0) {
//        kDebug() << "fall back to QImage, check your strigi";
        ImageSizeFinder *finder = new ImageSizeFinder(image);
        connect(finder, SIGNAL(sizeFound(QString,QSize)), this,
                SLOT(sizeFound(QString,QSize)));
        QThreadPool::globalInstance()->start(finder);
        size = QSize(-1, -1);
    }

    const_cast<BackgroundListModel *>(this)->m_sizeCache.insert(package, size);
    return size;
}

void BackgroundListModel::sizeFound(const QString &path, const QSize &s)
{
    QModelIndex index = indexOf(path);
    if (index.isValid()) {
        Plasma::Package *package = m_packages.at(index.row());
        m_sizeCache.insert(package, s);
        static_cast<Animatron*>(m_structureParent)->updateScreenshot(index);
    }
}

QVariant BackgroundListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_packages.size()) {
        return QVariant();
    }

    Plasma::Package *b = package(index.row());
    if (!b) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        QString title = b->metadata().name();

        if (title.isEmpty()) {
            return QFileInfo(b->filePath("preferred")).completeBaseName();
        }

        return title;
    }
    break;

    case BackgroundDelegate::ScreenshotRole: {
        if (m_previews.contains(b)) {
            return m_previews.value(b);
        }

        KUrl file(b->filePath("preferred"));
        if (!m_previewJobs.contains(file) && file.isValid()) {
            KFileItemList list;
            list.append(KFileItem(file, QString(), 0));
            KIO::PreviewJob* job = KIO::filePreview(list, BackgroundDelegate::SCREENSHOT_SIZE, BackgroundDelegate::SCREENSHOT_SIZE/1.6);
            job->setIgnoreMaximumSize(true);
            connect(job, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
                    this, SLOT(showPreview(const KFileItem&, const QPixmap&)));
            connect(job, SIGNAL(failed(const KFileItem&)),
                    this, SLOT(previewFailed(const KFileItem&)));
            const_cast<BackgroundListModel *>(this)->m_previewJobs.insert(file, QPersistentModelIndex(index));
        }

        const_cast<BackgroundListModel *>(this)->m_previews.insert(b, m_previewUnavailablePix);
        return m_previewUnavailablePix;
    }
    break;

    case BackgroundDelegate::AuthorRole:
        return b->metadata().author();
    break;

    case BackgroundDelegate::ResolutionRole:{
        QSize size = bestSize(b);

        if (size.isValid()) {
            return QString("%1x%2").arg(size.width()).arg(size.height());
        }

        return QString();
    }
    break;

    default:
        return QVariant();
    break;
    }
}

void BackgroundListModel::showPreview(const KFileItem &item, const QPixmap &preview)
{
    QPersistentModelIndex index = m_previewJobs.value(item.url());
    m_previewJobs.remove(item.url());

    if (!index.isValid()) {
        return;
    }

    Plasma::Package *b = package(index.row());
    if (!b) {
        return;
    }

    m_previews.insert(b, preview);
    //kDebug() << "preview size:" << preview.size();
    static_cast<Animatron*>(m_structureParent)->updateScreenshot(index);
}

void BackgroundListModel::previewFailed(const KFileItem &item)
{
    m_previewJobs.remove(item.url());
}

Plasma::Package* BackgroundListModel::package(int index) const
{
    return m_packages.at(index);
}

void BackgroundListModel::setWallpaperSize(const QSize& size)
{
    m_size = size;
}

void BackgroundListModel::setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod)
{
    m_resizeMethod = resizeMethod;
}

BackgroundFinder::BackgroundFinder(Plasma::Wallpaper *structureParent, const QStringList &paths)
    : QThread(structureParent),
      m_structure(Plasma::Wallpaper::packageStructure(structureParent)),
      m_paths(paths),
      m_token(QUuid().toString())
{
}

BackgroundFinder::~BackgroundFinder()
{
    wait();
}

QString BackgroundFinder::token() const
{
    return m_token;
}

void BackgroundFinder::run()
{
    //QTime t;
    //t.start();
    QSet<QString> suffixes;
    suffixes << "png" << "jpeg" << "jpg" << "svg" << "svgz";

    QStringList papersFound;
    //kDebug() << "starting with" << m_paths;

    QDir dir;
    dir.setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::Readable);
    Plasma::Package pkg(QString(), m_structure);

    int i;
    for (i = 0; i < m_paths.count(); ++i) {
        const QString path = m_paths.at(i);
        //kDebug() << "doing" << path;
        dir.setPath(path);
        const QFileInfoList files = dir.entryInfoList();
        foreach (const QFileInfo &wp, files) {
            if (wp.isDir()) {
                //kDebug() << "directory" << wp.fileName() << validPackages.contains(wp.fileName());
                const QString name = wp.fileName();
                if (name == "." || name == "..") {
                    // do nothing
                    continue;
                }

                const QString filePath = wp.filePath();
                if (QFile::exists(filePath + "/metadata.desktop")) {
                    pkg.setPath(filePath);
                    if (pkg.isValid()) {
                        papersFound << pkg.path();
                        continue;
                        //kDebug() << "gots a" << wp.filePath();
                    }
                }

                // add this to the directories we should be looking at
                m_paths.append(filePath);
            } else if (suffixes.contains(wp.suffix().toLower())) {
                //kDebug() << "     adding image file" << wp.filePath();
                papersFound << wp.filePath();
            }
        }
    }

    //kDebug() << "background found!" << papersFound.size() << "in" << i << "dirs, taking" << t.elapsed() << "ms";
    emit backgroundsFound(papersFound, m_token);
    deleteLater();
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
static const int BLUR_PAD = 6;

BackgroundDelegate::BackgroundDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
    m_maxHeight = SCREENSHOT_SIZE/1.6 + BLUR_INCREMENT;
    m_maxWidth = SCREENSHOT_SIZE + BLUR_INCREMENT;
}

void BackgroundDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const QString author = index.model()->data(index, AuthorRole).toString();
    const QString resolution = index.model()->data(index, ResolutionRole).toString();
    const QPixmap pix = index.model()->data(index, ScreenshotRole).value<QPixmap>();

    // Highlight selected item
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

    // Draw wallpaper thumbnail
    if (pix.isNull()) {
        painter->fillRect(option.rect, option.palette.brush(QPalette::Base));
    } else {
        // blur calculation
        QImage blur(pix.size() + QSize(BLUR_INCREMENT + BLUR_PAD, BLUR_INCREMENT + BLUR_PAD), QImage::Format_ARGB32);
        QRect blurRect = QRect(QPoint((blur.width() - pix.width()) / 2, (blur.height() - pix.height()) / 2), pix.size());
        blur.fill(Qt::transparent);
        QPainter p(&blur);

        QColor color = option.palette.color(QPalette::Base);
        bool darkBaseColor = qGray(color.rgb()) < 192;
        p.fillRect(blurRect, darkBaseColor ? Qt::white : Qt::black);
        p.end();

        // apply blur with a radius of 2 as thumbnail shadow
        Plasma::PaintUtils::shadowBlur(blur, 2, darkBaseColor ? Qt::white : Qt::black);

        // calculate point
        const int bx = (option.rect.width() - blur.width()) / 2;
        const int by = MARGIN + qMax(0, m_maxHeight - blur.height());
        QRect shadowRect = QRect(option.rect.topLeft(), blur.size()).translated(bx, by);
        // draw the blur
        painter->drawImage(shadowRect.topLeft(), blur);
        // draw the actual thumbnail
        painter->drawPixmap(QRect(shadowRect.topLeft() + QPoint((shadowRect.width() - pix.width()) / 2, (shadowRect.height() - pix.height()) / 2),
                                  pix.size()), pix);
    }

    //Use a QTextDocument to layout the text
    QTextDocument document;
    QString html = QString("<strong>%1</strong>").arg(title);

    if (!author.isEmpty()) {
        QString authorCaption = i18nc("Caption to wallpaper preview, %1 author name",
                              "by %1", author);

        html += QString("<br /><span style=\"font-size: %1pt;\">%2</span>")
                .arg(KGlobalSettings::smallestReadableFont().pointSize())
                .arg(authorCaption);
    }

    if (!resolution.isEmpty()) {
        html += QString("<br /><span style=\"font-size: %1pt;\">%2</span>")
                .arg(KGlobalSettings::smallestReadableFont().pointSize())
                .arg(resolution);
    }

    //Set the text color according to the item state
    QColor color;
    if (option.state & QStyle::State_Selected) {
        color = QApplication::palette().brush(QPalette::HighlightedText).color();
    }else{
        color = QApplication::palette().brush(QPalette::Text).color();
    }
    html = QString("<div style=\"color: %1\" align=\"center\">%2</div>").arg(color.name()).arg(html);

    document.setHtml(html);

    //Calculate positioning
    int x = option.rect.left() + MARGIN;

    //Enable word-wrap
    document.setTextWidth(m_maxWidth);

    //Center text on the row
    int y = option.rect.top() + m_maxHeight + MARGIN * 2; //qMax(0 ,(int)((option.rect.height() - document.size().height()) / 2));

    //Draw text
    painter->save();
    painter->translate(x, y);
    document.drawContents(painter, QRect(QPoint(0, 0), option.rect.size() - QSize(0, m_maxHeight + MARGIN * 2)));
    painter->restore();
}

QSize BackgroundDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option)
    const QString title = index.model()->data(index, Qt::DisplayRole).toString();
    const QString author = index.model()->data(index, AuthorRole).toString();
    const int fontSize = KGlobalSettings::smallestReadableFont().pointSize();

    //Generate a sample complete entry (with the real title) to calculate sizes
    QTextDocument document;
    QString html = QString("<strong>%1</strong><br />").arg(title);
    if (!author.isEmpty()) {
        html += QString("<span style=\"font-size: %1pt;\">by %2</span><br />").arg(fontSize).arg(author);
    }
    html += QString("<span style=\"font-size: %1pt;\">1600x1200</span>").arg(fontSize);

    document.setHtml(html);
    document.setTextWidth(m_maxWidth);

    QSize s(m_maxWidth + MARGIN * 2, 
            m_maxHeight + MARGIN * 3 + (int)(document.size().height()));
    return s;
}

RemoveButton::RemoveButton(QWidget* parent) :
    QAbstractButton(parent),
    m_isHovered(false),
    m_leftMouseButtonPressed(false),
    m_fadingValue(0),
    m_icon(),
    m_fadingTimeLine(0)
{
    setFocusPolicy(Qt::NoFocus);
    parent->installEventFilter(this);
    resize(sizeHint());
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(refreshIcon()));
    
    m_icon = KIconLoader::global()->loadIcon("edit-delete",
                                             KIconLoader::NoGroup,
                                             qMin(width(), height()));
    setToolTip(i18n("Remove from list"));
}

RemoveButton::~RemoveButton()
{
}

QSize RemoveButton::sizeHint() const
{
    return QSize(32, 32);
}

void RemoveButton::reset()
{
    m_itemName = "";
    hide();
}

void RemoveButton::setVisible(bool visible)
{
    QAbstractButton::setVisible(visible);

    stopFading();
    if (visible) {
        startFading();
    }

}

bool RemoveButton::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == parent()) {
        switch (event->type()) {
        case QEvent::Leave:
            hide();
            break;

        case QEvent::MouseMove:
            if (m_leftMouseButtonPressed) {
                // Don't forward mouse move events to the viewport,
                // otherwise a rubberband selection will be shown when
                // clicking on the selection removeButton and moving the mouse
                // above the viewport.
                return true;
            }
            break;

        default:
            break;
        }
    }

    return QAbstractButton::eventFilter(obj, event);
}

void RemoveButton::enterEvent(QEvent* event)
{
    QAbstractButton::enterEvent(event);

    // if the mouse cursor is above the selection removeButton, display
    // it immediately without fading timer
    m_isHovered = true;
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
    }
    m_fadingValue = 255;

    update();
}

void RemoveButton::leaveEvent(QEvent* event)
{
    QAbstractButton::leaveEvent(event);
    m_isHovered = false;
    update();
}

void RemoveButton::mousePressEvent(QMouseEvent* event)
{
    QAbstractButton::mousePressEvent(event);
    m_leftMouseButtonPressed = (event->buttons() & Qt::LeftButton);
}

void RemoveButton::mouseReleaseEvent(QMouseEvent* event)
{
    QAbstractButton::mouseReleaseEvent(event);
    m_leftMouseButtonPressed = (event->buttons() & Qt::LeftButton);
}

void RemoveButton::resizeEvent(QResizeEvent* event)
{
    QAbstractButton::resizeEvent(event);

    m_icon = KIconLoader::global()->loadIcon("edit-delete",
                                             KIconLoader::NoGroup,
                                             qMin(width(), height()));
    update();
}

void RemoveButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());

    // draw the icon overlay
    if (m_isHovered) {
        KIconEffect iconEffect;
        QPixmap activeIcon = iconEffect.apply(m_icon, KIconLoader::Desktop, KIconLoader::ActiveState);
        painter.drawPixmap(0, 0, activeIcon);
    } else {
        if (m_fadingValue < 255) {
            // apply an alpha mask respecting the fading value to the icon
            QPixmap icon = m_icon;
            QPixmap alphaMask(icon.width(), icon.height());
            const QColor color(m_fadingValue, m_fadingValue, m_fadingValue);
            alphaMask.fill(color);
            icon.setAlphaChannel(alphaMask);
            painter.drawPixmap(0, 0, icon);
        } else {
            // no fading is required
            painter.drawPixmap(0, 0, m_icon);
        }
    }
}

void RemoveButton::setFadingValue(int value)
{
    m_fadingValue = value;
    if (m_fadingValue >= 255) {
        Q_ASSERT(m_fadingTimeLine != 0);
        m_fadingTimeLine->stop();
    }
    update();
}

void RemoveButton::refreshIcon()
{
    m_icon = KIconLoader::global()->loadIcon("edit-delete",
                                             KIconLoader::NoGroup,
                                             qMin(width(), height()));
    update();
}

void RemoveButton::startFading()
{
    Q_ASSERT(m_fadingTimeLine == 0);

    const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    const int duration = animate ? 600 : 1;

    m_fadingTimeLine = new QTimeLine(duration, this);
    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));
    m_fadingTimeLine->setFrameRange(0, 255);
    m_fadingTimeLine->start();
    m_fadingValue = 0;
}

void RemoveButton::stopFading()
{
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
        delete m_fadingTimeLine;
        m_fadingTimeLine = 0;
    }
    m_fadingValue = 0;
}

void RemoveButton::setItemName(const QString& name)
{
   m_itemName = name;
}

QString RemoveButton::itemName() const
{
   return m_itemName;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
RemoveButtonManager::RemoveButtonManager(QAbstractItemView* parent, QStringList *list) :
    QObject(parent),
    m_view(parent),
    m_removeButton(0),
    m_connected(false)
{
    m_removableWallpapers = list;
  
    parent->setMouseTracking(true);
  
    connect(parent, SIGNAL(entered(const QModelIndex&)),
            this, SLOT(slotEntered(const QModelIndex&)));
    connect(parent, SIGNAL(viewportEntered()),
            this, SLOT(slotViewportEntered()));
    m_removeButton = new RemoveButton(m_view->viewport());
    m_removeButton->hide();
    connect(m_removeButton, SIGNAL(clicked(bool)),
            this, SLOT(removeButtonClicked()));
}

RemoveButtonManager::~RemoveButtonManager()
{
}

void RemoveButtonManager::slotEntered(const QModelIndex& index)
{
    m_removeButton->hide();

    BackgroundListModel *model = static_cast<BackgroundListModel *>(m_view->model());
    m_removeButton->setItemName(model->package(index.row())->filePath("preferred"));

    if (m_removableWallpapers->indexOf(m_removeButton->itemName()) < 0){
      return;
    }

    if (!m_connected) {
        connect(m_view->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotRowsRemoved(const QModelIndex&, int, int)));
//         connect(m_view->selectionModel(),
//                 SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
//                 this, SLOT(slotSelectionChanged(const QItemSelection&, const QItemSelection&)));
        m_connected = true;
    }

    // increase the size of the removeButton for large items
    const int height = m_view->iconSize().height();
    if (height >= KIconLoader::SizeEnormous) {
        m_removeButton->resize(KIconLoader::SizeMedium, KIconLoader::SizeMedium);
    } else if (height >= KIconLoader::SizeLarge) {
        m_removeButton->resize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    } else {
        m_removeButton->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    }

    const QRect rect = m_view->visualRect(index);
    int x = rect.left();
    int y = rect.top();

    m_removeButton->move(QPoint(x, y));
    m_removeButton->show();
}

void RemoveButtonManager::slotViewportEntered()
{
    m_removeButton->hide();
}

void RemoveButtonManager::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    m_removeButton->hide();
}

void RemoveButtonManager::removeButtonClicked()
{
    RemoveButton *removeButton = static_cast<RemoveButton *>(sender());
    emit removeClicked(removeButton->itemName());
}
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

#include "backgroundlist.moc"
#endif