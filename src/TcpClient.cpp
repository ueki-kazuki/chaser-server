#include "TcpClient.h"
#include <QSettings>
#include <QSharedPointer>

QString TCPClient::VisibilityString(QString str){
    QString answer;
    for(int i = 0;i < str.size();i++){
        if     (str[i] == '\n')answer.push_back("\\n");
        else if(str[i] == '\r')answer.push_back("\\r");
        else   answer.push_back(str[i]);
    }
    return answer;
}

QString TCPClient::WaitResponce(){
    int ignore = 0;
    while(ignore != this->IGNORE_INVALD){
        //レスポンス待ち
        qDebug() << "WaitStart:";
        if(this->client->waitForReadyRead(this->TIMEOUT)){
            //レスポンスあり

            QString response = "";

            //自動結合
            //bool f = false;
            do{
                response += client->readLine();
            }while(*(response.end()-1) != '\n' && this->client->waitForReadyRead(this->TIMEOUT));

            qDebug() << "" + VisibilityString(response);

            //不正文字列：空
            if(response == "" || response == "\n" || response == "\r" || response == "\r\n"){
                ignore++;

                continue;
            }
            //不正文字列：改行なし
            if(response.size() > 0 && *(response.end()-1) != '\n'){
                is_disconnected = true;
                qDebug() << QString("[Port") + QString::number(this->client->localPort()) +"]:Noting \\n";

                return QString();
            }

            qDebug()<<"test:"+response;
            return response;
        }else{
            //レスポンスなし
            is_disconnected = true;
            //qDebug() << QString("[Port") + QString::number(this->client->localPort()) +"]:Noting responce";
            return QString();
        }
    }
    is_disconnected=true;
    //qDebug() << QString("[Port") + QString::number(this->client->localPort()) +"]:Too many invald responce";
    return QString();
}

bool TCPClient::WaitGetReady(){
    if(is_disconnected)return false;
    //ターン開始文字列
    if(client == nullptr)return false;
    client->write(QString("@\r\n").toUtf8());

    //レスポンス待ち
    QString response = WaitResponce();
    qDebug() << "res:"+response;
    qDebug() << (response == "gr\r\n");
    return (response == "gr\r\n");
}
GameSystem::Method TCPClient::WaitReturnMethod(GameSystem::AroundData data){
    if(is_disconnected)return GameSystem::Method();
    //周辺情報文字列
    if(client == nullptr)return GameSystem::Method{GameSystem::TEAM::UNKNOWN,
                GameSystem::Method::ACTION::UNKNOWN,
                GameSystem::Method::ROTE::UNKNOWN};
    client->write(QString(data.toString() + "\r\n").toUtf8());
    qDebug() << "WRM";
    qDebug() << data.toString();
    //レスポンス待ち
    QString response = WaitResponce();
    if(response != QString())return GameSystem::Method::fromString(response);
    else return GameSystem::Method{GameSystem::TEAM::UNKNOWN,
                                   GameSystem::Method::ACTION::UNKNOWN,
                                   GameSystem::Method::ROTE::UNKNOWN};
}
bool TCPClient::WaitEndSharp(GameSystem::AroundData data){
    if(is_disconnected)return false;

    if(client == nullptr)return false;
    //周辺情報文字列
    client->write(QString(data.toString() + "\r\n").toUtf8());
    qDebug() << "WES";
    qDebug() << data.toString();
    //レスポンス待ち
    return(WaitResponce() == "#\r\n");
}


bool TCPClient::OpenSocket(int Port){
    this->server->listen(QHostAddress::Any,Port);
    return true;
}
bool TCPClient::CloseSocket(){
    if(this->client->isOpen()){
        this->client->disconnectFromHost();
    }
    if(this->server->isListening()){
        this->server->close();
    }
    //this->server = QSharedPointer<QTcpServer>::create(this);
    //emit DisConnected();
    return true;
}
bool TCPClient::isConnecting(){
    return this->server->isListening();
}
void TCPClient::NewConnection(){
    this->client.reset(this->server->nextPendingConnection());
    this->IP = this->client->peerAddress().toString();
    connect(this->client.data(), SIGNAL(readyRead()), this, SLOT(GetTeamName()));
    connect(this->client.data(), SIGNAL(disconnected()), this, SLOT(DisConnected()));
    emit Connected();
}

void TCPClient::DisConnected(){
    if(this->client->isOpen()){
        this->client->close();
    }
    this->IP   = "";
    this->Name = "";
    is_disconnected=true;
    emit Disconnected();
}

QString TCPClient::GetTeamName(){
    if(this->Name == ""){
        //Qstring::fromLocal8bitで文字化け対策しています
        //this->Name = QString::fromLocal8Bit(client->readAll());
        QByteArray bytebuf = client->readAll();
        QString namebuf = bytebuf;

        //ここで0xFFFD(utfへの自動変換の失敗)が観測された場合、変換を行う
        for(int i = 0; i < namebuf.size(); i++){
            QChar buffer = namebuf.at(i);
            //不明な文字が発見された場合、文字コードをUTFに変換
            if(buffer == 0xFFFD){
                namebuf = QString::fromLocal8Bit(bytebuf);
            }
        }

        this->Name = namebuf;

        disconnect(this->client.data(), SIGNAL(readyRead()), this, SLOT(GetTeamName()));
        emit WriteTeamName();
        emit Ready();
        return this->Name;

    }
    return this->Name;
}

TCPClient::TCPClient(QObject *parent) :
    BaseClient(parent)
{
    QSharedPointer<QSettings> mSettings;
    mSettings = QSharedPointer<QSettings>::create( "setting.ini", QSettings::IniFormat ); // iniファイルで設定を保存
    mSettings->setIniCodec( "UTF-8" ); // iniファイルの文字コード
    QVariant v = mSettings->value( "Timeout" );
    if (v.type() != QVariant::Invalid){
        TIMEOUT = v.toInt();
    }

    this->server = QSharedPointer<QTcpServer>::create(this);
    this->client.reset();
    //接続最大数を1に固定
    this->server->setMaxPendingConnections(1);
    //シグナルとスロットを接続
    connect(this->server.data(), SIGNAL(newConnection()), this, SLOT(NewConnection()));
}

TCPClient::~TCPClient()
{
    if(isConnecting()){
        CloseSocket();
    }
}

