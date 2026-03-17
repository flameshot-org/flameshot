#include "uinputscrollcontroller.h"

#if defined(Q_OS_LINUX)

#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>

UInputScrollController::UInputScrollController()
  : m_fd(-1)
{
}

UInputScrollController::~UInputScrollController()
{
    shutdown();
}

bool UInputScrollController::emitEvent(unsigned short type, unsigned short code, int value)
{
    if (m_fd < 0) {
        return false;
    }

    input_event ev{};
    ev.type = type;
    ev.code = code;
    ev.value = value;

    return write(m_fd, &ev, sizeof(ev)) == sizeof(ev);
}

bool UInputScrollController::syncEvents()
{
    return emitEvent(EV_SYN, SYN_REPORT, 0);
}

bool UInputScrollController::init()
{
    if (m_fd >= 0) {
        return true;
    }

    m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (m_fd < 0) {
        return false;
    }

    if (ioctl(m_fd, UI_SET_EVBIT, EV_KEY) < 0) {
        shutdown();
        return false;
    }

    if (ioctl(m_fd, UI_SET_KEYBIT, BTN_LEFT) < 0) {
        shutdown();
        return false;
    }

    if (ioctl(m_fd, UI_SET_EVBIT, EV_REL) < 0) {
        shutdown();
        return false;
    }

    if (ioctl(m_fd, UI_SET_RELBIT, REL_WHEEL) < 0) {
        shutdown();
        return false;
    }

    if (ioctl(m_fd, UI_SET_RELBIT, REL_HWHEEL) < 0) {
        shutdown();
        return false;
    }

    uinput_setup usetup{};
    std::snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "flameshot-scroll-capture");
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    usetup.id.version = 1;

    if (ioctl(m_fd, UI_DEV_SETUP, &usetup) < 0) {
        shutdown();
        return false;
    }

    if (ioctl(m_fd, UI_DEV_CREATE) < 0) {
        shutdown();
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    return true;
}

void UInputScrollController::shutdown()
{
    if (m_fd >= 0) {
        ioctl(m_fd, UI_DEV_DESTROY);
        close(m_fd);
        m_fd = -1;
    }
}

bool UInputScrollController::isReady() const
{
    return m_fd >= 0;
}

bool UInputScrollController::scrollDown(int steps)
{
    if (m_fd < 0) {
        return false;
    }

    for (int i = 0; i < steps; ++i) {
        if (!emitEvent(EV_REL, REL_WHEEL, -1)) {
            return false;
        }
        if (!syncEvents()) {
            return false;
        }
    }

    return true;
}

bool UInputScrollController::scrollUp(int steps)
{
    if (m_fd < 0) {
        return false;
    }

    for (int i = 0; i < steps; ++i) {
        if (!emitEvent(EV_REL, REL_WHEEL, 1)) {
            return false;
        }
        if (!syncEvents()) {
            return false;
        }
    }

    return true;
}

bool UInputScrollController::leftClick()
{
    if (m_fd < 0) {
        return false;
    }

    if (!emitEvent(EV_KEY, BTN_LEFT, 1)) {
        return false;
    }
    if (!syncEvents()) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(40));

    if (!emitEvent(EV_KEY, BTN_LEFT, 0)) {
        return false;
    }
    if (!syncEvents()) {
        return false;
    }

    return true;
}

#endif
