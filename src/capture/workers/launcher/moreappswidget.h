#ifndef MOREAPPSWIDGET_H
#define MOREAPPSWIDGET_H

#include "src/utils/desktopfileparse.h"
#include <QWidget>

class QPixmap;
class QVBoxLayout;
class QTabWidget;

class MoreAppsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit MoreAppsWidget(const QPixmap &pixmap,
							const DesktopFileParser &parser,
							QWidget *parent = nullptr);

signals:
	void appClicked(const QModelIndex &index);
private:
	QPixmap m_pixmap;
	QVBoxLayout *m_layout;
	QTabWidget *m_tabs;

};

#endif // MOREAPPSWIDGET_H
