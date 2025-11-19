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
#include <QSettings>
#include <QAbstractListModel>
#include <QVariantList>
#include <QSortFilterProxyModel>

struct SettingItem {
    QString id;
    QString label;
    QString section;
    QString type;
    QVariant value;
    QStringList options;
    bool visible = true;
};

class AppSettings : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        LabelRole,
        SectionRole,
        TypeRole,
        ValueRole,
        OptionsRole,
        VisibleRole
    };
    explicit AppSettings(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setVisible(const QString &id, bool visible);

    Q_INVOKABLE QVariantList getSectionSettings(const QString &sectionName) const;
    Q_INVOKABLE QVariantList getSections() const;

    Q_INVOKABLE QVariant getValue(const QString &id) const;
    Q_INVOKABLE void setValue(const QString &id, const QVariant &value);

signals:
    void settingChanged(const QString &id, const QVariant &value);

private:
    QList<SettingItem> m_items;
    QSettings m_settings;
    void loadFromSettings();
};
