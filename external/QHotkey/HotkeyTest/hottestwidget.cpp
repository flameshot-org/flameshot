#include "hottestwidget.h"
#include "ui_hottestwidget.h"

//#define TEST_MAPPING

HotTestWidget::HotTestWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HotTestWidget),
	hotkey_1(new QHotkey(this)),
	hotkey_2(new QHotkey(this)),
	hotkey_3(new QHotkey(this)),
	hotkey_4(new QHotkey(NULL)),
	hotkey_5(new QHotkey(NULL)),
	thread4(new QThread(this)),
	thread5(new QThread(this)),
	testHotkeys(),
	nativeHotkey(new QHotkey(this))
{
	ui->setupUi(this);
	this->thread4->start();
	this->thread5->start();

#ifdef TEST_MAPPING
	//shortcut mapping override
	QHotkey::addGlobalMapping(QKeySequence("X"), QHotkey::NativeShortcut());// add invalid mapping to test if the overwrite works for all platforms
#endif

	//1
	connect(this->ui->hotkeyCheckbox_1, &QCheckBox::toggled,
			this->hotkey_1, &QHotkey::setRegistered);
	connect(this->ui->hotkeySequenceEdit_1, &QKeySequenceEdit::keySequenceChanged,
			this, &HotTestWidget::setShortcut_1);
	connect(this->hotkey_1, &QHotkey::activated,
			this, &HotTestWidget::increase_1);

	//2
	connect(this->ui->hotkeyCheckbox_2, &QCheckBox::toggled,
			this->hotkey_2, &QHotkey::setRegistered);
	connect(this->ui->hotkeySequenceEdit_2, &QKeySequenceEdit::keySequenceChanged,
			this, &HotTestWidget::setShortcut_2);
	connect(this->hotkey_2, &QHotkey::activated,
			this, &HotTestWidget::increase_2);

	//3
	connect(this->ui->hotkeyCheckbox_3, &QCheckBox::toggled,
			this->hotkey_3, &QHotkey::setRegistered);
	connect(this->ui->hotkeySequenceEdit_3, &QKeySequenceEdit::keySequenceChanged,
			this, &HotTestWidget::setShortcut_3);
	connect(this->hotkey_3, &QHotkey::activated,
			this, &HotTestWidget::increase_3);

	//4
	connect(this->ui->hotkeyCheckbox_4, &QCheckBox::toggled,
			this->hotkey_4, &QHotkey::setRegistered);
	connect(this->ui->hotkeySequenceEdit_4, &QKeySequenceEdit::keySequenceChanged,
			this, &HotTestWidget::setShortcut_4);
	connect(this->hotkey_4, &QHotkey::activated,
			this, &HotTestWidget::increase_4);

	//5
	connect(this->ui->hotkeyCheckbox_5, &QCheckBox::toggled,
			this->hotkey_5, &QHotkey::setRegistered);
	connect(this->ui->hotkeySequenceEdit_5, &QKeySequenceEdit::keySequenceChanged,
			this, &HotTestWidget::setShortcut_5);
	connect(this->hotkey_5, &QHotkey::activated,
			this, &HotTestWidget::increase_5);

	//test connections
	this->testHotkeys += new QHotkey(Qt::Key_F, Qt::NoModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyFCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_F12, Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyCtrlAltMetaF12CheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_Cancel, Qt::ControlModifier | Qt::ShiftModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyCtrlShiftCancelCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_Delete, Qt::MetaModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyMetaDelCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_NumLock, Qt::NoModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyNumlockCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_5, Qt::ControlModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyCtrl5CheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_Tab, Qt::ShiftModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyShiftTabCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_Comma, Qt::ShiftModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyShiftCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_Semicolon, Qt::ShiftModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyShiftCheckBox_2, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_K, Qt::ShiftModifier | Qt::AltModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyShiftAltKCheckBox, &QCheckBox::toggle);
	this->testHotkeys += new QHotkey(Qt::Key_K, Qt::ShiftModifier | Qt::AltModifier, false, this);
	connect(this->testHotkeys.last(), &QHotkey::activated,
			this->ui->hotkeyShiftAltKCheckBox_2, &QCheckBox::toggle);

	//native
	connect(this->nativeHotkey, &QHotkey::activated,
			this, &HotTestWidget::increase_native);
}

