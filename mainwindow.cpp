#include "mainwindow.h"
#include "ui_mainwindow.h"

#define SHOW_GNUPLOT 0

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    ui->setupUi(this);
    for (int i = 5; i+barWidth <= this->width()-5; i+=barWidth+4)
        ++barCnt;

    /* initializing FFTW plan, should probably be written as C++ class */

    plan = new FftwPlan(barCnt*2, barCnt+1);

#if SHOW_GNUPLOT
    pipe = popen("gnuplot -persist", "w");
#endif


    pulseInput = new pulseaudio_input();

    //std::thread thr(&MainWindow::readFromPulse, this);
    thr = new std::thread([this]() {
        readFromPulse();
    });

    timer->start(50);

}

MainWindow::~MainWindow()
{
    delete pulseInput;
    delete plan;
    delete ui;
    thr->join();
}


void MainWindow::readFromPulse() {
    while(true) {
        pulseInput->read_buf();
#if SHOW_GNUPLOT
        fprintf(pipe, "set title \"bass\"\n");
        fprintf(pipe, "set yrange [-33000:33000]\n");
        fprintf(pipe, "plot '-' with lines\n");
        int bufsz = pulseInput->buf_size();
        for (int i = 0; i < bufsz; i++) {
            int rectNumber = i+1;
            int height = pulseInput->buf_get(i);
            fprintf(pipe, "%d\t%d\n", rectNumber, height);
        }
        fprintf(pipe, "%s\n", "e");
        fflush(pipe);
#endif
    }
}



void MainWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    /* reading from another thread, don't care about any thread-safety (maybe should) */

    plan->pulse_to_in(pulseInput);

    plan->exec();

    int rectNumber = 0;
    for (int i = 5; i+barWidth <= this->width()-5; i+=barWidth+4) {
        ++rectNumber;
        painter.setBrush(QColor().fromHsv(360/barCnt*rectNumber, 255, 255));

        int height = 2595*log10(1+(std::abs(plan->get_fft(rectNumber) )/(double)(barCnt/2)*this->height())/700.0); /* convert amplitude to mel scale (2595*log10(1+f/700)) */


        painter.drawRect(i, this->height()-height-5, barWidth, height);

    }


}

