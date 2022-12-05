#ifndef TEAMANIMATION_H
#define TEAMANIMATION_H

#include "ui_mainwindow.h"
#include "GameSystem.h"
#include <QTimer>
#include <QObject>

class TeamAnimation : public QObject
{
    Q_OBJECT

public:
    explicit TeamAnimation(Ui_MainWindow *ui, GameSystem::Map *map, QObject *parent = nullptr);
    void Run(int interval);
private slots:
    void startAnimation();

signals:
private:
    QTimer *timer;
    Ui_MainWindow *ui;
    GameSystem::Map *map;
    int team_count;
signals:
    void AnimationFinishSignal();
    void AnimationRepaintSignal();

};

#endif // TEAMANIMATION_H
