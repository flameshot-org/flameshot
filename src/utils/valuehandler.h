#pragma once

#include "src/widgets/capture/capturetoolbutton.h"

#include <QColor>
#include <QList>
#include <QString>

class QVariant;

/**
 * Handles the value of a configuration option.
 *
 * Each configuration option should usually be handled in three different ways:
 * - have its value checked for errors (type, format, etc.)
 * - have its value (that was taken from the config file) adapted for proper use
 * - provided a fallback value in case: the config does not explicitly specify
 *   it, or the config contains an error and is globally falling back to
 *   defaults
 *
 * Subclass this class to handle custom value types.
 *
 * If you wish to handle simple value types (those supported by QVariant) you
 * should use `SimpleValueHandler`. Note that you can't use that class if the
 * value has some custom constraints on it. TODO
 *
 * @note You will only need to override `get` if you have to change the value
 * that was read from the config file. If you are fine with the value as long as
 * it is error-free, you don't have to override it.
 *
 * @note Keep in mind that you will probably want `check` to return `true` for
 * invalid QVariant's (option not found in config file).
 *
 */
class ValueHandler
{
public:
    virtual bool check(const QVariant& val) = 0;
    virtual QVariant value(const QVariant& val);
    virtual QVariant fallback();
    virtual QVariant representation(const QVariant& val);

protected:
    virtual QVariant process(const QVariant& val);
};

class Bool : public ValueHandler
{
public:
    Bool(bool def);
    bool check(const QVariant& val) override;
    QVariant fallback() override;

private:
    bool m_def;
};

class String : public ValueHandler
{
public:
    String(const QString& def);
    bool check(const QVariant&) override;
    QVariant fallback() override;

private:
    QString m_def;
};

class Color : public ValueHandler
{
public:
    Color(const QColor& def);
    bool check(const QVariant& val) override;
    QVariant fallback() override;
    QVariant representation(const QVariant& val) override;

private:
    QColor m_def;
};

class BoundedInt : public ValueHandler
{
public:
    BoundedInt(int min, int max, int def);

    bool check(const QVariant& val) override;
    virtual QVariant fallback() override;
    ;

private:
    int m_min, m_max, m_def;
};

class LowerBoundedInt : public ValueHandler
{
public:
    LowerBoundedInt(int min, int def);
    bool check(const QVariant& val) override;
    virtual QVariant fallback() override;

private:
    int m_min, m_def;
};

class KeySequence : public ValueHandler
{
public:
    KeySequence(const QString& shortcutName);
    bool check(const QVariant& val) override;
    QVariant fallback() override;

private:
    QString m_shortcutName;
};

class ExistingDir : public ValueHandler
{
    bool check(const QVariant& val) override;

    QVariant fallback() override;
};

class FilenamePattern : public ValueHandler
{
    bool check(const QVariant&) override;
    QVariant fallback() override;
};

class ButtonList : public ValueHandler
{
public:
    bool check(const QVariant& val) override;
    QVariant value(const QVariant& val) override;
    QVariant process(const QVariant& val) override;
    QVariant fallback() override;
    QVariant representation(const QVariant& val) override;

    // UTILITY FUNCTIONS
    static QList<CaptureToolButton::ButtonType> fromIntList(const QList<int>&);
    static QList<int> toIntList(const QList<CaptureToolButton::ButtonType>& l);
    static bool normalizeButtons(QList<int>& buttons);
};

class UserColors : public ValueHandler
{
    bool check(const QVariant& val) override;
    QVariant process(const QVariant& val) override;
    QVariant fallback() override;
};
