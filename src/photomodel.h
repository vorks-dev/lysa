/*
* Lysa - Photo Organizer
* Copyright (C) 2025 vorks. DEV (Jeremy Voß)
* 
* This file is part of Lysa.
* 
* Lysa is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License version 3
* as published by the Free Software Foundation.
* 
* Lysa is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License 
* along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include <QThreadPool>
#include <QTemporaryDir>
#include "appsettings.h"
#include "structs.h"
#include "exifregistry.h"
#include "thumbnailworker.h"

class ThumbnailWorker;
class PhotoProvider;

struct PhotoItem {
    QString filePath;
    QString thumbPath;
    bool requested = false; // has thumbnail generation been requested yet
    QFileInfo info;
    ExifData exif;

    PhotoItem() = default;
    PhotoItem(const QString &path, const QString &thumb = QString(), bool req = false)
        : filePath(path), thumbPath(thumb), requested(req), info(path) {}
};

class PhotoModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { FilePathRole = Qt::UserRole + 1, ThumbPathRole, FileSizeRole, DateRole, ExposureTimeRole, CameraModelRole, IsoRole, FocalLengthRole };

    explicit PhotoModel(AppSettings *settings, QObject *parent = nullptr);
    ~PhotoModel();

    static QString ensureBasePath() {
        QString base = QDir::tempPath() + "/vorks/lysa";
        QDir().mkpath(base);
        return base;
    }

    void onSettingChanged(const QString &id, const QVariant &value);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void clear();

    int addPhoto(const QString &filePath);
    void batchChangeFinished();
    void exifReady(int firstIndex, int lastIndex);
    void setThumbnailSize(int targetShort);
    void loadThumbnail(int index);
    void clearThumbnail(int index);
    void setThumbnail(int index, const QString &thumbPath);

    int getIndex(QString filePath);
    PhotoProvider* getProvider(int index, int viewIndex);
    void pruneProviders();

signals:
    void modelChanged();
    void loadingStarted(const QList<int> &roles);
    void loadingFinished();

private:
    AppSettings* m_settings;
    QScopedPointer<QTemporaryDir> m_tempDir;
    ThumbnailWorker m_worker;
    ExifRegistry m_exif;
    QThreadPool m_providerPool;
    QVector<PhotoItem> m_photos;
    QHash<QString, int> m_indexMap;

    QHash<int, PhotoProvider*> m_providers;
    bool isValidIndex(QModelIndex index) const;
    bool isValidIndex(int index) const;
};
