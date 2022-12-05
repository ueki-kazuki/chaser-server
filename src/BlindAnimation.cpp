#include "BlindAnimation.h"
#include "ui_mainwindow.h"
#include "GameSystem.h"
#include "QRandomGenerator"
#include "QTimer"

BlindAnimation::BlindAnimation(Ui_MainWindow *ui, GameSystem::Map *map, QObject *parent)
    : QObject(parent)
{
    this->ui = ui;
    this->map = map;

    this->loop = 1;
}

void BlindAnimation::Run(int interval) {
    qDebug() << Q_FUNC_INFO << "Run";
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
    timer->start(interval);
}

void BlindAnimation::startAnimation() {
    this->ui->Field->RefreshOverlay();

    QPoint pos[2];
    //ランダムにワサッて
    for(int i=0;i<2;i++){
        do{
            pos[i].setX(QRandomGenerator::global()->generate() % this->map->size.x());
            pos[i].setY(QRandomGenerator::global()->generate() % this->map->size.y());
        }while(loop < this->map->size.x() * this->map->size.y() &&
               ui->Field->field.discover[pos[i].y()][pos[i].x()] == GameSystem::Discoverer::Unknown);
        ui->Field->field.discover[pos[i].y()][pos[i].x()] = GameSystem::Discoverer::Unknown;
    }

    if(loop >= map->size.x() * map->size.y()){
        for(auto& v : this->ui->Field->field.discover)v = QVector<GameSystem::Discoverer>
                (this->ui->Field->field.size.x(),GameSystem::Discoverer::Unknown);
        disconnect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
        timer->stop();
        Q_EMIT(AnimationFinishSignal());
    }
    loop += 2;
    qDebug() << Q_FUNC_INFO << " Repaint " << QString::number(loop);
    Q_EMIT(AnimationRepaintSignal());

}
