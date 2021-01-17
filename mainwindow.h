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


    FftwPlan* planL, *planR;

    pulseaudio_input* pulseInput;
    int channels;

    int barCnt = 60;
    Ui::MainWindow *ui;
    std::thread *thr;
    bool read_paused;
    std::condition_variable cv_;
    std::mutex m_;
    std::vector<std::string> devices;
    bool melspec = false;
    int frequency_scale = 100;

    FILE *pipe;

protected:
    void paintEvent(QPaintEvent*);
    void readFromPulse();
    void indexChanged1(int);
    void indexChanged2(int);
    void checkbox(int);
    void sliderChanged(int);
    void sliderChanged2(int);
    static void sourcelist_cb(pa_context*, const pa_source_info*, int, void*);
    static void context_state_cb(pa_context*, void*);
    int findDevices();
};
#endif // MAINWINDOW_H
