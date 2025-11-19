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

#include "gallerymodel.h"

GalleryModel::GalleryModel(AppSettings *settings, PhotoModel& sourceModel, QObject *parent)
    : QSortFilterProxyModel(parent), m_settings(settings), m_source(sourceModel)
{
    setDynamicSortFilter(false); //Handled by custom connection to prevent unnecessary re-sorting
    loadSettings();

    setSourceModel(&m_source);

    connect(m_settings, &AppSettings::settingChanged,
            this, &GalleryModel::onSettingChanged);

    connect(&m_source, &QAbstractItemModel::dataChanged,
        this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles) {
        emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
        if (roles.isEmpty() || roles.contains(m_sortMode)) sort();
    });

    // Forward model changes from source
    connect(&m_source, &PhotoModel::modelChanged, this, &GalleryModel::modelChanged);
    connect(&m_source, &PhotoModel::modelChanged, this, &GalleryModel::sort);

    connect(&m_source, &PhotoModel::loadingStarted, this, [this](const QList<int> &roles) {
        if(!roles.isEmpty() && !roles.contains(m_sortMode)) return;
        emit loadingStarted();
    });
    connect(&m_source, &PhotoModel::loadingFinished, this, &GalleryModel::loadingFinished);

    connect(this, &QSortFilterProxyModel::layoutChanged, this, &GalleryModel::modelChanged);
    connect(this, &QSortFilterProxyModel::modelReset, this, &GalleryModel::modelChanged);
}

void GalleryModel::onSettingChanged(const QString &id, const QVariant &value) {
    if(id == QStringLiteral("gallerySortMode")) {
        setSortMode(value.toString());
    }
    else if(id == QStringLiteral("gallerySortAscending")) {
        setSortAscending(value.toBool());
    }
}

void GalleryModel::loadSettings() {
    setSortMode(m_settings->getValue("gallerySortMode").toString());
    setSortAscending(m_settings->getValue("gallerySortAscending").toBool());
}

void GalleryModel::setSortAscending(bool asc) {
    if (m_sortAscending == asc) return;
    m_sortAscending = asc;
    m_settings->setValue("gallerySortAscending", m_sortAscending);
    emit sortAscendingChanged();
    sort();
}

QHash<QString, int> GalleryModel::sortRoles() const {
    return {
        { "size", PhotoModel::FileSizeRole },
        { "date", PhotoModel::DateRole },
        { "exposure", PhotoModel::ExposureTimeRole },
        { "camera", PhotoModel::CameraModelRole },
        { "iso", PhotoModel::IsoRole },
        { "focalLength", PhotoModel::FocalLengthRole }
    };
}

void GalleryModel::setSortMode(QString mode) {
    int role = sortRoles().value(mode, PhotoModel::DateRole);
    if(m_sortMode == role) return;
    m_sortMode = role;
    m_settings->setValue("gallerySortMode", mode);
    sort();
}

bool GalleryModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    int dataRole = m_sortMode;
    const QVariant leftData = m_source.data(left, dataRole);
    const QVariant rightData = m_source.data(right, dataRole);

    switch (dataRole) {
        case PhotoModel::FilePathRole: {
            const QString l = leftData.toString();
            const QString r = rightData.toString();
            const bool less = l.localeAwareCompare(r) < 0;
            return less;
        }
        case PhotoModel::FileSizeRole: {
            const qint64 l = leftData.toLongLong();
            const qint64 r = rightData.toLongLong();
            return l < r;
        }
        case PhotoModel::DateRole: {
            const QDateTime l = leftData.toDateTime();
            const QDateTime r = rightData.toDateTime();
            if(!l.isValid() || !r.isValid()) return false;
            return l < r;
        }
        case PhotoModel::ExposureTimeRole: {
            const double l = leftData.toDouble();
            const double r = rightData.toDouble();
            return l < r;
        }
        case PhotoModel::CameraModelRole: {
            const QString l = leftData.toString();
            const QString r = rightData.toString();
            const bool less = l.localeAwareCompare(r) < 0;
            return less;
        }
        case PhotoModel::IsoRole: {
            const int l = leftData.toInt();
            const int r = rightData.toInt();
            return l < r;
        }
        case PhotoModel::FocalLengthRole: {
            const double l = leftData.toDouble();
            const double r = rightData.toDouble();
            return l < r;
        }
        default:
            return false;
    }
}

void GalleryModel::loadThumbnails(int firstIndex, int lastIndex, int itemsPerRow, int preloadDirection) {
    if (firstIndex < 0 || lastIndex < 0) return;

    // Ensure visible ones are loaded
    _loadThumbnails(firstIndex, lastIndex);

    // Preload adjacent rows (if preloadDirection = 0: both directions)
    int preloadRows = 5;
    if(preloadDirection >= 0) _loadThumbnails(lastIndex + 1, lastIndex + itemsPerRow * preloadRows);
    if(preloadDirection <= 0) _loadThumbnails(firstIndex - 1, firstIndex - itemsPerRow * preloadRows);

    if(firstIndex - lastIndex == 0) return; //Don't delete old if only singular preload
    int cacheRowsDistance = 30;
    clearOldThumbnails(firstIndex - itemsPerRow * preloadRows, lastIndex + itemsPerRow * preloadRows, cacheRowsDistance * preloadRows);
}

void GalleryModel::_loadThumbnails(int firstIndex, int lastIndex) {
    for (int i = firstIndex; i <= lastIndex; ++i) {
        QModelIndex sourceIndex = mapToSource(index(i, 0));
        m_source.loadThumbnail(sourceIndex.row());
    }
}

void GalleryModel::clearOldThumbnails(int firstPreloaded, int lastPreloaded, int maxCacheDistance) {
    _clearThumbnails(0, firstPreloaded - 1 - maxCacheDistance);
    _clearThumbnails(lastPreloaded + 1 + maxCacheDistance, m_source.rowCount() - 1);
}

void GalleryModel::_clearThumbnails(int from, int to) {
    for(int i = from; i <= to; ++i) {
        QModelIndex sourceIndex = mapToSource(index(i, 0));
        m_source.clearThumbnail(sourceIndex.row());
    }
}

int GalleryModel::getIndex(QString filePath) {
    int sourceIndex = m_source.getIndex(filePath);
    if (sourceIndex < 0) return -1;

    QModelIndex sourceIdx = m_source.index(sourceIndex, 0);
    QModelIndex proxyIdx = mapFromSource(sourceIdx);
    return proxyIdx.row();
}

PhotoProvider* GalleryModel::getProvider(int idx) {
    if (idx < 0 || idx >= rowCount()) return nullptr;
    // Requested provider (active)
    QModelIndex sourceIdx = mapToSource(index(idx, 0));
    PhotoProvider* provider = m_source.getProvider(sourceIdx.row(), idx);
    if(provider) provider->setActive(true);

    // Preload neighbors (not active)
    int providerPreloadDistance = 2;
    for(int i = idx - providerPreloadDistance; i <= idx + providerPreloadDistance; ++i) {
        if(i == idx) continue; //Already loaded (active)
        if(i < 0) continue;
        if(i >= rowCount()) break;
        QModelIndex sourceI = mapToSource(index(i, 0));
        PhotoProvider* preload = m_source.getProvider(sourceI.row(), i);
        if(preload) preload->setActive(false);
    }

    // Prune all providers that are not needed to free-up memory
    m_source.pruneProviders();

    return provider;
}
