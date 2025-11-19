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

#include "exifregistry.h"
#include <QtConcurrent>
#include <QDebug>

ExifRegistry::ExifRegistry(QObject *parent)
    : QObject(parent) {}

ExifRegistry::~ExifRegistry() {
    m_threadPool.clear();
    m_threadPool.waitForDone();
}

ExifData ExifRegistry::getData(const QString &filePath) const {
    QMutexLocker locker(&m_mutex);
    return m_resultMap.value(filePath);
}

void ExifRegistry::requestData(int index, const QString &filePath) {
    QMutexLocker locker(&m_mutex);
    if(m_resultMap.contains(filePath)) return;
    m_pathList.append(filePath);
    if (m_firstRequestedIndex < 0) m_firstRequestedIndex = index;
    m_lastRequestedIndex = index;
}

void ExifRegistry::startProcessing() {
    {
        QMutexLocker locker(&m_mutex);
        if (m_pathList.isEmpty()) {
            clearQueue();
            return;
        }
    }

    QFuture<void> future = QtConcurrent::map(&m_threadPool, m_pathList, [this](const QString &filePath) {
        try {
            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(filePath.toStdString());
            if (!image) return;

            image->readMetadata();
            Exiv2::ExifData &exifData = image->exifData();
            if (exifData.empty()) return;

            ExifData data;

            // Camera / lens
            data.maker = getExifString(exifData, "Exif.Image.Make");
            data.cameraModel = getExifString(exifData, "Exif.Image.Model");
            data.lensModel = getExifString(exifData, {"Exif.Photo.LensModel", "Exif.Photo.LensSpecification", "Exif.Photo.LensMake"});

            // Date
            data.dateTaken = QDateTime::fromString(getExifString(exifData, {"Exif.Photo.DateTimeOriginal", "Exif.Photo.CreationDate", "Exif.Image.DateTime"}), "yyyy:MM:dd HH:mm:ss");

            // ISO
            data.iso = getExifInt(exifData, {"Exif.Photo.ISOSpeed", "Exif.Photo.SensitivityType", "Exif.Photo.ISOSpeedRatings"});

            // Aperture
            data.aperture = getExifRational(exifData, "Exif.Photo.FNumber", RationalType::Aperture);

            // Focal Length
            data.focalLength = getExifRational(exifData, "Exif.Photo.FocalLength", RationalType::FocalLength);

            // Exposure
            data.exposureTime = getExifRational(exifData, "Exif.Photo.ExposureTime", RationalType::Shutter);
            data.exposureBias = getExifRational(exifData, "Exif.Photo.ExposureBiasValue", RationalType::Bias);

            // Flash
            data.flashFired = getExifInt(exifData, "Exif.Photo.Flash");

            // Software
            data.software = getExifString(exifData, "Exif.Image.Software");

            // Orientation
            data.orientation = getExifInt(exifData, "Exif.Image.Orientation");

            // Dimensions
            data.width = getExifInt(exifData, {"Exif.Photo.PixelXDimension", "Exif.Image.ImageWidth"});
            data.height = getExifInt(exifData, {"Exif.Photo.PixelYDimension", "Exif.Image.ImageLength"});

            // GPS
            data.gpsLatitude = applyGpsRef(getExifGpsCoord(exifData, "Exif.GPSInfo.GPSLatitude"), getExifString(exifData, "Exif.GPSInfo.GPSLatitudeRef"));
            data.gpsLongitude = applyGpsRef(getExifGpsCoord(exifData, "Exif.GPSInfo.GPSLongitude"), getExifString(exifData, "Exif.GPSInfo.GPSLongitudeRef"));
            data.gpsAltitude = getExifGpsAltitude(exifData);


            {
                QMutexLocker locker(&m_mutex);
                m_resultMap[filePath] = data;
            }
        }
        catch (const Exiv2::Error &e) {
            //qWarning() << "Failed to load EXIF metadata for" << filePath << ":" << e.what();
        }
    });

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, &ExifRegistry::clearQueue);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QObject::deleteLater);
    watcher->setFuture(future);
}

void ExifRegistry::clearQueue() {
    emit dataReady(m_firstRequestedIndex, m_lastRequestedIndex);
    m_firstRequestedIndex = -1;
    m_lastRequestedIndex = -1;
    m_pathList.clear();
}

