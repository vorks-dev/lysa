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
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>

class FileService : public QObject {
    Q_OBJECT

public:
    explicit FileService(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QString resolveRelativePath(const QString &relativePath) {
        return QDir(QCoreApplication::applicationDirPath()).filePath(relativePath);
    }

    Q_INVOKABLE QString readTxt(const QString &relativePath) {
        QString fullPath = resolveRelativePath(relativePath);
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString("Could not open file: %1").arg(fullPath);

        QTextStream in(&file);
        return in.readAll();
    }
};
