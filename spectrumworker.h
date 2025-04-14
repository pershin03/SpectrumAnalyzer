#ifndef SPECTRUMWORKER_H
#define SPECTRUMWORKER_H
#include <QObject>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <fftw3_mingw1310/fftw3.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "vmpl_defs.h"

#define DEFAULT_BUFLEN 1024
#define DEFAULT_RTP_BUFLEN 4108
#define RTP_HEADER_LEN 12
#define RTP_DATA_LEN (DEFAULT_RTP_BUFLEN - RTP_HEADER_LEN)
#define AMOUNT_OF_COMPLEX_NUMBERS 512
#define QUANTITY_BYTES_IN_IQ sizeof(double)
#define SIZE_FFT 2048
#define MAX_QUANTITY_PACKET SIZE_FFT / (RTP_DATA_LEN / QUANTITY_BYTES_IN_IQ)
#define MAX_SIZE_UDP_PACKET 65535


class SpectrumWorker : public QObject {
    Q_OBJECT

public:


    explicit SpectrumWorker(QObject *parent = nullptr);
    ~SpectrumWorker();

    void start(const std::string& ip, int ctrlPort, uint32_t frequency);
    void stop();
    bool isRunning() const;

signals:
    void spectrumUpdated( std::vector<double> spectrum);


private:
    void run();
    void setupFFT();


    std::mutex mx;
    std::thread m_thread;
    std::atomic<bool> m_running{false};

    std::string m_ip;
    int m_ctrlPort;
    int m_iqPort;
    uint32_t m_frequency;

    SOCKET m_sockCtrl{INVALID_SOCKET};
    SOCKET m_sockIq{INVALID_SOCKET};

    int initWinsock();
    SOCKET createSocket(const std::string& ip_remote_address, int remote_port, int local_port);
    set_frequency_req setFreq(const uint32_t frequency_);
    IQ_req requestIQ(uint32_t state);
    std::vector<char> recvbuf;

    fftw_plan m_plan;
    std::vector<double> m_in;
    std::vector<double> m_out;
    std::vector<double> amplitudeSpectrum;



    int countPackets = 0;


    void copyBytesFromPacket(char* inputBuffer, std::vector<double>& outputBuffer, int offset);

};
Q_DECLARE_METATYPE(std::vector<double>);



#endif // SPECTRUMWORKER_H
