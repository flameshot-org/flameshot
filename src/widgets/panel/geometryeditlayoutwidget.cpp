#include "geometryeditlayoutwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

int const MIN_POSITION = 0;
int const MAX_POSITION = 100000;

int const MIN_SIZE = 1;
int const MAX_SIZE = 100000;

GeometryEditLayoutWidget::GeometryEditLayoutWidget(QWidget* parent)
  : QGroupBox("Grab area")
{
    auto grid = new QGridLayout(this);
    auto wl = new QLabel("Width");
    m_wEditor = new QSpinBox;
    m_wEditor->setMaximum(MAX_SIZE);
    m_wEditor->setMinimum(MIN_SIZE);
    m_wEditor->setValue(MIN_SIZE);

    auto hl = new QLabel("Height");
    m_hEditor = new QSpinBox;
    m_hEditor->setMaximum(MAX_SIZE);
    m_hEditor->setMinimum(MIN_SIZE);
    m_hEditor->setValue(MIN_SIZE);

    auto xl = new QLabel("X");
    m_xEditor = new QSpinBox;
    m_xEditor->setMaximum(MAX_POSITION);
    m_xEditor->setMinimum(MIN_POSITION);
    m_xEditor->setValue(MIN_POSITION);

    auto yl = new QLabel("Y");
    m_yEditor = new QSpinBox;
    m_yEditor->setMaximum(MAX_POSITION);
    m_yEditor->setMinimum(MIN_POSITION);
    m_yEditor->setValue(MIN_POSITION);

    grid->addWidget(wl, 0, 0);
    grid->addWidget(m_wEditor, 0, 1);

    grid->addWidget(hl, 1, 0);
    grid->addWidget(m_hEditor, 1, 1);

    grid->addWidget(xl, 2, 0);
    grid->addWidget(m_xEditor, 2, 1);

    grid->addWidget(yl, 3, 0);
    grid->addWidget(m_yEditor, 3, 1);

    setLayout(grid);

    auto updateCallback = [this]() {
        emit edited({ m_xEditor->value(),
                      m_yEditor->value(),
                      m_wEditor->value(),
                      m_hEditor->value() });
    };
    connect(m_wEditor, &QSpinBox::editingFinished, this, updateCallback);
    connect(m_hEditor, &QSpinBox::editingFinished, this, updateCallback);
    connect(m_xEditor, &QSpinBox::editingFinished, this, updateCallback);
    connect(m_yEditor, &QSpinBox::editingFinished, this, updateCallback);
}

void GeometryEditLayoutWidget::update(QRect const& g)
{
    m_wEditor->setValue(g.width());
    m_hEditor->setValue(g.height());
    m_xEditor->setValue(g.x());
    m_yEditor->setValue(g.y());
}
