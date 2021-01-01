#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <fftw3.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
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

    const static int BUFSZ = 2048;
    const int FRAMERATE = 44100;
    int16_t buf[BUFSZ];

    double* in;
    fftw_complex *out;
    fftw_plan plan;

    pa_simple *s;
    pa_sample_spec ss;
    const pa_buffer_attr pb = {.maxlength = (uint32_t)-1, .fragsize = BUFSZ}; /*without this read from api is too slow */

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
