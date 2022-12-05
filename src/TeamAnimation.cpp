#include "TeamAnimation.h"

TeamAnimation::TeamAnimation(Ui_MainWindow *ui, GameSystem::Map *map, QObject *parent)
    : QObject{parent}
{
    this->ui = ui;
    this->map = map;
    this->team_count = 0;
}

void TeamAnimation::Run(int interval) {
    qDebug() << Q_FUNC_INFO << "Run";
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
    timer->start(interval);
}

void TeamAnimation::startAnimation(){

    ui->Field->team_pos[team_count] = this->map->team_first_point[team_count];

    if(team_count == TEAM_COUNT){
        disconnect(timer,SIGNAL(timeout()),this,SLOT(startAnimation()));
        timer->stop();
        Q_EMIT(AnimationFinishSignal());
    }else{
        ui->Field->field.discover[ui->Field->team_pos[team_count].y()]
                [ui->Field->team_pos[team_count].x()] = GameSystem::Discoverer::Cool;
    }
    team_count++;
    Q_EMIT(AnimationRepaintSignal());
}
