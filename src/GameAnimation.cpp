#include "GameAnimation.h"
#include "ui_mainwindow.h"
#include "GameSystem.h"
#include "QRandomGenerator"
#include "QTimer"
#include "ClientSettingForm.h"
#include "StableLog.h"
#include "mainwindow.h"

QString convertString(GameSystem::Method method){
    QString str;
    if(method.action == GameSystem::Method::ACTION::GETREADY)str += "GetReady";
    if(method.action == GameSystem::Method::ACTION::LOOK)    str += "Look";
    if(method.action == GameSystem::Method::ACTION::PUT)     str += "Put";
    if(method.action == GameSystem::Method::ACTION::SEARCH)  str += "Search";
    if(method.action == GameSystem::Method::ACTION::WALK)    str += "Walk";

    if(method.rote == GameSystem::Method::ROTE::UP)   str += "Up";
    if(method.rote == GameSystem::Method::ROTE::RIGHT)str += "Right";
    if(method.rote == GameSystem::Method::ROTE::LEFT) str += "Left";
    if(method.rote == GameSystem::Method::ROTE::DOWN) str += "Down";

    return str;
}

GameAnimation::GameAnimation(Ui_MainWindow *ui, ClientSettingForm **players, QObject *parent)
    : QObject(parent)
{
    this->ui = ui;
    this->players = players;

    turn_count = 0;
    getready_flag = true;
    player = 0;
    log = StableLog("log" + getTime() + ".txt");
    this->win = GameSystem::WINNER::CONTINUE;
}

void GameAnimation::Run(int interval) {
    qDebug() << Q_FUNC_INFO << "Run";
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
    timer->start(interval);
}

