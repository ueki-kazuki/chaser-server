#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QKeyEvent>
#include "startupdialog.h"
#include <fstream>
#include <QDateTime>
#include <QFile>
#include <QDataStream>
#include <QMediaPlayer>
#include <QThread>
#include "StableLog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:

    int FRAME_RATE = 150;   //ゲームフレームレート
    StartupDialog* startup; //スタートアップダイアログ

    bool silent;

    bool dark;              //暗転処理
    bool isbotbattle;       //ボット戦モード

    QFile* file;    //ログファイル
    StableLog log;//ログストリーム
    int anime_map_time  = 6000;//マップ構築アニメーション時間
    int anime_team_time = 2000;//チーム配置アニメーション時間
    int anime_blind_time = 1000;//まっくらアニメーション時間

protected:
    void keyPressEvent(QKeyEvent* event);
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    //音楽
    QMediaPlayer *bgm;

private slots:
    void SaveFile();

    void initStartupDialog();
    void initGame();
    //決着
    void GameFinished();

    void StartAnimation();
    void ShowTeamAnimation();
    void StartGameAnimation();
    void StartBlindAnimation();
    void on_pushButtonClose_released();
};

#endif // MAINWINDOW_H
