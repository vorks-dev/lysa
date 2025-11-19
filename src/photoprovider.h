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
#include <QString>
#include <QFileInfo>
#include <QFuture>
#include "structs.h"

class PhotoProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString loadedPath READ loadedPath NOTIFY imageReady)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)
    Q_PROPERTY(QString thumbPath READ thumbPath NOTIFY thumbPathChanged)
    Q_PROPERTY(FileData fileData READ fileData NOTIFY fileDataChanged)
    Q_PROPERTY(ExifData exifData READ exifData NOTIFY exifDataChanged)

public:
    explicit PhotoProvider(const QString &filePath, const QString &thumbPath, const QFileInfo &info, const ExifData &exif, QThreadPool *threadPool, const QString &tempPath, QObject *parent = nullptr);
    ~PhotoProvider();

    bool active() const { return m_active; }
    Q_INVOKABLE void setActive(bool value) { m_active = value; }

    bool waiting() const { return m_waiting; }

    QString loadedPath() const { return m_loadedPath; }
    QString filePath() const { return m_filePath; }
    QString thumbPath() const { return m_thumbPath; }
    FileData fileData() const { return FileData(m_info); }
    ExifData exifData() const { return m_exif; }

signals:
    void imageReady(const QString &path);
    void loadingFailed(const QString &error);

    void filePathChanged();
    void thumbPathChanged();
    void fileDataChanged();
    void exifDataChanged();

private:
    QThreadPool *m_threadPool = nullptr;
    QString m_tempPath;
    bool m_active;
    bool m_waiting = false;
    QString m_filePath;
    QString m_loadedPath;
    QString m_thumbPath;
    QFileInfo m_info;
    ExifData m_exif;
    void startLoading();
    QFuture<void> m_future;
};