void GameAnimation::startAnimation() {
    //ゲーム進行
    static GameSystem::Method team_mehod[TEAM_COUNT];
    this->ui->Field->RefreshOverlay();

    //ターンログ出力
    if(ui->TimeBar->value() != turn_count){
       turn_count = ui->TimeBar->value();
       log << QString("-----残") + QString::number(turn_count) + "ターン-----" + "\r\n";
       qDebug() << QString("-----残") + QString::number(turn_count) + "ターン-----";
    }

    //GetReadyの取得
    if(getready_flag){
        // GetReady
        if(!players[player]->client->WaitGetReady()){
            log << getTime() + "[停止]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + "が正常にGetReadyを返しませんでした!" << "\r\n";
            players[player]->client->is_disconnected = true;
        }else{
            //log << getTime() + "GetReady" + "\r\n";
            GameSystem::AroundData buffer = ui->Field->FieldAccessAround(GameSystem::Method{static_cast<GameSystem::TEAM>(player),
                                                                         GameSystem::Method::ACTION::GETREADY,
                                                                         GameSystem::Method::ROTE::UNKNOWN},
                                                                         ui->Field->team_pos[player]);
            team_mehod[player] = players[player]->client->WaitReturnMethod(buffer);



            if(team_mehod[player].action == GameSystem::Method::ACTION::GETREADY){
                log << getTime() + "[停止]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + "が二度GetReadyを行いました!" << "\r\n";
                players[player]->client->is_disconnected = true;
            }
            team_mehod[player].team = static_cast<GameSystem::TEAM>(player);
        }
        //End
        this->win = Judge();

        if(win != GameSystem::WINNER::CONTINUE){

            player++;
            player %= TEAM_COUNT;

            players[player]->client->WaitGetReady();
            GameSystem::AroundData buffer = ui->Field->FieldAccessAround(GameSystem::Method{static_cast<GameSystem::TEAM>(player),
                                                                         GameSystem::Method::ACTION::GETREADY,
                                                                         GameSystem::Method::ROTE::UNKNOWN},
                                                                         ui->Field->team_pos[player]);
            buffer.connect = GameSystem::CONNECTING_STATUS::FINISHED;
            team_mehod[player] = players[player]->client->WaitReturnMethod(buffer);
            Finish(win);
        }

    }else{

        // Method
        GameSystem::AroundData around = ui->Field->FieldAccessMethod(team_mehod[player]);
        //アイテムの回収
        RefreshItem(team_mehod[player]);
        this->win = Judge();
        if(this->win != GameSystem::WINNER::CONTINUE){
            around.connect = GameSystem::CONNECTING_STATUS::FINISHED;
        }

        if(players[player]->client->WaitEndSharp(around)){
            //不正行動をはじく
            if(team_mehod[player].action == GameSystem::Method::ACTION::UNKNOWN){
                log << getTime() + "[停止]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + "が不正なメソッドを呼んでいます！" << "\r\n";
                players[player]->client->is_disconnected = true;
            }
            if(team_mehod[player].rote   == GameSystem::Method::ROTE::UNKNOWN){
                log << getTime() + "[停止]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + "の行動メソッドが不正な方向を示しています！" << "\r\n";
                players[player]->client->is_disconnected = true;
            }

            //行動ログの出力
            log << getTime() + "[行動]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + "が" + convertString(team_mehod[player]) + "を行いました。" << "\r\n";

            GameBoard*& board = this->ui->Field;
            GameSystem::AroundData team_around = board->FieldAccessAround(static_cast<GameSystem::TEAM>(player));
            log << getTime() + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + ":" + team_around.toString() << "\r\n";

            //refresh
            if(player ==  TEAM_COUNT-1){
                ui->TimeBar->setValue(this->ui->TimeBar->value() - 1);
                ui->TimeBar->repaint();
                this->ui->TurnLabel->setText("残りターン : " + QString::number(ui->TimeBar->value()));

                //ボット戦モードならば表記のリアルタイム更新
                if(this->isbotbattle){
                    int ScoreBuf = this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::COOL)];
                    ui->CoolScoreLabel->setText(QString::number(ui->TimeBar->value() + ScoreBuf*3) + "(ITEM:" + QString::number(ScoreBuf) + ")");
                    ScoreBuf = this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::HOT)];
                    ui->HotScoreLabel ->setText(QString::number(ui->TimeBar->value() + ScoreBuf*3) + "(ITEM:" + QString::number(ScoreBuf) + ")");

                }
            }
        }else{
            log << getTime() + "[停止]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(player)) + "が正常にGetReadyを返しませんでした!" << "\r\n";
            players[player]->client->is_disconnected = true;
        }
        //End

        player++;
        player %= TEAM_COUNT;

        if(win != GameSystem::WINNER::CONTINUE){
            players[player]->client->WaitGetReady();
            GameSystem::AroundData buffer = ui->Field->FieldAccessAround(GameSystem::Method{static_cast<GameSystem::TEAM>(player),
                                                                         GameSystem::Method::ACTION::GETREADY,
                                                                         GameSystem::Method::ROTE::UNKNOWN},
                                                                         ui->Field->team_pos[player]);
            buffer.connect = GameSystem::CONNECTING_STATUS::FINISHED;
            team_mehod[player] = players[player]->client->WaitReturnMethod(buffer);
            Finish(win);
        }

    }

    getready_flag = !getready_flag;

    qDebug() << Q_FUNC_INFO << " Repaint " << QString::number(turn_count);
    Q_EMIT(AnimationRepaintSignal());

}

void GameAnimation::RefreshItem(GameSystem::Method method){

    static int leave_item = 0;
    if(leave_item == 0)leave_item = this->ui->Field->leave_items;
    if(this->ui->Field->leave_items != leave_item){
        ui->ItemLeaveLabel->setText(QString::number(this->ui->Field->leave_items));
        log << getTime() + "[取得]" + GameSystem::TEAM_PROPERTY::getTeamName(method.team) + "がアイテムを取得しました。" << "\r\n";
        if(this->isbotbattle){
            int ScoreBuf = this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::COOL)];
            ui->CoolScoreLabel->setText(QString::number(ui->TimeBar->value() + ScoreBuf*3) + "(ITEM:" + QString::number(ScoreBuf) + ")");
            ScoreBuf = this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::HOT)];
            ui->HotScoreLabel ->setText(QString::number(ui->TimeBar->value() + ScoreBuf*3) + "(ITEM:" + QString::number(ScoreBuf) + ")");
        }else{
            ui->CoolScoreLabel->setText(QString::number(this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::COOL)]));
            ui->HotScoreLabel ->setText(QString::number(this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::HOT)]));
        }
        leave_item = this->ui->Field->leave_items;
    }


}

