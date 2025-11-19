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

#include "photomodel.h"
#include "photoprovider.h"
#include <QUrl>
#include <QDebug>
#include <QDir>

PhotoModel::PhotoModel(AppSettings *settings, QObject *parent)
    : QAbstractListModel(parent), m_settings(settings), m_tempDir(new QTemporaryDir(ensureBasePath() + "/XXXXXX")), m_worker(m_tempDir->path(), m_settings->getValue("galleryTargetWidth").toInt(), this), m_exif(this)
{
    if (!m_tempDir->isValid()) qWarning() << "Failed to create temporary directory!";

    m_providerPool.setMaxThreadCount(2);

    connect(m_settings, &AppSettings::settingChanged,
            this, &PhotoModel::onSettingChanged);

    connect(&m_exif, &ExifRegistry::dataReady,
            this, &PhotoModel::exifReady);

    connect(&m_worker, &ThumbnailWorker::thumbnailReady,
            this, &PhotoModel::setThumbnail);
}

PhotoModel::~PhotoModel() {
    qDeleteAll(m_providers);
    m_providers.clear();

    m_providerPool.clear();
    m_providerPool.waitForDone();
}

void PhotoModel::onSettingChanged(const QString &id, const QVariant &value) {
    if (id == QStringLiteral("galleryTargetWidth"))
        setThumbnailSize(value.toInt());
}

int PhotoModel::rowCount(const QModelIndex &) const {
    return m_photos.size();
}

bool PhotoModel::isValidIndex(QModelIndex index) const {
    return index.isValid() && index.row() < m_photos.size();
}
bool PhotoModel::isValidIndex(int index) const {
    return index >= 0 && index < m_photos.size();
}

QVariant PhotoModel::data(const QModelIndex &index, int role) const {
    if (!isValidIndex(index)) return {};

    const PhotoItem &photo = m_photos[index.row()];

    switch (role) {
    case FilePathRole:
        return photo.filePath;

    case ThumbPathRole:
        if (!photo.thumbPath.isEmpty()) return photo.thumbPath;
        return "";

    case FileSizeRole:
        return photo.info.size();

    case DateRole: {
        QDateTime exifDate = m_exif.getData(photo.filePath).dateTaken;
        if(exifDate.isValid()) return exifDate;
        QDateTime birthTime = photo.info.birthTime();
        if(birthTime.isValid()) return birthTime;
        return photo.info.lastModified();
    }

    case ExposureTimeRole:
        return m_exif.getData(photo.filePath).exposureTime.value;

    case CameraModelRole:
        return m_exif.getData(photo.filePath).cameraModel;

    case IsoRole:
        return m_exif.getData(photo.filePath).iso;
    
    case FocalLengthRole:
        return m_exif.getData(photo.filePath).focalLength.value;

    default:
        return {};
    }
}

QHash<int, QByteArray> PhotoModel::roleNames() const {
    return {
        { FilePathRole, "filePath" },
        { ThumbPathRole, "thumbPath" },
        { FileSizeRole, "fileSize"},
        { DateRole, "fileDate"}
    };
}

void PhotoModel::clear() {
    beginResetModel();

    // Delete all existing providers
    qDeleteAll(m_providers);
    m_providers.clear();

    // Clear thumbnails and associated data
    for (PhotoItem &photo : m_photos) {
        if (!photo.thumbPath.isEmpty() && QFile::exists(photo.thumbPath)) {
            if (!QFile::remove(photo.thumbPath)) {
                qWarning() << "Failed to delete thumbnail:" << photo.thumbPath;
            }
        }
    }

    // Clear the model data and index map
    m_photos.clear();
    m_indexMap.clear();

    endResetModel();

    // Clean worker queue and thread pool
    m_providerPool.clear();
    m_providerPool.waitForDone();

    // Clean the temp directory
    if (m_tempDir && m_tempDir->isValid()) {
        QDir temp(m_tempDir->path());
        temp.removeRecursively();
    }
}

