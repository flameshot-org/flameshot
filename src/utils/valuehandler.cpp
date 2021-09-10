#include "valuehandler.h"
#include "confighandler.h"
#include <QColor>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVariant>

// VALUE HANDLER

QVariant ValueHandler::value(const QVariant& val)
{
    if (!val.isValid() || !check(val)) {
        return fallback();
    } else {
        return process(val);
    }
}

QVariant ValueHandler::fallback()
{
    return QVariant();
}

QVariant ValueHandler::representation(const QVariant& val)
{
    return val.toString();
}

QVariant ValueHandler::process(const QVariant& val)
{
    return val;
}

// BOOL

Bool::Bool(bool def)
  : m_def(def)
{}

bool Bool::check(const QVariant& val)
{
    QString str = val.toString();
    if (str != "true" && str != "false") {
        return false;
    }
    return true;
}

QVariant Bool::fallback()
{
    return m_def;
}

// STRING

String::String(const QString& def)
  : m_def(def)
{}

bool String::check(const QVariant&)
{
    return true;
}

QVariant String::fallback()
{
    return m_def;
}

// COLOR

Color::Color(const QColor& def)
  : m_def(def)
{}

bool Color::check(const QVariant& val)
{
    return QColor::isValidColor(val.toString());
}

QVariant Color::fallback()
{
    return m_def;
}

QVariant Color::representation(const QVariant& val)
{
    return QString(val.value<QColor>().name());
}

// BOUNDED INT

BoundedInt::BoundedInt(int min, int max, int def)
  : m_min(min)
  , m_max(max)
  , m_def(def)
{}

bool BoundedInt::check(const QVariant& val)
{
    QString str = val.toString();
    bool conversionOk;
    int num = str.toInt(&conversionOk);
    return conversionOk && (m_max < m_min || num <= m_max);
}

QVariant BoundedInt::fallback()
{
    return m_def;
}

// LOWER BOUNDED INT

LowerBoundedInt::LowerBoundedInt(int min, int def)
  : m_min(min)
  , m_def(def)
{}

bool LowerBoundedInt::check(const QVariant& val)
{
    QString str = val.toString();
    bool conversionOk;
    int num = str.toInt(&conversionOk);
    return conversionOk && num >= m_min;
}

QVariant LowerBoundedInt::fallback()
{
    return m_def;
}

// KEY SEQUENCE

KeySequence::KeySequence(const QKeySequence& fallback)
  : m_fallback(fallback)
{}

bool KeySequence::check(const QVariant& val)
{
    QString str = val.toString();
    if (!str.isEmpty() && QKeySequence(str).toString().isEmpty()) {
        return false;
    }
    return true;
}

QVariant KeySequence::fallback()
{
    return m_fallback;
}

// EXISTING DIR

bool ExistingDir::check(const QVariant& val)
{
    if (!val.canConvert(QVariant::String) || val.toString().isEmpty()) {
        return false;
    }
    QFileInfo info(val.toString());
    return info.isDir() && info.exists();
}

QVariant ExistingDir::fallback()
{
    using SP = QStandardPaths;
    for (auto location :
         { SP::PicturesLocation, SP::HomeLocation, SP::TempLocation }) {
        QString path = SP::writableLocation(location);
        if (QFileInfo(path).isDir()) {
            return path;
        }
    }
    return {};
}

// FILENAME PATTERN

bool FilenamePattern::check(const QVariant&)
{
    return true;
}

QVariant FilenamePattern::fallback()
{
    return ConfigHandler().filenamePatternDefault();
}

QVariant FilenamePattern::process(const QVariant& val)
{
    QString str = val.toString();
    return !str.isEmpty() ? val : fallback();
}

// BUTTON LIST

using BType = CaptureToolButton::ButtonType;
using BList = QList<BType>;

bool ButtonList::check(const QVariant& val)
{
    using CTB = CaptureToolButton;
    auto allButtons = CTB::getIterableButtonTypes();
    for (int btn : val.value<QList<int>>()) {
        if (!allButtons.contains(static_cast<CTB::ButtonType>(btn))) {
            return false;
        }
    }
    return true;
}

// Helper
void sortButtons(BList& buttons)
{
    std::sort(buttons.begin(), buttons.end(), [](BType a, BType b) {
        return CaptureToolButton::getPriorityByButton(a) <
               CaptureToolButton::getPriorityByButton(b);
    });
}

QVariant ButtonList::process(const QVariant& val)
{
    QList<int> intButtons = val.value<QList<int>>();
    auto buttons = ButtonList::fromIntList(intButtons);
    sortButtons(buttons);
    return QVariant::fromValue(buttons);
}

QVariant ButtonList::fallback()
{
    auto buttons = CaptureToolButton::getIterableButtonTypes();
    buttons.removeOne(CaptureToolButton::TYPE_SIZEDECREASE);
    buttons.removeOne(CaptureToolButton::TYPE_SIZEINCREASE);
    sortButtons(buttons);
    return QVariant::fromValue(buttons);
}

QVariant ButtonList::representation(const QVariant& val)
{
    auto intList = toIntList(val.value<BList>());
    normalizeButtons(intList);
    return QVariant::fromValue(intList);
}

QList<CaptureToolButton::ButtonType> ButtonList::fromIntList(
  const QList<int>& l)
{
    QList<CaptureToolButton::ButtonType> buttons;
    buttons.reserve(l.size());
    for (auto const i : l)
        buttons << static_cast<CaptureToolButton::ButtonType>(i);
    return buttons;
}

QList<int> ButtonList::toIntList(const QList<CaptureToolButton::ButtonType>& l)
{
    QList<int> buttons;
    buttons.reserve(l.size());
    for (auto const i : l)
        buttons << static_cast<int>(i);
    return buttons;
}

bool ButtonList::normalizeButtons(QList<int>& buttons)
{
    QList<int> listTypesInt =
      toIntList(CaptureToolButton::getIterableButtonTypes());

    bool hasChanged = false;
    for (int i = 0; i < buttons.size(); i++) {
        if (!listTypesInt.contains(buttons.at(i))) {
            buttons.removeAt(i);
            hasChanged = true;
        }
    }
    return hasChanged;
}

// USER COLORS

bool UserColors::check(const QVariant& val)
{
    if (!val.isValid()) {
        return true;
    }
    if (!val.canConvert(QVariant::StringList)) {
        return false;
    }
    for (const QString& str : val.toStringList()) {
        if (!QColor::isValidColor(str)) {
            return false;
        }
    }
    return true;
}

QVariant UserColors::process(const QVariant& val)
{
    QStringList strColors = val.toStringList();
    if (strColors.isEmpty()) {
        return fallback();
    }

    QVector<QColor> colors;
    colors.reserve(strColors.size());

    for (const QString& str : strColors) {
        colors.append(QColor(str));
    }

    return QVariant::fromValue(colors);
}

QVariant UserColors::fallback()
{
    return QVariant::fromValue(QVector<QColor>{ Qt::darkRed,
                                                Qt::red,
                                                Qt::yellow,
                                                Qt::green,
                                                Qt::darkGreen,
                                                Qt::cyan,
                                                Qt::blue,
                                                Qt::magenta,
                                                Qt::darkMagenta,
                                                QColor() });
}
