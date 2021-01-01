#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include "fftwplan.h"
#include "pulseaudio_input.h"
#include <thread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    const int FRAMERATE = 44100;


    FftwPlan* plan;

    pulseaudio_input* pulseInput;

    const int barWidth = 20;
    int barCnt = 0;
    Ui::MainWindow *ui;
    std::thread *thr;

    FILE *pipe;

protected:
    void paintEvent(QPaintEvent*);
    void readFromPulse();
};
#endif // MAINWINDOW_H
