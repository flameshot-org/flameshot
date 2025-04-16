#ifndef CHECKABLESTAR_H
#define CHECKABLESTAR_H

#include <QToolButton>

class CheckableStar : public QToolButton
{
    Q_OBJECT

public:
    explicit CheckableStar(QWidget* parent = nullptr);

    void setStarred(bool starred);
    bool isStarred() const;

signals:
    void starredChanged(bool starred);

private slots:
    void toggleStar();

private:
    bool m_starred;
    void updateIcon();
};

#endif // CHECKABLESTAR_H
