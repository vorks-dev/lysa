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
#include <QObject>
#include <QThreadPool>
#include "photomodel.h"
#include "gallerymodel.h"
#include "thumbnailworker.h"
#include "directorymodel.h"
#include "appsettings.h"

class PhotoController : public QObject {
    Q_OBJECT
public:
    explicit PhotoController(AppSettings *settings, QObject *parent = nullptr);
    ~PhotoController();

    void onSettingChanged(const QString &id, const QVariant &value);

    void setRootFolder(const QString &folder);
    Q_INVOKABLE void loadFolder(const QString &folder);
    void fillPhotoModel(const QString &folder);

    GalleryModel* galleryModel() { return &m_galleryModel; }
    DirectoryModel* dirs() { return &m_directories; }

signals:
    void dirsFound(QStringList dirs);
    void photosFound(QStringList files);

private:
    AppSettings *m_settings; // injected from main.cpp
    PhotoModel m_model;
    GalleryModel m_galleryModel;
    DirectoryModel m_directories;

    QThreadPool m_loadingPool;
    QFuture<void> m_dirScanFuture;
    QFuture<void> m_photoScanFuture;
    std::atomic<int> m_dirGeneration {0};
    std::atomic<int> m_photoGeneration {0};

    QString m_rootFolder;
};
