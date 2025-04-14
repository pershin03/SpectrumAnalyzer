#include "qdebug.h"
#include <spectrumworker.h>


//initialization Winsock
int SpectrumWorker::initWinsock() {
    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }
    return 0;
}

SOCKET SpectrumWorker::createSocket(const std::string& ip_remote_address, int remote_port, int local_port = 0) {

    u_long iMode = 1;

    struct sockaddr_in hints;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        std::cout << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    int iResult = ioctlsocket(sock, FIONBIO, &iMode);
    if (iResult != NO_ERROR) {

        std::cout << "ioctlsocket failed with error: " << iResult << std::endl;
    }

    memset(&hints, 0, sizeof(sockaddr_in));
    hints.sin_family = AF_INET;
    hints.sin_port = htons(local_port);
    inet_pton(AF_INET, 0, &hints.sin_addr);
    //inet_pton(AF_INET, "192.168.21.10", &hints.sin_addr);


    iResult = bind(sock, (struct sockaddr*)&hints, sizeof(hints));
    if (iResult < 0)
    {
        std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    memset(&hints, 0, sizeof(sockaddr_in));
    hints.sin_family = AF_INET;
    hints.sin_port = htons(remote_port);
    inet_pton(AF_INET, ip_remote_address.c_str(), &hints.sin_addr);
    iResult = ::connect(sock, (struct sockaddr*)&hints, sizeof(hints));
    if (iResult < 0)
    {
        std::cout << "Unable to connect " << iResult << std::endl;
        WSACleanup();
        return -1;
    }

    return sock;
}

set_frequency_req SpectrumWorker::setFreq(const uint32_t frequency_) {

    set_frequency_req setfrequency;



    setfrequency.header.data_length = 0x0400;

    setfrequency.header.cmd_id = SetFrequency;
    setfrequency.frequency_Hz = frequency_;

    return setfrequency;

}

IQ_req SpectrumWorker::requestIQ(uint32_t state) {

    IQ_req IQreq;

    IQreq.header.data_length = 0x0400;


    IQreq.header.cmd_id = SetRtpCtrl;
    IQreq.state = state;

    return IQreq;

}

// std::vector<double> getVectorIQforFFT(const char* buff) {
//     int start_index_for_IQ_packet = RTP_HEADER_LEN;
//     int length_of_my_IQ_packet = (DEFAULT_RTP_BUFLEN - RTP_HEADER_LEN) / 4;
//     std::vector<int32_t> my_IQ_packet(length_of_my_IQ_packet);
//     std::memcpy(my_IQ_packet.data(), &buff[start_index_for_IQ_packet], length_of_my_IQ_packet * sizeof(int32_t));

//     std::vector<double> complex_IQ_packet(my_IQ_packet.begin(), my_IQ_packet.end());

//     return complex_IQ_packet;
// }

void SpectrumWorker::copyBytesFromPacket(char* inputBuffer, std::vector<double>& outputBuffer, int offset) {
    int* ptrInputIQ = (int*)(inputBuffer + RTP_HEADER_LEN);
    int size = RTP_DATA_LEN/sizeof(int);

    for(int i = 0; i < size; i++)
    {
        outputBuffer[i + offset] = ptrInputIQ[i];
    }
}


SpectrumWorker::SpectrumWorker(QObject *parent) : QObject(parent), recvbuf(MAX_SIZE_UDP_PACKET), m_plan(nullptr), amplitudeSpectrum(SIZE_FFT)
{
    setupFFT();
}

SpectrumWorker::~SpectrumWorker() {
    stop();
    fftw_destroy_plan(m_plan);
    fftw_cleanup();
    m_plan = nullptr;


}

void SpectrumWorker::start(const std::string& ip, int ctrlPort, uint32_t frequency) {
    if (m_running) return;

    m_ip = ip;
    m_ctrlPort = ctrlPort;
    m_iqPort = ctrlPort - 1;
    m_frequency = frequency;


    m_running.store(true);
    m_thread = std::thread(&SpectrumWorker::run, this);
}

void SpectrumWorker::stop() {


    if (!m_running.load()) return;


    m_running.store(false);




    if (m_sockCtrl != INVALID_SOCKET) {
        IQ_req stopCmd = requestIQ(0); // Состояние 0 — остановка
        if (send(m_sockCtrl, (const char*)&stopCmd, sizeof(stopCmd), 0) == SOCKET_ERROR) {
            std::cout << "[ERROR] Failed to send stop command: " << WSAGetLastError() << std::endl;
        } else {
            std::cout << "[INFO] Stop command sent successfully" << std::endl;
        }
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }


    // closesocket(m_sockCtrl);
    // closesocket(m_sockIq);
    // WSACleanup();
}



bool SpectrumWorker::isRunning() const {
    return m_running;
}


void SpectrumWorker::setupFFT() {
    if (m_plan) {
        fftw_destroy_plan(m_plan);
    }

    m_in.resize(SIZE_FFT*2);
    m_out.resize(SIZE_FFT*2);

    m_plan = fftw_plan_dft_1d(SIZE_FFT,
                              (fftw_complex*)(m_in.data()),
                              (fftw_complex*)(m_out.data()),
                              FFTW_FORWARD,
                              FFTW_ESTIMATE);
}



void SpectrumWorker::run() {

    if (initWinsock() != 0) {
        qDebug() << "Не удалось инициализировать библиотеку WinSock";
        return;
    }

    m_sockCtrl = createSocket(m_ip, m_ctrlPort, 45601);
    if (m_sockCtrl == INVALID_SOCKET) {


        closesocket(m_sockIq);
        m_sockIq = INVALID_SOCKET;
        WSACleanup();
        qDebug() << "Не удалось создать сокет IQ";
        return;
    }

    m_sockIq = createSocket(m_ip, m_ctrlPort - 1, 45600);
    if (m_sockIq == INVALID_SOCKET) {

        closesocket(m_sockIq);
        m_sockIq = INVALID_SOCKET;
        WSACleanup();
        qDebug() << "Не удалось создать сокет ctrl";
        return;
    }

    set_frequency_req set_freq = setFreq(m_frequency);
    IQ_req rq_IQ = requestIQ(1);


    if (send(m_sockCtrl, (const char*)&set_freq, sizeof(set_freq), 0) == SOCKET_ERROR) {

        closesocket(m_sockCtrl);
        closesocket(m_sockIq);
        WSACleanup();
        return;
    }

    if (send(m_sockCtrl, (const char*)&rq_IQ, sizeof(IQ_req), 0) == SOCKET_ERROR) {

        closesocket(m_sockCtrl);
        closesocket(m_sockIq);
        WSACleanup();
        return;
    }


    while(m_running.load()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(m_sockCtrl, &readSet);
        FD_SET(m_sockIq, &readSet);

        timeval timeout = { 0, 10000 };


        int res = select(0, &readSet, nullptr, nullptr, &timeout);

        if(res > 0) {

            int recv_data;

            if (FD_ISSET(m_sockCtrl, &readSet)) {
                std::cout << "Socket1 is ready for reading!" << std::endl;
                if (res > 0) {

                    // char recvbuf[DEFAULT_BUFLEN];
                    // ZeroMemory(recvbuf, DEFAULT_BUFLEN);

                    recvbuf.clear();
                    recv_data = recv(m_sockCtrl, recvbuf.data(), MAX_SIZE_UDP_PACKET, 0);
                    if (recv_data > 0) {
                        std::cout << "Bytes recieved 1: " << recv_data << std::endl;
                    }
                    else if (recv_data == 0) {
                        std::cout << "Connection closed 1" << std::endl;
                    }

                    else {
                        std::cout << "recv 1 failed: \n" << WSAGetLastError() << std::endl;
                    }
                }
            }


            if (FD_ISSET(m_sockIq, &readSet)) {
                std::cout << "Socket2 is ready for reading!" << std::endl;
                if (res > 0) {


                    recvbuf.clear();
                    recv_data = recv(m_sockIq, recvbuf.data(), MAX_SIZE_UDP_PACKET, 0);
                    if (recv_data > 0) {
                        std::cout << "Bytes recieved 2: " << recv_data << std::endl;


                        copyBytesFromPacket(recvbuf.data(), m_in, countPackets*(AMOUNT_OF_COMPLEX_NUMBERS*2));
                        countPackets++;

                        if(countPackets == MAX_QUANTITY_PACKET)
                        {
                            fftw_execute(m_plan);
                            countPackets = 0;
                            std::rotate(std::begin(m_out), std::begin(m_out)+m_out.size()/2, std::end(m_out));

                            for(int i = 0; i < SIZE_FFT; i++) {
                                double re = m_out[2*i];
                                double im = m_out[2*i+1];
                                amplitudeSpectrum[i] =10 * log10(sqrt(re*re + im*im));
                            }


                            emit spectrumUpdated(amplitudeSpectrum);
                        }

                    }
                    else if (recv_data == 0) {
                        std::cout << "Connection closed 2" << std::endl;
                    }

                    else {
                        std::cout << "recv 2 failed: \n" << WSAGetLastError() << std::endl;
                    }
                }
            }
        }
    }


    closesocket(m_sockCtrl);
    closesocket(m_sockIq);
    WSACleanup();
}

