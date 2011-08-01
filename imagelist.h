#ifndef backgroundlist_h
#define backgroundlist_h
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
#include <QPixmap>
#include <QRunnable>
#include <QThread>
#include <QTimeLine>
#include <QAbstractButton>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>
#include <QAbstractItemView>
#include <KDirWatch>
#include <KFileItem>
#include <kurl.h>

#include <Plasma/Wallpaper>

class QEventLoop;
class KProgressDialog;

namespace Plasma {
class Package;
}

class ImageSizeFinder : public QObject, public QRunnable
{
    Q_OBJECT
    public:
        ImageSizeFinder(const QString &path, QObject *parent = 0);
        void run();

    Q_SIGNALS:
        void sizeFound(const QString &path, const QSize &size);

    private:
        QString m_path;
};

class BackgroundListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    BackgroundListModel(Plasma::Wallpaper *listener, QObject *parent);
    virtual ~BackgroundListModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Plasma::Package *package(int index) const;

    void reload();
    void reload(const QStringList &selected);
    void addBackground(const QString &path);
    QModelIndex indexOf(const QString &path) const;
    virtual bool contains(const QString &bg) const;

    void setWallpaperSize(const QSize& size);
    void setResizeMethod(Plasma::Wallpaper::ResizeMethod resizeMethod);

protected Q_SLOTS:
    void removeBackground(const QString &path);
    void showPreview(const KFileItem &item, const QPixmap &preview);
    void previewFailed(const KFileItem &item);
    void sizeFound(const QString &path, const QSize &s);
    void backgroundsFound(const QStringList &paths, const QString &token);
    void processPaths(const QStringList &paths);

private:
    QSize bestSize(Plasma::Package *package) const;

    Plasma::Wallpaper *m_structureParent;
    QList<Plasma::Package *> m_packages;
    QHash<Plasma::Package *, QSize> m_sizeCache;
    QHash<Plasma::Package *, QPixmap> m_previews;
    QHash<KUrl, QPersistentModelIndex> m_previewJobs;
    KDirWatch m_dirwatch;

    QSize m_size;
    Plasma::Wallpaper::ResizeMethod m_resizeMethod;
    QString m_findToken;
    QPixmap m_previewUnavailablePix;
};

class BackgroundFinder : public QThread
{
    Q_OBJECT

public:
    BackgroundFinder(Plasma::Wallpaper *structureParent, const QStringList &p);
    ~BackgroundFinder();

    QString token() const;

signals:
    void backgroundsFound(const QStringList &paths, const QString &token);

protected:
    void run();

private:
    Plasma::PackageStructure::Ptr m_structure;
    QStringList m_paths;
    QString m_token;
};

class BackgroundDelegate : public QAbstractItemDelegate
{
public:
    enum {
        AuthorRole = Qt::UserRole,
        ScreenshotRole,
        ResolutionRole
    };

    BackgroundDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

    static const int SCREENSHOT_SIZE = 128;
    static const int BLUR_INCREMENT = 9;
    static const int MARGIN = 6;

    void resetMaxHeight() { m_maxHeight = 0; }
    int m_maxHeight;
private:
    int m_maxWidth;
};

class RemoveButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit RemoveButton(QWidget* parent);
    virtual ~RemoveButton();
    virtual QSize sizeHint() const;

    /**
     * Resets the selection removeButton so that it is hidden and stays
     * visually invisible for at least one second after it is shown again.
     */
    void reset();
    void setItemName(const QString& name);
    QString itemName() const;

public slots:
    virtual void setVisible(bool visible);

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private slots:
    /**
     * Sets the alpha value for the fading animation and is
     * connected with m_fadingTimeLine.
     */
    void setFadingValue(int value);

    void refreshIcon();

private:
    void startFading();
    void stopFading();

private:
    bool m_isHovered;
    bool m_leftMouseButtonPressed;
    int m_fadingValue;
    QPixmap m_icon;
    QTimeLine* m_fadingTimeLine;
    QString m_itemName;
};

class RemoveButtonManager : public QObject
{
    Q_OBJECT

public:
    RemoveButtonManager(QAbstractItemView* parent, QStringList *list);
    virtual ~RemoveButtonManager();

signals:
    /** Is emitted if the selection has been changed by the removeButton button. */
    void selectionChanged();
    void removeClicked(QString item);

private slots:
    void slotEntered(const QModelIndex& index);
    void slotViewportEntered();
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    void removeButtonClicked();

private:
    const QModelIndex indexForUrl(const KUrl& url) const;

private:
    QAbstractItemView* m_view;
    RemoveButton* m_removeButton;
    bool m_connected;
    QStringList *m_removableWallpapers;
};
#endif
