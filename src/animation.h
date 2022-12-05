#ifndef ANIMATION_H
#define ANIMATION_H

#include "ui_mainwindow.h"
#include "GameSystem.h"
#include <QTimer>
#include <QObject>

static int ANIMATION_SIZE = 4;

class Animation : public QObject
{
    Q_OBJECT

public:
    explicit Animation(Ui_MainWindow *ui, GameSystem::Map *map, QObject *parent = nullptr);
    ~Animation();
    void Run(int interval);
private slots:
    void startAnimation();
private:
    QTimer *timer;
    Ui_MainWindow *ui;
    GameSystem::Map *map;
    Field<GameSystem::MAP_OVERLAY> f;
    int loop;
    int ANIMATION_TYPE;
signals:
    void AnimationFinishSignal();
    void AnimationRepaintSignal();
};

#endif // ANIMATION_H
