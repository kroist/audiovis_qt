#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <iostream>

#define SHOW_GNUPLOT 0

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    ui->setupUi(this);

    /* initializing FFTW plan, should probably be written as C++ class */

    //plan = new FftwPlan(barCnt*2, barCnt+1);
    planL = new FftwPlan(1024, 513);
    planR = new FftwPlan(1024, 513);
    this->findDevices();
    this->channels = 1;
    pulseInput = new pulseaudio_input(2, this->devices[0]);
    ui->comboBox_2->clear();
    for (std::string x : this->devices) {
        ui->comboBox_2->addItem(QString().fromStdString(x));
    }
    this->read_paused = false;
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){indexChanged1(index);});
    connect(ui->comboBox_2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){indexChanged2(index);});
    connect(ui->checkBox, QOverload<int>::of(&QCheckBox::stateChanged), this, [=](int index){checkbox(index);});
    connect(ui->horizontalSlider, QOverload<int>::of(&QSlider::valueChanged), this, [=](int index){sliderChanged(index);});
    connect(ui->horizontalSlider_2, QOverload<int>::of(&QSlider::valueChanged), this, [=](int index){sliderChanged2(index);});



#if SHOW_GNUPLOT
    pipe = popen("gnuplot -persist", "w");
#endif

//"alsa_output.pci-0000_00_1f.3.analog-stereo.monitor"

    //std::thread thr(&MainWindow::readFromPulse, this);
    thr = new std::thread([this]() {
        readFromPulse();
    });

    timer->start(50);

}

MainWindow::~MainWindow()
{
    delete pulseInput;
    delete planL;
    if (planR != nullptr)
        delete planR;
    delete ui;
    thr->join();
}