HotTestWidget::~HotTestWidget()
{
	this->thread4->quit();
	this->thread4->wait();
	this->thread5->quit();
	this->thread5->wait();

	delete this->hotkey_4;
	delete this->hotkey_5;

	delete ui;
}

void HotTestWidget::setShortcut_1(const QKeySequence &sequence)
{
	this->hotkey_1->setShortcut(sequence, false);
}

void HotTestWidget::setShortcut_2(const QKeySequence &sequence)
{
	this->hotkey_2->setShortcut(sequence, false);
}

void HotTestWidget::setShortcut_3(const QKeySequence &sequence)
{
	this->hotkey_3->setShortcut(sequence, false);
}

void HotTestWidget::setShortcut_4(const QKeySequence &sequence)
{
	this->hotkey_4->setShortcut(sequence, false);
}

void HotTestWidget::setShortcut_5(const QKeySequence &sequence)
{
	this->hotkey_5->setShortcut(sequence, false);
}

void HotTestWidget::increase_1()
{
	this->ui->hotkeyCount_1->display(this->ui->hotkeyCount_1->intValue() + 1);
}

void HotTestWidget::increase_2()
{
	this->ui->hotkeyCount_2->display(this->ui->hotkeyCount_2->intValue() + 1);
}

void HotTestWidget::increase_3()
{
	this->ui->hotkeyCount_3->display(this->ui->hotkeyCount_3->intValue() + 1);
}

void HotTestWidget::increase_4()
{
	this->ui->hotkeyCount_4->display(this->ui->hotkeyCount_4->intValue() + 1);
}

void HotTestWidget::increase_5()
{
	this->ui->hotkeyCount_5->display(this->ui->hotkeyCount_5->intValue() + 1);
}

void HotTestWidget::on_resetButton_1_clicked()
{
	this->ui->hotkeyCount_1->display(0);
}

void HotTestWidget::on_resetButton_2_clicked()
{
	this->ui->hotkeyCount_2->display(0);
}

void HotTestWidget::on_resetButton_3_clicked()
{
	this->ui->hotkeyCount_3->display(0);
}

void HotTestWidget::on_resetButton_4_clicked()
{
	this->ui->hotkeyCount_4->display(0);
}

void HotTestWidget::on_resetButton_5_clicked()
{
	this->ui->hotkeyCount_5->display(0);
}

void HotTestWidget::on_groupBox_toggled(bool checked)
{
	for(QHotkey *hotkey : this->testHotkeys)
		hotkey->setRegistered(checked);
}

void HotTestWidget::on_threadEnableCheckBox_clicked()
{
	this->ui->threadEnableCheckBox->setEnabled(false);
	this->ui->hotkeyCheckbox_1->setChecked(false);
	this->ui->hotkeyCheckbox_2->setChecked(false);
	this->ui->hotkeyCheckbox_3->setChecked(false);
	this->ui->hotkeyCheckbox_4->setChecked(false);
	this->ui->hotkeyCheckbox_5->setChecked(false);

	QApplication::processEvents();

	Q_ASSERT(!this->hotkey_4->isRegistered());
	Q_ASSERT(!this->hotkey_5->isRegistered());

	this->hotkey_4->moveToThread(this->thread4);
	this->hotkey_5->moveToThread(this->thread5);

	QApplication::processEvents();
	Q_ASSERT(this->hotkey_4->thread() == this->thread4);
	Q_ASSERT(this->hotkey_5->thread() == this->thread5);

	connect(this->thread4, &QThread::finished, this, [this](){
		this->hotkey_4->moveToThread(qApp->thread());
	});
	connect(this->thread5, &QThread::finished, this, [this](){
		this->hotkey_5->moveToThread(qApp->thread());
	});

	this->ui->tabWidget->setCurrentIndex(0);
}

void HotTestWidget::on_registeredCheckBox_toggled(bool checked)
{
	if(checked) {
		this->nativeHotkey->setNativeShortcut({
												  (quint32)this->ui->nativeKeySpinBox->value(),
												  (quint32)this->ui->nativeModifiersSpinBox->value()
											  }, true);
	} else
		this->nativeHotkey->setRegistered(false);
}

void HotTestWidget::increase_native()
{
	this->ui->nativeCount->display(this->ui->nativeCount->intValue() + 1);
}
