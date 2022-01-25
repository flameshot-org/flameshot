#include "hottestwidget.h"
#include <QApplication>

//#define START_BACKGROUND

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HotTestWidget w;

#ifdef START_BACKGROUND
	auto startKey = new QHotkey(QKeySequence(Qt::MetaModifier | Qt::ControlModifier | Qt::Key_S), true, &w);
	QObject::connect(startKey, &QHotkey::activated,
					 &w, &QWidget::show);
#else
	w.show();
#endif

	return a.exec();
}
