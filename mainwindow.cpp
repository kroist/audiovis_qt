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
    in = (double*) fftw_malloc(sizeof(double) * barCnt*2);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (barCnt+1));
    plan = fftw_plan_dft_r2c_1d(barCnt*2, in, out, FFTW_ESTIMATE);


#if SHOW_GNUPLOT
    pipe = popen("gnuplot -persist", "w");
#endif

    /* sample parameters */
    ss.format = PA_SAMPLE_S16LE;
    ss.channels = 1;
    ss.rate = 44100;

    s = pa_simple_new(NULL, "fftw", PA_STREAM_RECORD, "alsa_output.pci-0000_00_1f.3.analog-stereo.monitor", "ffting", &ss, NULL, &pb, NULL);



    //std::thread thr(&MainWindow::readFromPulse, this);
    thr = new std::thread([this]() {
        readFromPulse();
    });

    timer->start(50);

}

MainWindow::~MainWindow()
{
    fftw_free(in);
    fftw_free(out);
    fftw_destroy_plan(plan);
    pa_simple_free(s);
    delete ui;
    thr->join();
}

double abs(double x[2]) {
    return sqrt(x[0]*x[0]+x[1]*x[1]);
}



void MainWindow::readFromPulse() {
    while(true) {
        pa_simple_read(s, buf, sizeof(buf), NULL);
#if SHOW_GNUPLOT
        fprintf(pipe, "set title \"bass\"\n");
        fprintf(pipe, "set yrange [-33000:33000]\n");
        fprintf(pipe, "plot '-' with lines\n");
        for (int i = 0; i < BUFSZ; i++) {
            int rectNumber = i+1;
            int height = buf[i];
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
    for (int i = 0; i < barCnt*2; i++) {
        if (barCnt*2 < BUFSZ) {
            in[i] = buf[(int) (BUFSZ*(double)i/(barCnt*2)) ]; /* almost windowing function */
            in[i] /= 32766.0; /* rescale amplitude to [0...1] */
        }
        else {
            if (i < BUFSZ)
                in[i] = buf[i]/32766.0; /* just take prefix, should be fixed */
            else
                in[i] = 0;
        }
    }

    fftw_execute(plan);

    int rectNumber = 0;
    for (int i = 5; i+barWidth <= this->width()-5; i+=barWidth+4) {
        ++rectNumber;
        painter.setBrush(QColor().fromHsv(360/barCnt*rectNumber, 255, 255));

        int height = 2595*log10(1+(abs(out[rectNumber])/(double)(barCnt/2)*this->height())/700.0); /* convert amplitude to mel scale (2595*log10(1+f/700)) */


        painter.drawRect(i, this->height()-height-5, barWidth, height);

    }


}

