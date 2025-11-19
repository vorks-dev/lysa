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

#include "photocontroller.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QSettings>
#include <QtConcurrent>
#include <QDebug>

PhotoController::PhotoController(AppSettings *settings, QObject *parent)
    : QObject(parent), m_settings(settings), m_model(settings), m_galleryModel(settings, m_model, this), m_directories(m_settings)
{

    connect(&m_directories, &DirectoryModel::activePathChanged,
            this, &PhotoController::fillPhotoModel);

    connect(m_settings, &AppSettings::settingChanged,
            this, &PhotoController::onSettingChanged);

    connect(this, &PhotoController::dirsFound, this, [this](const QStringList &dirs){
        for (const QString &path : dirs)
        m_directories.addDirectory(path);
    }, Qt::QueuedConnection);

    connect(this, &PhotoController::photosFound, this, [this](const QStringList &files){
                for (const QString &path : files)
                m_model.addPhoto(path);
                m_model.batchChangeFinished();
            }, Qt::QueuedConnection);

    QString rootFolder = m_settings->getValue("rootFolder").toString();
    setRootFolder(rootFolder);
}

PhotoController::~PhotoController() {
    ++m_dirGeneration;
    ++m_photoGeneration;
    m_dirScanFuture.cancel();
    m_photoScanFuture.cancel();
}

void PhotoController::onSettingChanged(const QString &id, const QVariant &value) {
    if (id == QStringLiteral("rootFolder")) {
        setRootFolder(value.toString());
        m_directories.setActivePath(m_rootFolder);
    }
}

void PhotoController::setRootFolder(const QString &folder) {
    if(m_rootFolder == folder) return;
    if(folder.isEmpty() || !QDir(folder).exists()) return;
    m_rootFolder = folder;
    loadFolder(m_rootFolder);
}

void PhotoController::loadFolder(const QString &folder) {
    QString openedDirectory = m_settings->getValue("openedDirectory").toString();
    if(openedDirectory.isEmpty()) m_directories.setActivePath(folder);
    else m_directories.setActivePath(openedDirectory);

    m_directories.clear();
    m_directories.setRootPath(folder);

    int gen = ++m_dirGeneration;
    QPointer<PhotoController> guard(this);

    if(m_dirScanFuture.isRunning()) m_dirScanFuture.cancel();

    m_dirScanFuture = QtConcurrent::run(&m_loadingPool, [folder, gen, guard]() {
        if (!guard) return;

        QStringList dirs;
        QDirIterator dirIt(folder, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (dirIt.hasNext()) {
            dirs << dirIt.next();
        }

        if (!guard || gen != guard->m_dirGeneration) return; // discard stale results

        emit guard->dirsFound(dirs);
    });
}

void PhotoController::fillPhotoModel(const QString &folder) {
    m_model.clear();

    int gen = ++m_photoGeneration;
    QPointer<PhotoController> guard(this);

    if(m_photoScanFuture.isRunning()) m_photoScanFuture.cancel();

    m_photoScanFuture = QtConcurrent::run(&m_loadingPool, [folder, gen, guard]() {
        if (!guard) return;

        QStringList files;
        QDirIterator it(folder,
                        {"*.jpg","*.jpeg","*.png","*.bmp"},
                        QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            files << it.next();
        }

        if (!guard || gen != guard->m_photoGeneration) return; // discard stale results

        emit guard->photosFound(files);
    });
}
