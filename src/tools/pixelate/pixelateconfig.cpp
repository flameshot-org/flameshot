#include "pixelateconfig.h"

PixelateConfig::PixelateConfig(QWidget* parent)
  : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_invertSelectionCheckBox = new QCheckBox(tr("invert selection"), this);
    m_layout->addWidget(m_invertSelectionCheckBox);
    connect(m_invertSelectionCheckBox,
            &QCheckBox::toggled,
            this,
            &PixelateConfig::toggleInvertSelection);
}

void PixelateConfig::setInvertSelection(bool invert)
{
    m_invertSelectionCheckBox->setTristate(invert);
}
