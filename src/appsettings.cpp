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

#include "appsettings.h"

AppSettings::AppSettings(QObject *parent)
    : QAbstractListModel(parent), m_settings("vorks", "Lysa") {
        loadFromSettings();
    }


//----- QAbstractListModel override -----//
int AppSettings::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_items.size();
}

QVariant AppSettings::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || index.row() >= m_items.size()) return {};

    const auto &item = m_items[index.row()];

    switch(role) {
        case IdRole: return item.id;
        case LabelRole: return item.label;
        case SectionRole: return item.section;
        case TypeRole: return item.type;
        case ValueRole: return item.value;
        case OptionsRole: return item.options;
        case VisibleRole: return item.visible;
        default: return {};
    }
}

bool AppSettings::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(!index.isValid() || role != ValueRole) return false;

    m_items[index.row()].value = value;
    emit dataChanged(index, index, {ValueRole});
    return true;
}

QHash<int, QByteArray> AppSettings::roleNames() const {
    return {
        {IdRole, "id"},
        {LabelRole, "label"},
        {SectionRole, "section"},
        {TypeRole, "type"},
        {ValueRole, "value"},
        {OptionsRole, "options"},
        {VisibleRole, "visible"}
    };
}


//----- Settings logic -----//
void AppSettings::loadFromSettings() {
    beginResetModel();
    m_items.clear();

    // Define all settings automatically
    m_items = {
        //{"language", "Language", "General", "ComboBox", m_settings.value("language", "English").toString(), {"English", "German", "French"}, true}, //No language System yet
        {"theme", "Theme", "General", "ComboBox", m_settings.value("theme", "dark").toString(), {"dark", "light"}, true},
        {"autoHideToolbar", "Automatically hide Toolbar", "General", "Switch", m_settings.value("autoHideToolbar", false).toBool(), {}, true},
        {"rootFolder", "Root Folder", "Gallery", "FolderDialog", m_settings.value("rootFolder", "").toString(), {}, true},
        {"gallerySortMode", "Sort by", "Gallery", "ComboBox", m_settings.value("gallerySortMode", "date").toString(), {"date", "size", "exposure", "camera", "iso", "focalLength"}, true},
        {"gallerySortAscending", "Sort in ascending order", "Gallery", "Switch", m_settings.value("gallerySortAscending", true).toBool(), {}, true},
        {"photoBackground", "Fullscreen Background", "Photo View", "ComboBox", m_settings.value("photoBackground", "black").toString(), {"black", "standard"}},

        // About-Section
        {"impressum", "", "About", "Link", "vorks-dev.github.io/impressum/", {"https://", "Impressum"}, true},
        {"sourceCode", "", "About", "Link", "github.com/vorks-dev/lysa/", {"https://", "Source Code"}, true},
        {"eula", "", "About", "TxtArea", "EULA.txt", {}, true},
        {"privacy", "", "About", "TxtArea", "privacy.txt", {}, true},
        {"license", "", "About", "TxtArea", "LICENSE.txt", {}, true},
        {"license-qt", "", "About", "TxtArea", "licenses/qt_lgpl.txt", {}, true},
        {"license-exiv2", "", "About", "TxtArea", "licenses/exiv2_gpl2.txt", {}, true},
        {"license-expat", "", "About", "TxtArea", "licenses/expat_mit.txt", {}, true},
        {"license-inih", "", "About", "TxtArea", "licenses/inih_bsd.txt", {}, true},
        {"license-brotli", "", "About", "TxtArea", "licenses/brotli_mit.txt", {}, true},
        {"license-zlib", "", "About", "TxtArea", "licenses/zlib_license.txt", {}, true},

        // Internal settings
        {"showSidebar", "Sidebar visible", "Gallery", "Switch", m_settings.value("showSidebar", true).toBool(), {}, false},
        {"openedDirectory", "Opened Album", "Gallery", "FolderDialog", m_settings.value("openedDirectory", "").toString(), {}, false},
        {"galleryTargetWidth", "Targetted thumbnail width", "Gallery", "TextField", m_settings.value("galleryTargetWidth", 200).toInt(), {}, false},
        {"photoWindowMaximized", "Photo-Window Maximized", "Photo View", "Switch", m_settings.value("photoWindowMaximized", false).toBool(), {}, false},
        {"showPhotoMetadata", "Metadata visible", "Photo View", "Switch", m_settings.value("showPhotoMetadata", false).toBool(), {}, false}
    };
    endResetModel();
}

void AppSettings::setVisible(const QString &id, bool visible) {
    for(int i = 0; i < m_items.size(); ++i) {
        if(m_items[i].id != id) continue;
        m_items[i].visible = visible;
        emit dataChanged(index(i), index(i), {VisibleRole});
        break;
    }
}

void AppSettings::reload() {
    loadFromSettings();
}

QVariantList AppSettings::getSectionSettings(const QString &sectionName) const {
    QVariantList list;
    for(const auto &item : m_items)
        if(item.section == sectionName) {
            QVariantMap map;
            map["id"] = item.id;
            map["label"] = item.label;
            map["type"] = item.type;
            map["value"] = item.value.isValid() ? item.value : QVariant();
            map["options"] = item.options;
            map["visible"] = item.visible;
            list.append(map);
        }
    return list;
}

QVariantList AppSettings::getSections() const {
    QStringList uniqueSections;
    for(const auto &item : m_items) {
        if(!uniqueSections.contains(item.section))
            uniqueSections.append(item.section);
    }
    QVariantList list;
    for(const auto &sec : uniqueSections)
        list.append(QVariant(sec));
    return list;
}


//----- Runtime helpers -----//
QVariant AppSettings::getValue(const QString &id) const {
    for(const auto &item : m_items) {
        if(item.id == id)
            return item.value;
    }

    // Check QSettings if no active setting
    if(m_settings.contains(id)) {
        QVariant v = m_settings.value(id);
        return v;
    }

    return {};
}

void AppSettings::setValue(const QString &id, const QVariant &value) {
    qDebug() << "Setting" << id << "to" << value;
    for(int i = 0; i < m_items.size(); ++i) {
        if(m_items[i].id != id) continue;
        if(m_items[i].value != value) {
            m_items[i].value = value;
            emit dataChanged(index(i), index(i), {ValueRole});
            emit settingChanged(id, value);
        }
        break;
    }

    // Save to QSettings
    m_settings.setValue(id, value);
    m_settings.sync();
}