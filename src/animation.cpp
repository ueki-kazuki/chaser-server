#include "Animation.h"
#include "ui_mainwindow.h"
#include "GameSystem.h"
#include "QRandomGenerator"
#include "QTimer"

Animation::Animation(Ui_MainWindow *ui, GameSystem::Map *map, QObject *parent)
    : QObject(parent)
{
    this->ui = ui;
    this->map = map;

    this->loop = 1;
    this->f = Field<GameSystem::MAP_OVERLAY>(map->size.y(),
                QVector<GameSystem::MAP_OVERLAY>(map->size.x(),GameSystem::MAP_OVERLAY::ERASE));
    this->ANIMATION_TYPE = QRandomGenerator::global()->generate() % ANIMATION_SIZE;
}

Animation::~Animation() {
    timer->stop();
}

void Animation::Run(int interval) {
    qDebug() << Q_FUNC_INFO << "Run";
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
    timer->start(interval);
}

void Animation::startAnimation() {
    int count = 0;

    this->ui->Field->RefreshOverlay();

    QPoint pos[2];
    qDebug() << Q_FUNC_INFO << " ANIMATION_TYPE " << QString::number(ANIMATION_TYPE);
    if(ANIMATION_TYPE == 0){
        //ランダムにワサッて
        //一度に2マス表示させるのですわ
        int i_max = 2;
        if(loop == 1)i_max ++;
        for(int i=0;i<i_max;i++){
            do{
                pos[i].setX(QRandomGenerator::global()->generate() % map->size.x());
                pos[i].setY(QRandomGenerator::global()->generate() % map->size.y());
            }while(loop < map->size.x() * map->size.y() && f[pos[i].y()][pos[i].x()] != GameSystem::MAP_OVERLAY::ERASE);
            f[pos[i].y()][pos[i].x()] = GameSystem::MAP_OVERLAY::NOTHING;
        }
        for(int i=0;i<map->size.y();i++){
            for(int j=0;j<map->size.x();j++){
                ui->Field->overlay[i][j] = f[i][j];
            }
        }
    }else if(ANIMATION_TYPE == 1){
        //上からガーって
        for(int j=0;j<map->size.y();j++){
            for(int k=0;k<map->size.x();k++){
                if(count >= loop){
                    ui->Field->overlay[j][k] = f[j][k];
                }
                count++;
            }
        }
    }else if(ANIMATION_TYPE == 2){
        //なんかはさみ込む感じで
        for(int j=0;j<map->size.y();j++){
            for(int k=0;k<map->size.x();k++){
                if(count*2 < loop){
                    f[map->size.y() - j - 1][map->size.x() - k - 1] = GameSystem::MAP_OVERLAY::NOTHING;
                    f[j][k] = GameSystem::MAP_OVERLAY::NOTHING;
                }
                count++;
            }
        }

        for(int i=0;i<map->size.y();i++){
            for(int j=0;j<map->size.x();j++){
                ui->Field->overlay[i][j] = f[i][j];
            }
        }
    }else if(ANIMATION_TYPE == 3){
        //下からガーって
        for(int j=map->size.y()-1;j>=0;j--){
            for(int k=map->size.x()-1;k>=0;k--){
                if(count >= loop){
                    ui->Field->overlay[j][k] = f[j][k];
                }
                count++;
            }
        }
    }
    if(loop >= map->size.x() * map->size.y()){
        qDebug() << Q_FUNC_INFO << " LoopEnd " << QString::number(loop);
        disconnect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
        timer->stop();
        Q_EMIT(AnimationFinishSignal());
    }
    loop += 2;
    qDebug() << Q_FUNC_INFO << " Repaint " << QString::number(loop);
    Q_EMIT(AnimationRepaintSignal());

}
