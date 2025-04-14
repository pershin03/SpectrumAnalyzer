QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    spectrumworker.cpp \
    widget.cpp

HEADERS += \
    spectrumworker.h \
    vmpl_defs.h \
    widget.h

# DEFINES += QCUSTOMPLOT_USE_LIBRARY


unix|win32: LIBS += -lws2_32 \
                    -L "C:\Qt\fftw3_mingw1310" -lfftw3 \
                    -L$$PWD -lqcustomplot2

# CONFIG(debug, release|debug) {
#         LIBS += -L$$PWD -lqcustomplotd2_debug

# } else {
#         LIBS += -L$$PWD -lqcustomplot2_release
# }

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
