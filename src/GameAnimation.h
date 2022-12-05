#ifndef GAMEANIMATION_H
#define GAMEANIMATION_H

#include "ui_mainwindow.h"
#include "GameSystem.h"
#include <QTimer>
#include <QObject>
#include "ClientSettingForm.h"
#include "StableLog.h"

class GameAnimation : public QObject
{
    Q_OBJECT

public:
    explicit GameAnimation(Ui_MainWindow *ui, ClientSettingForm **players, QObject *parent = nullptr);
    void Run(int interval);
private slots:
    void startAnimation();
private:
    //アイテム取得の判定
    void RefreshItem(GameSystem::Method method);
    //決着判定
    GameSystem::WINNER Judge();
    //決着
    void Finish(GameSystem::WINNER winner);
    //
    QString getTime();

    // インスタンス変数
    QTimer *timer;
    Ui_MainWindow *ui;
    ClientSettingForm **players;

    //Field<GameSystem::MAP_OVERLAY> f;
    int player;      //次ターン行動プレイヤー
    int turn_count;
    bool getready_flag;
    GameSystem::WINNER win;
    StableLog log;
    bool isbotbattle;
signals:
    void AnimationFinishSignal();
    void AnimationRepaintSignal();
};

#endif // GAMEANIMATION_H
