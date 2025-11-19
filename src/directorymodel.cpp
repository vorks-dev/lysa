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

#include "directorymodel.h"
#include <QFileInfo>
#include <QDir>

DirectoryModel::DirectoryModel(AppSettings *settings, QObject *parent)
    : QAbstractItemModel(parent), m_root(new DirectoryItem{QString(), QString(), nullptr}), m_settings(settings)
{}

DirectoryModel::~DirectoryModel() {
    delete m_root;
}

void DirectoryModel::clear() {
    if (m_root->children.isEmpty()) return;

    beginResetModel();
    qDeleteAll(m_root->children);
    m_root->children.clear();
    endResetModel();
}

int DirectoryModel::columnCount(const QModelIndex &) const {
    return 1; // single column (name)
}

QModelIndex DirectoryModel::index(int row, int column, const QModelIndex &parent) const {
    if (column != 0 || row < 0)
        return QModelIndex();

    DirectoryItem *parentItem = itemFromIndex(parent);
    if (row >= parentItem->children.size())
        return QModelIndex();

    return createIndex(row, column, parentItem->children[row]);
}

QModelIndex DirectoryModel::parent(const QModelIndex &child) const {
    if (!child.isValid())
        return QModelIndex();

    DirectoryItem *childItem = static_cast<DirectoryItem*>(child.internalPointer());
    DirectoryItem *parentItem = childItem->parent;

    if (!parentItem || parentItem == m_root)
        return QModelIndex();

    DirectoryItem *grandParent = parentItem->parent;
    int row = grandParent ? grandParent->children.indexOf(parentItem) : 0;
    return createIndex(row, 0, parentItem);
}

int DirectoryModel::rowCount(const QModelIndex &parent) const {
    DirectoryItem *parentItem = itemFromIndex(parent);
    return parentItem ? parentItem->children.size() : 0;
}

QVariant DirectoryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};

    auto *item = static_cast<DirectoryItem*>(index.internalPointer());
    switch (role) {
        case PathRole: return item->path;
        case NameRole: return item->name;
        default: return {};
    }
}

QHash<int, QByteArray> DirectoryModel::roleNames() const {
    return {
        { PathRole, "path" },
        { NameRole, "name" }
    };
}

DirectoryItem* DirectoryModel::itemFromIndex(const QModelIndex &index) const {
    if (index.isValid()) return static_cast<DirectoryItem*>(index.internalPointer());
    return m_root;
}

void DirectoryModel::addDirectory(const QString &path, DirectoryItem *parentItem) {
    QFileInfo info(path);
    if (!info.exists() || !info.isDir())
        return;

    // Determine the parent from the given path if not provided
    if (!parentItem) {
        QString parentPath = info.absolutePath();
        parentItem = findItemByPath(parentPath);
        if (!parentItem)
            parentItem = m_root; // fallback to root
    }

    // Check if this directory already exists in the tree
    for (const auto *child : parentItem->children) {
        if (child->path == path)
            return; // already present
    }

    int row = parentItem->children.size();
    QModelIndex parentIndex = (parentItem == m_root)
        ? QModelIndex()
        : createIndex(findRow(parentItem), 0, parentItem);

    beginInsertRows(parentIndex, row, row);

    auto *child = new DirectoryItem{
        info.absoluteFilePath(),
        info.fileName(),
        parentItem
    };
    parentItem->children.append(child);

    endInsertRows();
}

DirectoryItem* DirectoryModel::findItemByPath(const QString &path) const {
    if (path.isEmpty() || path == m_root->path)
        return m_root;

    QList<DirectoryItem*> stack{ m_root };
    while (!stack.isEmpty()) {
        DirectoryItem *item = stack.takeLast();
        if (item->path == path)
            return item;
        for (auto *child : item->children)
            stack.append(child);
    }
    return nullptr;
}

int DirectoryModel::findRow(DirectoryItem *item) const {
    if (!item || !item->parent)
        return 0;
    return item->parent->children.indexOf(item);
}

void DirectoryModel::setActivePath(const QString &path) {
    if(m_activePath == path) return;
    m_activePath = path;
    m_settings->setValue("openedDirectory", path);
    emit activePathChanged(m_activePath);
}

QStringList DirectoryModel::parentPathChain(const QString &path) const {
    QStringList result;
    DirectoryItem *item = findItemByPath(path);
    while (item && item->parent) {
        result.prepend(item->parent->path);
        item = item->parent;
    }
    return result;
}

QString DirectoryModel::getLongName(const QString &path) const {
    DirectoryItem *item = findItemByPath(path);
    if (!item) return QString();

    QStringList names;
    while (item && item != m_root) {
        names.prepend(item->name);
        item = item->parent;
    }

    return names.join('/');
}
