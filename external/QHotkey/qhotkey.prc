mac: LIBS += -framework Carbon
else:win32: LIBS += -luser32
else:unix {
	equals(QT_MAJOR_VERSION, 6) {
		lessThan(QT_VERSION, 6.2.0) {
			error("Qt 6.2.0 or greater is required when using Qt6")
		}
	}
	else {
		lessThan(QT_MAJOR_VERSION, 6) {
			QT += x11extras
		}
	}
	LIBS += -lX11
}
