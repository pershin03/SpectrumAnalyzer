#include "widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>


Widget::Widget(QWidget *parent)
    : QWidget(parent), isRunning(false)
{
    setupUI();
    setupPlot();

    worker = new SpectrumWorker(this);
    connect(worker, &SpectrumWorker::spectrumUpdated, this, &Widget::updatePlot);
    connect(toggleButton, &QPushButton::clicked, this, &Widget::toggleControl);
}

Widget::~Widget() {
    if (isRunning) {
        worker->stop();
    }
    delete worker;
}

void Widget::setupUI() {
    plot = new QCustomPlot(this);


    ipAdress = new QLabel("IP-адрес");
    ipEdit = new QLineEdit("192.168.21.2", this);

    port = new QLabel("Порт");
    portEdit = new QLineEdit("5051", this);

    frequency_kHz = new QLabel("Частота, кГц");
    frequencyEdit = new QLineEdit("3000", this);
    freqValidator = new QIntValidator(3000, 30000, this);
    frequencyEdit->setValidator(freqValidator);



    toggleButton = new QPushButton("Старт", this);
    // toggleButton->setText("Стоп");

    mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(plot, 1);



    controlLayout = new QVBoxLayout();

    controlLayout->addWidget(ipAdress);
    controlLayout->addWidget(ipEdit);
    controlLayout->addWidget(port);
    controlLayout->addWidget(portEdit);
    controlLayout->addWidget(frequency_kHz);
    controlLayout->addWidget(frequencyEdit);
    controlLayout->addWidget(toggleButton);

    controlLayout->addStretch();

    mainLayout->addLayout(controlLayout);
    setLayout(mainLayout);
}

void Widget::setupPlot() {

    plot->addGraph();
    plot->xAxis->setRange(-24000, 24000);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->replot();

}

void Widget::toggleControl() {
    if (isRunning) {
        worker->stop();
        toggleButton->setText("Старт");
    } else {
        bool ok;
        int freq_kHz = frequencyEdit->text().toInt(&ok);

        if (!ok || freq_kHz < 3000 || freq_kHz > 30000) {
            QMessageBox::warning(this, "Ошибка", "Частота должна быть от 3000 до 30000 кГц");
            return;
        }

            std::string ip = ipEdit->text().toStdString();
            int port = portEdit->text().toInt();
            uint32_t freq = frequencyEdit->text().toUInt() * 1000; // kHz -> Hz

            worker->start(ip, port, freq);
            toggleButton->setText("Стоп");

    }
    isRunning = !isRunning;
}


void Widget::updatePlot(std::vector<double> spectrum) {

    int countIQ = spectrum.size();

    if(countIQ > x.size()) {
        x.resize(countIQ);
    }

    // if(countIQ > y.size()) {
    //     y.resize(countIQ);
    // }

    // if(countIQ > amplitudeSpectrum.size()) {
    //     amplitudeSpectrum.resize(countIQ);
    // }

    // double fs = 48000;
    // for(int i = 0; i < countIQ; i++) {
    //     double re = spectrum[2*i];
    //     double im = spectrum[2*i+1];
    //     amplitudeSpectrum[i] = sqrt(re*re + im*im);
    // }


    // double maxPower = *std::max_element(amplitudeSpectrum.begin(), amplitudeSpectrum.end());
    // if (maxPower > 0) {
        // for (auto& val : amplitudeSpectrum) {
        //     val = 10 * log10(val);
        // }
    // }


    QVector<double> amplitudeSpectrumVisual2(spectrum.cbegin(), spectrum.cend());



    for (int i = 0; i < countIQ; i++) {
        x[i] = -fs/2 + i*(fs/countIQ);
    }

    plot->graph(0)->setData(x, amplitudeSpectrumVisual2);
    // plot->yAxis->setRange(-100, 10);
    // plot->yAxis->rescale();
    plot->yAxis->setRange(0, 60);
    plot->replot();
}