QString ExifRegistry::getExifString(const Exiv2::ExifData &exifData, const char* key) {
    auto it = exifData.findKey(Exiv2::ExifKey(key));
    if (it != exifData.end() && it->size() > 0) {
        return QString::fromStdString(it->toString());
    }
    return QString();
}
QString ExifRegistry::getExifString(const Exiv2::ExifData &exifData, std::initializer_list<const char*> keys) {
    for (const char* key : keys) {
        QString val = getExifString(exifData, key);
        if (!val.isEmpty()) return val;
    }
    return QString();
}

int ExifRegistry::getExifInt(const Exiv2::ExifData &exifData, const char* key) {
    auto it = exifData.findKey(Exiv2::ExifKey(key));
    if (it != exifData.end() && it->size() > 0) {
        bool ok = false;
        int val = QString::fromStdString(it->toString()).toInt(&ok);
        return ok ? val : 0;
    }
    return 0;
}
int ExifRegistry::getExifInt(const Exiv2::ExifData &exifData, std::initializer_list<const char*> keys) {
    for (const char* key : keys) {
        int val = getExifInt(exifData, key);
        if (val != 0) return val;
    }
    return 0;
}

double ExifRegistry::getExifDouble(const Exiv2::ExifData &exifData, const char* key) {
    auto it = exifData.findKey(Exiv2::ExifKey(key));
    if (it != exifData.end() && it->size() > 0) {
        bool ok = false;
        double val = QString::fromStdString(it->toString()).toDouble(&ok);
        return ok ? val : 0.0;
    }
    return 0.0;
}
double ExifRegistry::getExifDouble(const Exiv2::ExifData &exifData, std::initializer_list<const char*> keys) {
    for (const char* key : keys) {
        double val = getExifDouble(exifData, key);
        if (val != 0.0) return val;
    }
    return 0.0;
}

double ExifRegistry::getExifGpsCoord(const Exiv2::ExifData &exifData, const char* key) {
    auto it = exifData.findKey(Exiv2::ExifKey(key));
    if (it == exifData.end() || it->count() < 3) return 0.0;

    const Exiv2::Value& v = it->value();

    auto getR = [&](size_t index) -> double {
        if (index >= v.count()) return 0.0;
        auto r = v.toRational(index);
        if (r.second == 0) return 0.0;
        return static_cast<double>(r.first) / r.second;
    };

    double deg = getR(0);
    double min = getR(1);
    double sec = getR(2);

    return dmsToDecimal(deg, min, sec);
}
double ExifRegistry::dmsToDecimal(double d, double m, double s) {
    double sign = d < 0 ? -1.0 : 1.0;
    return sign * (std::abs(d) + m / 60.0 + s / 3600.0);
}

double ExifRegistry::applyGpsRef(double value, const QString &ref) {
    if (ref == "S" || ref == "W")
        return -value;
    return value;
}

double ExifRegistry::getExifGpsAltitude(const Exiv2::ExifData &data) {
    auto it = data.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"));
    if (it == data.end() || it->count() == 0) return 0.0;

    auto r = it->value().toRational();
    double alt = static_cast<double>(r.first) / (r.second ? r.second : 1);

    auto refIt = data.findKey(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"));
    if (refIt != data.end()) {
        int ref = refIt->toFloat();
        if (ref == 1) alt = -alt; // below sea level
    }
    return alt;
}

ExifValueRational ExifRegistry::getExifRational(const Exiv2::ExifData& data, const char* key, RationalType format) {
    auto it = data.findKey(Exiv2::ExifKey(key));
    if (it == data.end() || it->count() == 0)
        return ExifValueRational(0, 1, format);

    const Exiv2::Value& v = it->value();

    if (v.typeId() == Exiv2::unsignedRational || v.typeId() == Exiv2::signedRational) {
        auto r = v.toRational();
        int num = r.first;
        int den = r.second ? r.second : 1;
        return ExifValueRational(num, den, format);
    }

    // Fallback: parse as double
    bool ok = false;
    double val = QString::fromStdString(v.toString()).toDouble(&ok);
    if (!ok) return ExifValueRational(0, 1, format);

    // Construct from double by converting to rational
    ExifValueRational out(1, 1, format);
    out.value = val;
    return out;
}
