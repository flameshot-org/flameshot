#include "geometryeditlayoutwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

GeometryEditLayoutWidget::GeometryEditLayoutWidget(QWidget* parent)
  : QGroupBox("Grab area")
{
    auto grid = new QGridLayout(this);
    auto wl = new QLabel("Width");
    m_wEditor = new QSpinBox;
    m_wEditor->setMaximum(3000);
    m_wEditor->setMinimum(1);
    m_wEditor->setValue(1280);

    auto hl = new QLabel("Height");
    m_hEditor = new QSpinBox;
    m_hEditor->setMaximum(3000);
    m_hEditor->setMinimum(1);
    m_hEditor->setValue(1280);

    auto xl = new QLabel("X");
    m_xEditor = new QSpinBox;
    m_xEditor->setMaximum(3000);
    m_xEditor->setMinimum(1);
    m_xEditor->setValue(1280);

    auto yl = new QLabel("Y");
    m_yEditor = new QSpinBox;
    m_yEditor->setMaximum(3000);
    m_yEditor->setMinimum(1);
    m_yEditor->setValue(1280);

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
