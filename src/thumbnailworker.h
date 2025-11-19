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

class ThumbnailWorker : public QObject {
    Q_OBJECT
public:
    ThumbnailWorker(const QString &tempPath, int targetShort, QObject *parent=nullptr);
    ~ThumbnailWorker();
    void requestThumbnail(int index, QString filePath);
    int targetSize() { return m_targetShort; }
    void setTargetSize(int targetShort) { m_targetShort = targetShort; }

signals:
    void thumbnailReady(int index, QString thumbPath);

private:
    QThreadPool m_threadPool;
    int m_targetShort;
    QString m_tempPath;
};