void MainWindow::readFromPulse() {
    while(true) {
        while(read_paused) {
            std::unique_lock<decltype(this->m_)> l(this->m_);
            this->cv_.wait(l);
            l.unlock();
        }
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
void MainWindow::indexChanged1(int x) {
    if (this->devices[x] == "1")
        this->channels = 1;
    else
        this->channels = 2;
}
void MainWindow::indexChanged2(int x) {
    {
        std::unique_lock<decltype(this->m_)>(this->m_);
        read_paused = true;
    }
    delete pulseInput;
    pulseInput = new pulseaudio_input(2, this->devices[x]);
    {
        std::unique_lock<decltype(this->m_)>(this->m_);
        read_paused = false;
        this->cv_.notify_one();
    }
}
void MainWindow::checkbox(int x) {
    this->melspec ^= 1;
    std::cout << this->width() << ' ' << barCnt << std::endl;
    double ratio_of_spacing = 1 + 1.0/4.0;
    int barWidth = ((double)this->width())/(barCnt * ratio_of_spacing);
    if (barWidth <= 0) {
        barWidth = 1;
    }
    int spacing = ((double)this->width())/(barCnt * ratio_of_spacing) * (ratio_of_spacing-1);
    std::cout << barWidth << ' ' << spacing << ' ' << spacing+(barWidth+spacing)*barCnt << std::endl;
}

void MainWindow::sliderChanged(int x) {
    this->frequency_scale = x;
}
void MainWindow::sliderChanged2(int x) {
    this->barCnt = x;
}

void MainWindow::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    ui->horizontalSlider_2->setMaximum((this->width()+1)/2);

    /* reading from another thread, don't care about any thread-safety (maybe should) */

    planL->pulse_to_in(pulseInput, 0);

    planL->exec();

    if (channels == 2) {
        planR->pulse_to_in(pulseInput, 1);
        planR->exec();
    }
    painter.setPen(Qt::NoPen);

    double ratio_of_spacing = 1 + 1.0/4.0;
    int barWidth = ((double)this->width())/(barCnt * ratio_of_spacing);
    if (barWidth == 1) {
        barCnt = this->width();
    }
    if (barWidth <= 0) {
        barWidth = 1;
    }

    int spacing = barWidth * (ratio_of_spacing-1);
    if (channels == 1) {
        int i = spacing;
        for (int rectNumber = 1; rectNumber <= barCnt; rectNumber++) {
            painter.setBrush(QColor().fromHsv((int)(360.0/barCnt*rectNumber), 255, 255));

            int gtId;
            if (barCnt <= 512)
                gtId = (int)((double)this->frequency_scale/(double)barCnt*(double)rectNumber);
            else
                gtId = ((double)rectNumber)/((double)barCnt/512.0);

            int height;
            if (!this->melspec)
                height = std::abs(planL->get_fft(gtId) )/(double)(44100/1024)*this->height();
            else
                height = 2595*log10(1+(std::abs(planL->get_fft(gtId) )/(double)(44100/1024)*this->height())/700.0); /* convert amplitude to mel scale (2595*log10(1+f/700)) */


            painter.drawRect(i, this->height()-height-5, barWidth, height);

            i += barWidth+spacing;

        }
    }
    else {
        int i = spacing;
        for (int rectNumber = barCnt/2; rectNumber >= 1; rectNumber--) {
            painter.setBrush(QColor().fromHsv((int)(360.0/(barCnt/2)*rectNumber), 255, 255));

            int gtId = (int)((double)this->frequency_scale/(double)(barCnt/2)*(double)rectNumber);

            int height;
            if (!this->melspec)
                height = std::abs(planL->get_fft(gtId) )/(double)(44100/1024)*this->height();
            else
                height = 2595*log10(1+(std::abs(planL->get_fft(gtId) )/(double)(44100/1024)*this->height())/700.0); /* convert amplitude to mel scale (2595*log10(1+f/700)) */


            painter.drawRect(i, this->height()-height-5, barWidth, height);
            i += barWidth+spacing;
        }
        for (int rectNumber = 1; rectNumber+barCnt/2 <= barCnt; rectNumber++) {
            painter.setBrush(QColor().fromHsv((int)(360.0/(barCnt/2)*rectNumber), 255, 255));

            int gtId = (int)((double)this->frequency_scale/(double)(barCnt/2)*(double)rectNumber);

            int height;

            if (!this->melspec)
                height = std::abs(planR->get_fft(gtId) )/(double)(44100/1024)*this->height();
            else
                height = 2595*log10(1+(std::abs(planR->get_fft(gtId) )/(double)(44100/1024)*this->height())/700.0); /* convert amplitude to mel scale (2595*log10(1+f/700)) */


            painter.drawRect(i, this->height()-height-5, barWidth, height);
            i += barWidth+spacing;

        }
    }

}

void MainWindow::sourcelist_cb(pa_context* c, const pa_source_info *i, int eol, void* userdata) {
    std::vector<std::string> *devices = (std::vector<std::string>*)userdata;
    if (eol > 0) {
        return;
    }
    devices->push_back(std::string(i->name));
}

void MainWindow::context_state_cb(pa_context* c, void* userdata) {
    int *pa_ready = (int*)userdata;
    switch(pa_context_get_state(c)) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            *pa_ready = 2;
            break;
        case PA_CONTEXT_READY:
            *pa_ready = 1;
            break;

    }
}

int MainWindow::findDevices() {

    pa_mainloop* loop;
    pa_mainloop_api* loop_api;
    pa_operation* op;
    pa_context* ctx;


    int state = 0, pa_ready = 0;
    loop = pa_mainloop_new();
    loop_api = pa_mainloop_get_api(loop);

    ctx = pa_context_new(loop_api, "test");

    pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

    pa_context_set_state_callback(ctx, context_state_cb, &pa_ready);


    while(1) {
        if (pa_ready == 0) {
            pa_mainloop_iterate(loop, 1, NULL);
            continue;
        }
        if (pa_ready == 2) {
            pa_context_disconnect(ctx);
            pa_context_unref(ctx);
            pa_mainloop_free(loop);
            return -1;
        }
        switch(state) {
            case 0:
                op = pa_context_get_source_info_list(ctx, sourcelist_cb, &this->devices);
                ++state;
                break;
            case 1:
                if (pa_operation_get_state(op) == PA_OPERATION_DONE) {
                    pa_operation_unref(op);
                    pa_context_disconnect(ctx);
                    pa_context_unref(ctx);
                    pa_mainloop_free(loop);
                    return 0;
                }
                break;
            default:
                return -1;

        }
        pa_mainloop_iterate(loop, 1, NULL);

    }

}
