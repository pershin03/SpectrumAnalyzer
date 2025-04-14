#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include "./qcustomplot.h"
#include "spectrumworker.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void toggleControl();
    void updatePlot(std::vector<double> spectrum);


private:
    void setupUI();
    void setupPlot();


    QLabel *ipAdress;
    QLineEdit *ipEdit;

    QLabel *port;
    QLineEdit *portEdit;

    QLabel *frequency_kHz;
    QLineEdit *frequencyEdit;
    QIntValidator* freqValidator;

    QVector<double> x;
    QVector<double> y;

    double fs = 48000;

    QPushButton *toggleButton;

    QCustomPlot *plot;

    QHBoxLayout *mainLayout;
    QVBoxLayout *controlLayout;

    SpectrumWorker *worker;
    bool isRunning;

};

#endif // WIDGET_H
