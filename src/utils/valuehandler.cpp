#include "valuehandler.h"
#include "capturetool.h"
#include "colorpickerwidget.h"
#include "confighandler.h"
#include "screengrabber.h"
#include <QColor>
#include <QFileInfo>
#include <QImageWriter>
#include <QKeySequence>
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
    return {};
}

QVariant ValueHandler::representation(const QVariant& val)
{
    return val.toString();
}

QString ValueHandler::expected()
{
    return {};
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

QString Bool::expected()
{
    return QStringLiteral("true or false");
}

// STRING

String::String(QString def)
  : m_def(std::move(def))
{}

bool String::check(const QVariant&)
{
    return true;
}

QVariant String::fallback()
{
    return m_def;
}

QString String::expected()
{
    return QStringLiteral("string");
}

// COLOR

Color::Color(QColor def)
  : m_def(std::move(def))
{}

bool Color::check(const QVariant& val)
{
    QString str = val.toString();
    // Disable #RGB, #RRRGGGBBB and #RRRRGGGGBBBB formats that QColor supports
    return QColor::isValidColor(str) &&
           (str[0] != '#' ||
            (str.length() != 4 && str.length() != 10 && str.length() != 13));
}

QVariant Color::process(const QVariant& val)
{
    QString str = val.toString();
    QColor color(str);
    if (str.length() == 9 && str[0] == '#') {
        // Convert #RRGGBBAA (flameshot) to #AARRGGBB (QColor)
        int blue = color.blue();
        color.setBlue(color.green());
        color.setGreen(color.red());
        color.setRed(color.alpha());
        color.setAlpha(blue);
    }
    return color;
}

QVariant Color::fallback()
{
    return m_def;
}

QVariant Color::representation(const QVariant& val)
{
    QString str = val.toString();
    QColor color(str);
    if (str.length() == 9 && str[0] == '#') {
        // Convert #AARRGGBB (QColor) to #RRGGBBAA (flameshot)
        int alpha = color.alpha();
        color.setAlpha(color.red());
        color.setRed(color.green());
        color.setGreen(color.blue());
        color.setBlue(alpha);
    }
    return color.name();
}

QString Color::expected()
{
    return QStringLiteral("color name or hex value");
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
    return conversionOk && m_min <= num && num <= m_max;
}

QVariant BoundedInt::fallback()
{
    return m_def;
}

QString BoundedInt::expected()
{
    return QStringLiteral("number between %1 and %2").arg(m_min).arg(m_max);
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

QString LowerBoundedInt::expected()
{
    return QStringLiteral("number >= %1").arg(m_min);
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

QString KeySequence::expected()
{
    return QStringLiteral("keyboard shortcut");
}

QVariant KeySequence::representation(const QVariant& val)
{
    QString str(val.toString());
    if (QKeySequence(str) == QKeySequence(Qt::Key_Return)) {
        return QStringLiteral("Enter");
    }
    return str;
}

QVariant KeySequence::process(const QVariant& val)
{
    QString str(val.toString());
    if (str == "Enter") {
        return QKeySequence(Qt::Key_Return).toString();
    }
    return str;
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

QString ExistingDir::expected()
{
    return QStringLiteral("existing directory");
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

QString FilenamePattern::expected()
{
    return QStringLiteral("please edit using the GUI");
}

// BUTTON LIST

using BType = CaptureTool::Type;
using BList = QList<CaptureTool::Type>;

bool ButtonList::check(const QVariant& val)
{
    // TODO stop using CTB
    using CTB = CaptureToolButton;
    auto allButtons = CTB::getIterableButtonTypes();
    for (int btn : val.value<QList<int>>()) {
        if (!allButtons.contains(static_cast<BType>(btn))) {
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
    auto intButtons = val.value<QList<int>>();
    auto buttons = ButtonList::fromIntList(intButtons);
    sortButtons(buttons);
    return QVariant::fromValue(buttons);
}

QVariant ButtonList::fallback()
{
    auto buttons = CaptureToolButton::getIterableButtonTypes();
    buttons.removeOne(CaptureTool::TYPE_SIZEDECREASE);
    buttons.removeOne(CaptureTool::TYPE_SIZEINCREASE);
    sortButtons(buttons);
    return QVariant::fromValue(buttons);
}

QVariant ButtonList::representation(const QVariant& val)
{
    auto intList = toIntList(val.value<BList>());
    normalizeButtons(intList);
    return QVariant::fromValue(intList);
}

QString ButtonList::expected()
{
    return QStringLiteral("please don't edit by hand");
}

QList<CaptureTool::Type> ButtonList::fromIntList(const QList<int>& l)
{
    QList<CaptureTool::Type> buttons;
    buttons.reserve(l.size());
    for (auto const i : l) {
        buttons << static_cast<CaptureTool::Type>(i);
    }
    return buttons;
}

QList<int> ButtonList::toIntList(const QList<CaptureTool::Type>& l)
{
    QList<int> buttons;
    buttons.reserve(l.size());
    for (auto const i : l) {
        buttons << static_cast<int>(i);
    }
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

UserColors::UserColors(int min, int max)
  : m_min(min)
  , m_max(max)
{}

bool UserColors::check(const QVariant& val)
{
    if (!val.isValid()) {
        return false;
    }
    if (!val.canConvert(QVariant::StringList)) {
        return false;
    }
    for (const QString& str : val.toStringList()) {
        if (!QColor::isValidColor(str) && str != "picker") {
            return false;
        }
    }

    int sz = val.toStringList().size();

    return sz >= m_min && sz <= m_max;
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
        if (str != "picker") {
            colors.append(QColor(str));
        } else {
            colors.append(QColor());
        }
    }

    return QVariant::fromValue(colors);
}

QVariant UserColors::fallback()
{
    if (ConfigHandler().predefinedColorPaletteLarge()) {
        return QVariant::fromValue(
          ColorPickerWidget::getDefaultLargeColorPalette());
    } else {
        return QVariant::fromValue(
          ColorPickerWidget::getDefaultSmallColorPalette());
    }
}

QString UserColors::expected()
{
    return QStringLiteral(
             "list of colors(min %1 and max %2) separated by comma")
      .arg(m_min - 1)
      .arg(m_max - 1);
}

QVariant UserColors::representation(const QVariant& val)
{
    auto colors = val.value<QVector<QColor>>();

    QStringList strColors;

    for (const auto& col : colors) {
        if (col.isValid()) {
            strColors.append(col.name(QColor::HexRgb));
        } else {
            strColors.append(QStringLiteral("picker"));
        }
    }

    return QVariant::fromValue(strColors);
}

// SET SAVE FILE AS EXTENSION

bool SaveFileExtension::check(const QVariant& val)
{
    if (!val.canConvert(QVariant::String) || val.toString().isEmpty()) {
        return false;
    }

    QString extension = val.toString();

    if (extension.startsWith(".")) {
        extension.remove(0, 1);
    }

    QStringList imageFormatList;
    foreach (auto imageFormat, QImageWriter::supportedImageFormats())
        imageFormatList.append(imageFormat);

    if (!imageFormatList.contains(extension)) {
        return false;
    }

    return true;
}

QVariant SaveFileExtension::process(const QVariant& val)
{
    QString extension = val.toString();

    if (extension.startsWith(".")) {
        extension.remove(0, 1);
    }

    return QVariant::fromValue(extension);
}

QString SaveFileExtension::expected()
{
    return QStringLiteral("supported image extension");
}

// REGION

bool Region::check(const QVariant& val)
{
    QVariant region = process(val);
    return process(val).isValid();
}

#include <QApplication> // TODO remove after FIXME (see below)
#include <utility>

QVariant Region::process(const QVariant& val)
{
    // FIXME: This is temporary, just before D-Bus is removed
    char** argv = new char*[1];
    int* argc = new int{ 0 };
    if (QGuiApplication::screens().empty()) {
        new QApplication(*argc, argv);
    }

    QString str = val.toString();

    if (str == "all") {
        return ScreenGrabber().desktopGeometry();
    } else if (str.startsWith("screen")) {
        bool ok;
        int number = str.midRef(6).toInt(&ok);
        if (!ok || number < 0) {
            return {};
        }
        return ScreenGrabber().screenGeometry(qApp->screens()[number]);
    }

    QRegExp regex("(-{,1}\\d+)"   // number (any sign)
                  "[x,\\.\\s]"    // separator ('x', ',', '.', or whitespace)
                  "(-{,1}\\d+)"   // number (any sign)
                  "[\\+,\\.\\s]*" // separator ('+',',', '.', or whitespace)
                  "(-{,1}\\d+)"   // number (non-negative)
                  "[\\+,\\.\\s]*" // separator ('+', ',', '.', or whitespace)
                  "(-{,1}\\d+)"   // number (non-negative)
    );

    if (!regex.exactMatch(str)) {
        return {};
    }

    int w, h, x, y;
    bool w_ok, h_ok, x_ok, y_ok;
    w = regex.cap(1).toInt(&w_ok);
    h = regex.cap(2).toInt(&h_ok);
    x = regex.cap(3).toInt(&x_ok);
    y = regex.cap(4).toInt(&y_ok);

    if (!(w_ok && h_ok && x_ok && y_ok)) {
        return {};
    }

    return QRect(x, y, w, h).normalized();
}
