#ifndef BLINDANIMATION_H
#define BLINDANIMATION_H

#include "ui_mainwindow.h"
#include "GameSystem.h"
#include <QTimer>
#include <QObject>

class BlindAnimation : public QObject
{
    Q_OBJECT

public:
    explicit BlindAnimation(Ui_MainWindow *ui, GameSystem::Map *map, QObject *parent = nullptr);
    void Run(int interval);
private slots:
    void startAnimation();
private:
    QTimer *timer;
    Ui_MainWindow *ui;
    GameSystem::Map *map;
    int loop;
signals:
    void AnimationFinishSignal();
    void AnimationRepaintSignal();
};

#endif // BLINDANIMATION_H
