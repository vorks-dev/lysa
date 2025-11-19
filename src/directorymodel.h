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
#include <QAbstractItemModel>
#include <QString>
#include <QVector>
#include <QPointer>
#include "appsettings.h"

struct DirectoryItem {
    QString path;
    QString name;
    DirectoryItem* parent = nullptr;
    QVector<DirectoryItem*> children;

    ~DirectoryItem() {
        qDeleteAll(children);
    }
};

class DirectoryModel : public QAbstractItemModel {
    Q_OBJECT
    Q_PROPERTY(QString activePath READ activePath WRITE setActivePath NOTIFY activePathChanged)
    Q_PROPERTY(QString rootPath READ rootPath NOTIFY rootPathChanged)
public:
    explicit DirectoryModel(AppSettings *settings, QObject *parent = nullptr);
    ~DirectoryModel() override;

    void clear();

    enum Roles { PathRole = Qt::UserRole + 1, NameRole };

    // Basic model overrides
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Public API
    void addDirectory(const QString &path, DirectoryItem *parentItem = nullptr);

    Q_INVOKABLE void setActivePath(const QString &path);
    QString activePath() const { return m_activePath; }
    void setRootPath(const QString &path) { m_rootPath = path; }
    QString rootPath() const { return m_rootPath; }

    Q_INVOKABLE QStringList parentPathChain(const QString &path) const;
    Q_INVOKABLE QString getLongName(const QString &path) const;

signals:
    void activePathChanged(QString path);
    void rootPathChanged(QString path);

private:
    DirectoryItem *m_root;
    QString m_rootPath;
    QString m_activePath;
    AppSettings* m_settings;
    DirectoryItem* itemFromIndex(const QModelIndex &index) const;

    DirectoryItem* findItemByPath(const QString &path) const;
    int findRow(DirectoryItem *item) const;
};