void GameAnimation::Finish(GameSystem::WINNER winner){
    this->timer->stop();
    QString append_str = "";
    //disconnect
    for(int i=0;i<TEAM_COUNT;i++){
        if(players[i]->client->is_disconnected){
            append_str.append("\r\n[" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + " 切断により]");
            log << getTime() + "[終了]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + "との通信が切断されています。" << "\r\n";
        }
    }
    log << this->ui->WinnerLabel->text() << "\r\n";

    if(winner == GameSystem::WINNER::COOL){
        this->ui->WinnerLabel->setText("COOL WIN!" + append_str);
        log << getTime() + "[決着]COOLが勝利しました。" << "\r\n";
        //負けチームのスコア更新（ターン数分減らす）
        if(this->isbotbattle){
            int ScoreBuf = this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::HOT)];
            ui->HotScoreLabel ->setText(QString::number(ScoreBuf*3) + "(ITEM:" + QString::number(ScoreBuf) + ")");
        }
    }
    if(winner == GameSystem::WINNER::HOT){
        this->ui->WinnerLabel->setText("HOT WIN!"  + append_str);
        log << getTime() + "[決着]HOTが勝利しました。" << "\r\n";
        //負けチームのスコア更新（ターン数分減らす）
        if(this->isbotbattle){
            int ScoreBuf = this->ui->Field->team_score[static_cast<int>(GameSystem::TEAM::COOL)];
            ui->CoolScoreLabel->setText(QString::number(ScoreBuf*3) + "(ITEM:" + QString::number(ScoreBuf) + ")");
        }
    }
    if(winner == GameSystem::WINNER::DRAW){
        this->ui->WinnerLabel->setText("DRAW");
        log << getTime() + "[決着]引き分けです。" << "\r\n";
    }
    Q_EMIT(AnimationFinishSignal());
}

GameSystem::WINNER GameAnimation::Judge(){
    bool team_lose[TEAM_COUNT];

    for(int i=0;i<TEAM_COUNT;i++)team_lose[i] = false;
    GameBoard*& board = this->ui->Field;

    for(int i=0;i<TEAM_COUNT;i++){

        GameSystem::AroundData team_around = board->FieldAccessAround(static_cast<GameSystem::TEAM>(i));
        //log << getTime() + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + ":" + team_around.toString() << "\r\n";

        //ブロック置かれ死
        if(team_around.data[4] == GameSystem::MAP_OBJECT::BLOCK){
            log << getTime() + "[死因]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + "ブロック下敷き" << "\r\n";
            team_around.finish();
            team_lose[i]=true;
        }

        //ブロック囲まれ死
        if(team_around.data[1] == GameSystem::MAP_OBJECT::BLOCK &&
           team_around.data[3] == GameSystem::MAP_OBJECT::BLOCK &&
           team_around.data[5] == GameSystem::MAP_OBJECT::BLOCK &&
           team_around.data[7] == GameSystem::MAP_OBJECT::BLOCK){
            log << getTime() + "[死因]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + "ブロック囲まれ" << "\r\n";
            team_around.finish();
            team_lose[i]=true;
        }

        //切断死
        if(players[i]->client->is_disconnected){
            log << getTime() + "[死因]" + GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + "通信切断" << "\r\n";
            team_around.finish();
            team_lose[i]=true;
        }
    }

    //相打ち、または時間切れ時はアイテム判定とする
    if(!std::find(team_lose,team_lose+TEAM_COUNT,false) || ui->TimeBar->value()==0){
        log << getTime() + "[情報]相打ちまたは、タイムアップのためアイテム判定を行います" + "\r\n";
        log << getTime() + "[得点]";
        for(int i=0;i<TEAM_COUNT;i++){
            log << GameSystem::TEAM_PROPERTY::getTeamName(static_cast<GameSystem::TEAM>(i)) + ":" + QString::number(this->ui->Field->team_score[i]) + "  ";
            team_lose[i] = false;
        }
        log << "\r\n";

        //引き分けかな？
        QSet<int> team_score_set; //スコア
        for(int i=0;i<TEAM_COUNT;i++){
            team_score_set.insert(this->ui->Field->team_score[i]);
        }
        if(team_score_set.size()==1)return GameSystem::WINNER::DRAW;

        int index=0;
        for(int i=0;i<TEAM_COUNT;i++){
            if(this->ui->Field->team_score[index] < this->ui->Field->team_score[i])index = i;
        }
        //勝者判定
        return static_cast<GameSystem::WINNER>(index);
    }

    if(team_lose[0])return GameSystem::WINNER::HOT;
    else if(team_lose[1])return GameSystem::WINNER::COOL;
    else return GameSystem::WINNER::CONTINUE;
}

QString GameAnimation::getTime(){
    return QString("[") + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QString("]");
}


