#ifndef CONFIGENTERPRISE_H
#define CONFIGENTERPRISE_H

class QSettings;


class ConfigEnterprise
{
public:
    ConfigEnterprise();

    QSettings *settings();

private:
    QSettings *m_settings;
};

#endif // CONFIGENTERPRISE_H
