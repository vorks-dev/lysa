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
#include <QString>
#include <QDateTime>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <exiv2/exiv2.hpp>
#include "structs.h"

class ExifRegistry : public QObject {
    Q_OBJECT
public:
    ExifRegistry(QObject *parent=nullptr);
    ~ExifRegistry();
    ExifData getData(const QString &filePath) const;
    void requestData(int index, const QString &filePath);
    void startProcessing();

signals:
    void dataReady(int firstIndex, int lastIndex);

private:
    mutable QMutex m_mutex;
    QThreadPool m_threadPool;
    QHash<QString, ExifData> m_resultMap;
    QVector<QString> m_pathList;
    int m_firstRequestedIndex = -1;
    int m_lastRequestedIndex = -1;

    void clearQueue();

    QString getExifString(const Exiv2::ExifData &exifData, const char* key);
    QString getExifString(const Exiv2::ExifData &exifData, std::initializer_list<const char*> keys);
    int getExifInt(const Exiv2::ExifData &exifData, const char* key);
    int getExifInt(const Exiv2::ExifData &exifData, std::initializer_list<const char*> keys);
    double getExifDouble(const Exiv2::ExifData &exifData, const char* key);
    double getExifDouble(const Exiv2::ExifData &exifData, std::initializer_list<const char*> keys);
    double getExifGpsCoord(const Exiv2::ExifData &exifData, const char* key);
    double dmsToDecimal(double d, double m, double s);
    double applyGpsRef(double value, const QString&ref);
    double getExifGpsAltitude(const Exiv2::ExifData &data);
    ExifValueRational getExifRational(const Exiv2::ExifData& data, const char* key, RationalType format);
};