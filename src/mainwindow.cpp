#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QRandomGenerator>
#include "Animation.h"
#include "TeamAnimation.h"
#include "GameAnimation.h"
#include "BlindAnimation.h"

QString getTime(){
    return QString("[") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString("]");
}


void MainWindow::keyPressEvent(QKeyEvent * event){
    //縦に比率を合わせる
    if(event->key()==Qt::Key_F){
        int left_margin=0,right_margin=0;
        this->ui->centralWidget->layout()->getContentsMargins(&left_margin,nullptr,&right_margin,nullptr);
        this->ui->Field->resize((static_cast<float>(this->ui->Field->size().height())/ui->Field->field.size.y())*ui->Field->field.size.x(),this->ui->Field->size().height());
        this->resize(QSize(this->ui->Field->width() + left_margin + right_margin,this->size().height()));
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << Q_FUNC_INFO << "Startup";
    ui->setupUi(this);
    this->startup = new StartupDialog();

    qDebug() << Q_FUNC_INFO << "Connect destroyed signal to SaveFile";
    connect(this,SIGNAL(destroyed()),this,SLOT(SaveFile()));

    this->initStartupDialog();

    //スタートアップダイアログ開始
    connect(this->startup,SIGNAL(startupClosedSignal()), this, SLOT(initGame()));
    this->startup->exec();
}


void MainWindow::initStartupDialog() {
    //ServerSetting読み込み
    qDebug() << Q_FUNC_INFO << "ServerSetting";
    QString path;
    QSettings* mSettings;
    QVariant v;
    QFileInfo fi("setting.ini");
    qDebug() << Q_FUNC_INFO << "SettingIni Open " << fi.absoluteFilePath();
    mSettings = new QSettings( "setting.ini", QSettings::IniFormat ); // iniファイルで設定を保存
    mSettings->setIniCodec( "UTF-8" ); // iniファイルの文字コード
    v = mSettings->value( "LogFilepath" );
    if (v.type() != QVariant::Invalid)path = v.toString();
    v = mSettings->value( "Gamespeed" );
    if (v.type() != QVariant::Invalid)FRAME_RATE = v.toInt();
    v = mSettings->value( "Silent" );
    if (v.type() != QVariant::Invalid)silent = v.toBool();
    else silent = false;

    v = mSettings->value( "Team" );
    if (v.type() != QVariant::Invalid)anime_team_time = v.toInt();

    //デザイン設定を書き換え
    QSettings* dSettings;
    QVariant v2;
    dSettings = new QSettings( "design.ini", QSettings::IniFormat ); // iniファイルで設定を保存
    dSettings->setIniCodec( "UTF-8" ); // iniファイルの文字コード
    v2 = dSettings->value( "Dark" );
    if (v2.type() != QVariant::Invalid)dark = v2.toBool();
    else dark = false;
    v2 = dSettings->value( "Bot" );
    if (v2.type() != QVariant::Invalid)isbotbattle = v2.toBool();
    else isbotbattle = false;
    if(dark == true)this->anime_map_time -= this->anime_blind_time;

    //ログファイルオープン
    log = StableLog("log" + getTime() + ".txt");
}

void MainWindow::initGame() {
    QSettings* mSettings;

    //マップ初期化
    for(int i=0;i<TEAM_COUNT;i++){
        this->ui->Field->team_pos[i] = this->startup->map.team_first_point[i];
        qDebug() << Q_FUNC_INFO << this->ui->Field->team_pos[i];
    }
    //ui初期化
    this->ui->Field  ->setMap(this->startup->map);
    this->ui->TimeBar->setMaximum(this->startup->map.turn);
    this->ui->TimeBar->setValue  (this->startup->map.turn);
    this->ui->TurnLabel     ->setText("残りターン : " + QString::number(this->ui->TimeBar->value()));
    this->ui->CoolNameLabel ->setText(this->startup->team_client[static_cast<int>(GameSystem::TEAM::COOL)]->client->Name == "" ? "Cool" : this->startup->team_client[static_cast<int>(GameSystem::TEAM::COOL)]->client->Name);
    this->ui->HotNameLabel  ->setText(this->startup->team_client[static_cast<int>(GameSystem::TEAM::HOT )]->client->Name == "" ? "Hot"  : this->startup->team_client[static_cast<int>(GameSystem::TEAM::HOT )]->client->Name);

    //ボット戦モードならば表記の変更
    if(this->isbotbattle){
        this->ui->HotScoreLabel ->setText(QString::number(this->startup->map.turn) + "(ITEM:0)");
        this->ui->CoolScoreLabel->setText(QString::number(this->startup->map.turn) + "(ITEM:0)");
    }else{
        this->ui->HotScoreLabel ->setText("0");
        this->ui->CoolScoreLabel->setText("0");
    }

    /*
    music = new QSound(MUSIC_DIRECTORY + "/Music/" + this->startup->music_text + ".wav");

    if(!silent)music->play();
    */

    //消音モードじゃない かつ Musicフォルダに音楽が存在する ならBGMセット
    if(!silent && this->startup->music_text != "None"){
        bgm = new QMediaPlayer;
        connect(bgm, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
        bgm->setMedia(QUrl::fromLocalFile("./Music/" + this->startup->music_text));
        bgm->setVolume(50);
        bgm->play();

        //log << "[ Music : " + MUSIC_DIRECTORY + "/Music/" + this->startup->music_text + ".wav ]" + "\r\n";
        qDebug() << Q_FUNC_INFO << " " << QUrl::fromLocalFile("./Music/" + this->startup->music_text).toLocalFile();
        log << "[ Music : " + QUrl::fromLocalFile("./Music/" + this->startup->music_text).toLocalFile() + " ]" + "\r\n";
        //log << MUSIC_DIRECTORY + "/Music/" + this->startup->music_text + ".wav";
    }


    for(int i=0;i<TEAM_COUNT;i++){
        ui->Field->team_pos[i].setX(-1);
        ui->Field->team_pos[i].setY(-1);
    }

    //アイテム数ラベルセット
    for(int i=0;i<startup->map.size.y();i++){
       for(int j=0;j<startup->map.size.x();j++){
            if(startup->map.field[i][j] == GameSystem::MAP_OBJECT::ITEM)this->ui->Field->leave_items++;
       }
    }
    ui->ItemLeaveLabel->setText(QString::number(this->ui->Field->leave_items));

    mSettings = new QSettings( "setting.ini", QSettings::IniFormat ); // iniファイルで設定を保存
    mSettings->setIniCodec( "UTF-8" ); // iniファイルの文字コード
    QVariant v;
    v = mSettings->value( "Maximum" );
    if (v.type() != QVariant::Invalid && v.toBool()){
        setWindowState(Qt::WindowMaximized);
    }


    //AnimationTime読み込み
    mSettings = new QSettings( "AnimationTime.ini", QSettings::IniFormat ); // iniファイルで設定を保存
    v = mSettings->value( "Map" );
    if (v.type() != QVariant::Invalid)anime_map_time = v.toInt();
    else{

        QSettings* mSettings;
        mSettings = new QSettings( "AnimationTime.ini", QSettings::IniFormat ); // iniファイルで設定を保存
        mSettings->setIniCodec( "UTF-8" ); // iniファイルの文字コード

        mSettings->setValue( "Map" , anime_map_time );
        mSettings->setValue( "Team", anime_team_time );

    }
    qDebug() << "セットアップ完了　ゲームを開始します。";
    log << getTime() + "セットアップ完了　ゲームを開始します。\r\n";
    this->StartAnimation();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SaveFile(){
    file->close();
}


//終了処理
void MainWindow::GameFinished(){
    if(!silent)bgm->stop();

    if(!silent){
        bgm = new QMediaPlayer;
        connect(bgm, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
        bgm->setMedia(QUrl("qrc:/Sound/ji_023.wav"));
        bgm->setVolume(50);
        bgm->play();
    }
    this->ui->pushButtonClose->setEnabled(true);
}

/**
 * ゲーム開始後に地図を徐々に描画するアニメーション
 *
 * @brief MainWindow::StartAnimation
 */
void MainWindow::StartAnimation(){
    Animation *animation = new Animation(this->ui, &this->startup->map);
    connect(animation, SIGNAL(AnimationRepaintSignal()), this, SLOT(repaint()));
    connect(animation, SIGNAL(AnimationFinishSignal()), this, SLOT(ShowTeamAnimation()));
    animation->Run(this->anime_map_time / (startup->map.size.x()*startup->map.size.y()));
}


/**
 * Botを地図上の初期位置に配置するアニメーション
 * @brief MainWindow::ShowTeamAnimation
 */
void MainWindow::ShowTeamAnimation(){
    TeamAnimation *animation = new TeamAnimation(this->ui, &this->startup->map);
    connect(animation, SIGNAL(AnimationRepaintSignal()), this, SLOT(repaint()));
    connect(animation, SIGNAL(AnimationFinishSignal()), this, SLOT(StartGameAnimation()));
    animation->Run(this->anime_team_time / TEAM_COUNT);
}

/**
 * ステージ暗転モードのときのアニメーション
 * @brief MainWindow::StartBlindAnimation
 */
void MainWindow::StartBlindAnimation(){
    if(dark == true){
        BlindAnimation *animation = new BlindAnimation(this->ui, &this->startup->map);
        connect(animation, SIGNAL(AnimationRepaintSignal()), this, SLOT(repaint()));
        connect(animation, SIGNAL(AnimationFinishSignal()), this, SLOT(StartGameAnimation()));
        animation->Run(anime_blind_time / (startup->map.size.x()*startup->map.size.y()));
    } else {
        this->StartGameAnimation();
    }
}

/**
 * 実際のゲーム進行アニメーション
 * @brief MainWindow::StartGameAnimation
 */
void MainWindow::StartGameAnimation(){
    GameAnimation *animation = new GameAnimation(this->ui, this->startup->team_client);
    connect(animation, SIGNAL(AnimationRepaintSignal()), this, SLOT(repaint()));
    connect(animation, SIGNAL(AnimationFinishSignal()), this, SLOT(GameFinished()));
    animation->Run(FRAME_RATE);
}



void MainWindow::on_pushButtonClose_released()
{
    this->hide();
    GameSystem::Map old_map = this->startup->map;
    this->startup = new StartupDialog();
    this->startup->map = old_map;
    connect(this->startup,SIGNAL(startupClosedSignal()), this, SLOT(initGame()));
    this->startup->exec();
    this->show();
}

