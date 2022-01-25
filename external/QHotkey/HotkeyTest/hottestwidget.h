#ifndef HOTTESTWIDGET_H
#define HOTTESTWIDGET_H

#include <QWidget>
#include <QHotkey>
#include <QThread>

namespace Ui {
	class HotTestWidget;
}

class HotTestWidget : public QWidget
{
	Q_OBJECT

public:
	explicit HotTestWidget(QWidget *parent = 0);
	~HotTestWidget();

private slots:
	void setShortcut_1(const QKeySequence &sequence);
	void setShortcut_2(const QKeySequence &sequence);
	void setShortcut_3(const QKeySequence &sequence);
	void setShortcut_4(const QKeySequence &sequence);
	void setShortcut_5(const QKeySequence &sequence);

	void increase_1();
	void increase_2();
	void increase_3();
	void increase_4();
	void increase_5();

	void on_resetButton_1_clicked();
	void on_resetButton_2_clicked();
	void on_resetButton_3_clicked();
	void on_resetButton_4_clicked();
	void on_resetButton_5_clicked();

	void on_groupBox_toggled(bool checked);
	void on_threadEnableCheckBox_clicked();

	void on_registeredCheckBox_toggled(bool checked);
	void increase_native();

private:
	Ui::HotTestWidget *ui;

	QHotkey *hotkey_1;
	QHotkey *hotkey_2;
	QHotkey *hotkey_3;
	QHotkey *hotkey_4;
	QHotkey *hotkey_5;

	QThread *thread4;
	QThread *thread5;

	QList<QHotkey*> testHotkeys;

	QHotkey *nativeHotkey;
};

#endif // HOTTESTWIDGET_H
