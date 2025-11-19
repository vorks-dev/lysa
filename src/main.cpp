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

#include <QApplication>
#include <QQuickStyle>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QIcon>
#include "photocontroller.h"
#include "appsettings.h"
#include "fileservice.h"

//----- LOGGING -----//

static QFile logFile;
void messageLogger(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    if(!logFile.isOpen()) return;
    QTextStream out(&logFile);
    QString prefix;

    switch(type) {
        case QtDebugMsg: prefix = "DEBUG"; break;
        case QtWarningMsg: prefix = "WARN"; break;
        case QtCriticalMsg: prefix = "CRIT"; break;
        case QtFatalMsg: prefix = "FATAL"; break;
    }

    out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
        << " [" << prefix << "] "
        << msg << "\n";
    out.flush();

    if(type == QtFatalMsg) abort();
}

static QString createSessionLogFile() {
    QDir logDir(QDir::currentPath() + "/logs");
    if(!logDir.exists()) logDir.mkpath(".");

    QString fileName = QString("lysa_%1.log").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    return logDir.filePath(fileName);
}

static void rotateLogs() {
    QDir logDir(QDir::currentPath() + "/logs");
    if(!logDir.exists()) return;

    QStringList files = logDir.entryList(QStringList() << "lysa_*.log", QDir::Files, QDir::Time);

    // Keep only 10 newest logs
    for(int i = 10; i < files.size(); ++i) {
        logDir.remove(files.at(i));
    }
}


//----- MAIN -----//

int main(int argc, char *argv[]) {
    rotateLogs();
    logFile.setFileName(createSessionLogFile());
    logFile.open(QIODevice::WriteOnly |QIODevice::Text);
    qInstallMessageHandler(messageLogger);

    QApplication app(argc, argv);
    QApplication::setOrganizationName("vorks");
    QApplication::setOrganizationDomain("vorks.dev");
    QApplication::setApplicationName("Lysa");
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
    QApplication::setWindowIcon(QIcon(":/icons/lysa.svg"));

    // Create settings instance
    AppSettings settings;

    // Create photo controller
    PhotoController controller(&settings);

    // Create TxtReader
    FileService fileService;

    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("galleryModel", controller.galleryModel());
    engine.rootContext()->setContextProperty("directoryModel", controller.dirs());
    engine.rootContext()->setContextProperty("photoController", &controller);
    engine.rootContext()->setContextProperty("settingsModel", &settings);
    engine.rootContext()->setContextProperty("fileService", &fileService);

    engine.load(QUrl("qrc:/main.qml"));

    return app.exec();
}
