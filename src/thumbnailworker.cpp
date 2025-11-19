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

#include "thumbnailworker.h"
#include <QtConcurrent>
#include <QImageReader>
#include <QDir>
#include <QDebug>

ThumbnailWorker::ThumbnailWorker(const QString &tempPath, int targetShort, QObject *parent) : QObject(parent), m_tempPath(tempPath), m_targetShort(targetShort) {}
ThumbnailWorker::~ThumbnailWorker() {
    m_threadPool.clear();
    m_threadPool.waitForDone();
}

void ThumbnailWorker::requestThumbnail(int index, QString filePath) {
    QFuture<void> future = QtConcurrent::run(&m_threadPool, [=]() {
        QImageReader reader(filePath);
        reader.setAutoTransform(true);
        QImage img = reader.read();
        if (img.isNull()) return;

        // Scale so the shorter side = 200px, keeping aspect ratio
        int w = img.width();
        int h = img.height();
        int targetShort = m_targetShort;
        QSize scaledSize = (w < h)
                        ? QSize(targetShort, targetShort * h / w)
                        : QSize(targetShort * w / h, targetShort);
        QImage thumb = img.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QString thumbDirPath = m_tempPath + "/thumbnails";
        QDir tempDir(thumbDirPath);

        if (!tempDir.exists() && !tempDir.mkpath(".")) {
            qWarning() << "Failed to create temp directory:" << tempDir.path();
            return;
        }

        QString thumbPath = tempDir.filePath("thumb_" + QUuid::createUuid().toString(QUuid::Id128) + ".jpg");
        thumb.save(thumbPath, "JPG");
        emit thumbnailReady(index, thumbPath);
    });

}