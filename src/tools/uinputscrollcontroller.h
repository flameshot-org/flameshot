#pragma once

#include <QtCore/QtGlobal>

#if defined(Q_OS_LINUX)

class UInputScrollController
{
public:
    UInputScrollController();
    ~UInputScrollController();

    bool init();
    void shutdown();

    bool isReady() const;

    bool scrollDown(int steps = 1);
    bool scrollUp(int steps = 1);
    bool leftClick();

private:
    bool emitEvent(unsigned short type, unsigned short code, int value);
    bool syncEvents();

private:
    int m_fd;
};

#endif
