#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    qDebug() << QDir::currentPath();
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/AsahikawaProcon-Server";
    qDebug() << path;
    QDir logDir(path);
    if(!logDir.exists()) {
        if(!logDir.mkpath(".")){
            qDebug() << "Could not makepath " << logDir.absolutePath();
        }
    }
    if (!QDir::setCurrent(path))
        qDebug() << "Could not change the current working directory";
    qDebug() << QDir::currentPath();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}