int PhotoModel::addPhoto(const QString &filePath) {
    beginInsertRows(QModelIndex(), m_photos.size(), m_photos.size());
    m_photos.append(PhotoItem(filePath));
    int index = m_photos.size() - 1;
    m_indexMap.insert(filePath, index);
    endInsertRows();
    m_exif.requestData(index, filePath);
    return index;
}

void PhotoModel::batchChangeFinished() {
    emit loadingStarted({DateRole});
    emit modelChanged();
    m_exif.startProcessing();
}

void PhotoModel::exifReady(int firstIndex, int lastIndex) {
    if (isValidIndex(firstIndex) && isValidIndex(lastIndex))
        emit dataChanged(this->index(firstIndex), this->index(lastIndex), {DateRole, ExposureTimeRole});
    emit loadingFinished();
}

void PhotoModel::setThumbnailSize(int targetShort) {
    if(m_worker.targetSize() == targetShort) return;

    m_worker.setTargetSize(targetShort);
    // Lazy-recalculate existing thumbnails
    for(int i = 0; i < m_photos.size(); ++i) {
        const PhotoItem &photoItem = m_photos[i];
        if(photoItem.requested && !photoItem.thumbPath.isEmpty()) {
            m_worker.requestThumbnail(i, photoItem.filePath);
        }
    }
}

void PhotoModel::loadThumbnail(int index) {
    if(!isValidIndex(index)) return;
    PhotoItem& photoItem = m_photos[index];
    if(photoItem.requested) return;
    m_worker.requestThumbnail(index, photoItem.filePath);
    photoItem.requested = true;
}

void PhotoModel::clearThumbnail(int index) {
    if(!isValidIndex(index)) return;
    PhotoItem& photoItem = m_photos[index];
    if(photoItem.thumbPath.isEmpty()) return;
    if(!QFile::exists(photoItem.thumbPath)) {
        photoItem.thumbPath.clear();
        photoItem.requested = false;
        return;
    }
    if (!QFile::remove(photoItem.thumbPath))
        qWarning() << "Failed to delete preloaded image:" << photoItem.thumbPath;
}

void PhotoModel::setThumbnail(int index, const QString &thumbPath) {
    if (!isValidIndex(index)) return;

    m_photos[index].thumbPath = thumbPath;
    emit dataChanged(this->index(index), this->index(index), {ThumbPathRole});
}

int PhotoModel::getIndex(QString filePath) {
    return m_indexMap.value(filePath, -1);
}

void PhotoModel::pruneProviders() {
    QSet<int> keepIndices;

    // Find all indices that should be kept
    for (auto it = m_providers.cbegin(); it != m_providers.cend(); ++it) {
        int idx = it.key();
        PhotoProvider* provider = it.value();

        if(provider->active()) {
            keepIndices << idx << idx - 1 << idx - 2 << idx + 1 << idx + 2;  //Keep provider and its neighbors
        }
        else if(provider->waiting()) keepIndices << idx; //Keep if provider is waiting for a Image-Preload to ensure proper preload deletion later
    }

    // Remove all others
    QList<int> toRemove;
    for (auto it = m_providers.cbegin(); it != m_providers.cend(); ++it) {
        if (!keepIndices.contains(it.key())) {
            toRemove << it.key();
        }
    }

    for (int idx : toRemove) {
        delete m_providers.take(idx);
    }
}

PhotoProvider* PhotoModel::getProvider(int index, int viewIndex) {
    if (!isValidIndex(index)) return nullptr;

    PhotoProvider* provider = nullptr;

    if (m_providers.contains(viewIndex)) {
        provider = m_providers.value(viewIndex);
    } else {
        PhotoItem photo = m_photos[index];
        provider = new PhotoProvider(photo.filePath, photo.thumbPath, photo.info, m_exif.getData(photo.filePath), &m_providerPool, m_tempDir->path(), this);
        m_providers.insert(viewIndex, provider);
    }

    return provider;
}
