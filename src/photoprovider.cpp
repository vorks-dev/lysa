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

#include "photoprovider.h"
#include <QtConcurrent>
#include <QFuture>
#include <QImageReader>
#include <QMetaObject>
#include <QDir>
#include <QDebug>
#include <QPointer>
#include <QUuid>

PhotoProvider::PhotoProvider(const QString &filePath, const QString &thumbPath, const QFileInfo &info, const ExifData &exif, QThreadPool *threadPool, const QString &tempPath, QObject *parent)
    : QObject(parent), m_filePath(filePath), m_thumbPath(thumbPath), m_info(info), m_exif(exif), m_threadPool(threadPool), m_tempPath(tempPath), m_active(false)
{
    emit filePathChanged();
    emit thumbPathChanged();
    startLoading();
}

PhotoProvider::~PhotoProvider() {
    // Clean up preloaded image file if it exists
    if (!m_loadedPath.isEmpty() && QFile::exists(m_loadedPath)) {
        if (!QFile::remove(m_loadedPath))
            qWarning() << "Failed to delete preloaded image:" << m_loadedPath;
    }
}

void PhotoProvider::startLoading() {
    QPointer<PhotoProvider> that(this); // safe weak reference

    m_waiting = true;
    m_future = QtConcurrent::run(m_threadPool, [that]() {
        if (!that) return;

        QImageReader reader(that->m_filePath);
        reader.setAutoTransform(true);
        QImage img = reader.read();

        if (img.isNull()) {
            qWarning() << "Failed to load image:" << that->m_filePath;
            QMetaObject::invokeMethod(that, [that]() {
                if (that) {
                    that->m_waiting = false;
                    emit that->loadingFailed("Failed to load image");
                }
            }, Qt::QueuedConnection);
            return;
        }

        QString imgDirPath = that->m_tempPath + "/images";
        QDir tempDir(imgDirPath);

        if (!tempDir.exists() && !tempDir.mkpath(".")) {
            qWarning() << "Failed to create temp directory:" << tempDir.path();
            return;
        }

        QString imgPath = tempDir.filePath("img_" + QUuid::createUuid().toString(QUuid::Id128) + ".jpg");

        if (!img.save(imgPath, "JPG")) {
            qWarning() << "Failed to save preloaded image:" << imgPath;
            return;
        }

        QMetaObject::invokeMethod(that, [that, imgPath]() {
            if (that) {
                that->m_loadedPath = imgPath;
                that->m_waiting = false;
                emit that->imageReady(imgPath);
            }
        }, Qt::QueuedConnection);
    });
}
