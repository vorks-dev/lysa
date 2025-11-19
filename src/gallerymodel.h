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
#include "photomodel.h"
#include "photoprovider.h"
#include <QSortFilterProxyModel>

class GalleryModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(bool sortAscending READ sortAscending WRITE setSortAscending NOTIFY sortAscendingChanged)

public:
    explicit GalleryModel(AppSettings *settings, PhotoModel& sourceModel, QObject *parent = nullptr);

    void sort() { QSortFilterProxyModel::sort(0, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder); }

    void onSettingChanged(const QString &id, const QVariant &value);
    void loadSettings();

    bool sortAscending() const { return m_sortAscending; }
    void setSortAscending(bool asc);

    QHash<QString, int> sortRoles() const;
    Q_INVOKABLE void setSortMode(QString mode);

    Q_INVOKABLE int size() { return m_source.rowCount(); }

    Q_INVOKABLE void loadThumbnails(int firstIndex, int lastIndex, int itemsPerRow, int preloadDirection);
    void _loadThumbnails(int firstIndex, int lastIndex);
    void clearOldThumbnails(int firstPreloaded, int lastPreloaded, int maxCacheDistance);
    void _clearThumbnails(int from, int to);
    Q_INVOKABLE int getIndex(QString filePath);
    Q_INVOKABLE PhotoProvider* getProvider(int index);

signals:
    void sortAscendingChanged();
    void modelChanged();
    void loadingStarted();
    void loadingFinished();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    AppSettings* m_settings;
    PhotoModel& m_source;
    bool m_sortAscending = true;
    int m_sortMode;
};
