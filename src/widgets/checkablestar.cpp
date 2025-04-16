#include "checkablestar.h"
#include <QIcon>

CheckableStar::CheckableStar(QWidget* parent)
  : QToolButton(parent)
  , m_starred(false)
{
    setAutoRaise(true);
    setCursor(Qt::PointingHandCursor);
    updateIcon();

    connect(this, &QToolButton::clicked, this, &CheckableStar::toggleStar);
}

void CheckableStar::toggleStar()
{
    m_starred = !m_starred;
    updateIcon();
    emit starredChanged(m_starred);
}

void CheckableStar::updateIcon()
{
    QIcon icon = QIcon(m_starred ? ":/img/material/black/star-solid.svg"
                                 : ":/img/material/black/star-regular.svg");
    setIcon(icon);
}

void CheckableStar::setStarred(bool starred)
{
    if (m_starred != starred) {
        m_starred = starred;
        updateIcon();
    }
}

bool CheckableStar::isStarred() const
{
    return m_starred;
}
