#include "StableLog.h"
#include "QtCore/qdebug.h"



void StableLog::Write(const QString& str) const{
    //追加書き込みオープン
    QFile* file = new QFile(filename);
    file->open(QIODevice::Append);
    QTextStream log;
    log.setDevice(file);

    //書いて
    log << str;

    //閉じる
    delete file;
}

const StableLog& StableLog::operator<<(const QString& str)const{
    Write(str);
    return (*this);
}

StableLog::StableLog()
{

}

StableLog::StableLog(QString filename) : filename(filename){

    QFile* file = new QFile(filename);
    qDebug() << "StableLog";

    try {
        QTextStream log;
        QFileInfo fi(file->fileName());
        qDebug() << "Log Open " << fi.absoluteFilePath();
        file->open(QIODevice::WriteOnly);
        qDebug() << "Log setDevice";
        log.setDevice(file);
        //ログファイルオープン
        qDebug() << "Log setDevice2";
        log.setDevice(file);
        qDebug() << "Log output2";
        log << "--Stable Log--\r\n";
        qDebug() << "Log delete";
        delete file;
        qDebug() << "Log deleted";
    } catch(const std::exception& ex) {
        qDebug() << "std::exception" << ex.what();
    } catch (const std::runtime_error& e) {
        // この処理は実行されない
        qDebug() << "std::runtime_error: " << e.what();
    }
    qDebug() << "End StableLog";
}

StableLog::~StableLog()
{

}
