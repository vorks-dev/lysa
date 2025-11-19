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
#include <QDateTime>
#include <QFileInfo>

enum class RationalType { Aperture, FocalLength, Shutter, Bias };
struct ExifValueRational {
    Q_GADGET
    Q_PROPERTY(QString formatted READ formatted)
    Q_ENUM(RationalType)

    public:
        ExifValueRational()
        : ExifValueRational(0, 1, RationalType::Shutter) {}
        ExifValueRational(int n, int d, RationalType t = RationalType::Shutter)
        : num(n), den(d), value(d != 0 ? double(n)/d : 0.0), type(t) {}

        int num = 0;
        int den = 1;
        double value = 0.0;
        RationalType type;

        QString formatted() const {
            switch (type) {
                case RationalType::Aperture:
                    return QString("f/%1").arg(value, 0, 'f', 1);
                case RationalType::FocalLength:
                    return QString("%1 mm").arg(value, 0, 'f', 1);
                case RationalType::Shutter:
                    if (num > 0 && num < den && den % num == 0)
                        return QString("%1/%2 s").arg(num).arg(den);
                    return QString("%1 s").arg(value, 0, 'f', 4);
                case RationalType::Bias:
                    return QString("%1 EV").arg(value, 0, 'f', 2);
            }
            return QString();
        }
};
inline bool operator==(const ExifValueRational &a, const ExifValueRational &b) {
    return a.num == b.num
        && a.den == b.den
        && a.value == b.value
        && a.type == b.type;
}
inline bool operator!=(const ExifValueRational &a, const ExifValueRational &b) {
    return !(a == b);
}

struct ExifData {
    Q_GADGET
    Q_PROPERTY(QString maker MEMBER maker CONSTANT)
    Q_PROPERTY(QString cameraModel MEMBER cameraModel CONSTANT)
    Q_PROPERTY(QString lensModel MEMBER lensModel CONSTANT)
    Q_PROPERTY(QDateTime dateTaken MEMBER dateTaken CONSTANT)
    Q_PROPERTY(int iso MEMBER iso CONSTANT)
    Q_PROPERTY(ExifValueRational aperture MEMBER aperture CONSTANT)
    Q_PROPERTY(ExifValueRational focalLength MEMBER focalLength CONSTANT)
    Q_PROPERTY(ExifValueRational exposureTime MEMBER exposureTime CONSTANT)
    Q_PROPERTY(ExifValueRational exposureBias MEMBER exposureBias CONSTANT)
    Q_PROPERTY(int flashFired MEMBER flashFired CONSTANT)
    Q_PROPERTY(QString software MEMBER software CONSTANT)
    Q_PROPERTY(int orientation MEMBER orientation CONSTANT)
    Q_PROPERTY(int width MEMBER width CONSTANT)
    Q_PROPERTY(int height MEMBER height CONSTANT)
    Q_PROPERTY(double gpsLatitude MEMBER gpsLatitude CONSTANT)
    Q_PROPERTY(double gpsLongitude MEMBER gpsLongitude CONSTANT)
    Q_PROPERTY(double gpsAltitude MEMBER gpsAltitude CONSTANT)

    public:
        // Camera / lens
        QString maker;
        QString cameraModel;
        QString lensModel;

        // Date
        QDateTime dateTaken;

        // ISO
        int iso;

        // Aperture
        ExifValueRational aperture;

        // Focal Length
        ExifValueRational focalLength;

        // Exposure
        ExifValueRational exposureTime;
        ExifValueRational exposureBias;

        // Flash
        int flashFired;

        // Software
        QString software;

        // Orientation
        int orientation;

        // Dimensions
        int width;
        int height;

        // GPS
        double gpsLatitude;
        double gpsLongitude;
        double gpsAltitude;
};

struct FileData {
    Q_GADGET
    Q_PROPERTY(QString fileName MEMBER fileName CONSTANT)
    Q_PROPERTY(QString baseName MEMBER baseName CONSTANT)
    Q_PROPERTY(QString suffix MEMBER suffix CONSTANT)
    Q_PROPERTY(QString filePath MEMBER filePath CONSTANT)
    Q_PROPERTY(QString directoryPath MEMBER directoryPath CONSTANT)

    public:
        FileData() = default;
        explicit FileData(const QFileInfo &info)
        : fileName(info.fileName()), baseName(info.baseName()), suffix(info.suffix()), filePath(info.absoluteFilePath()), directoryPath(info.absolutePath()) {}

        QString fileName;
        QString baseName;
        QString suffix;
        QString filePath;
        QString directoryPath;
};
