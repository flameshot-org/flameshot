// ondulatedtool.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ondulatedtool.h"
#include <QPainter>
#include <QDebug>
#include <QGuiApplication>
#include <cmath>
#include <algorithm>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

OndulatedTool::OndulatedTool(QObject* parent)
    : AbstractTwoPointTool(parent),
      m_userWavelength(16.0),
      m_wavelength(16.0),
      m_userAmplitude(6.0),
      m_amplitude(6.0),
      m_amplitudeMultiplier(1.0),
      m_userSize(3),
      m_userSizeBackup(3),
      m_startMouse(0,0),
      m_initialWavelength(16.0),
      m_initialAmplitude(6.0),
      m_shiftAdjustActive(false),
      m_shiftBaselineWavelength(0.0),
      m_shiftBaselineAmplitude(0.0),
      m_phaseOffset(0.0),
      m_compactness(0.6)
{
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
    setModifierInvertsConstraint(true);
}

OndulatedTool::~OndulatedTool() = default;

QIcon OndulatedTool::icon(const QColor& background, bool /*inEditor*/) const
{
    return QIcon(iconPath(background) + "ondulated-outline.svg");
}

QString OndulatedTool::name() const { return tr("Ondulated"); }
QString OndulatedTool::description() const { return tr("Set the Ondulated line as the paint tool"); }
CaptureTool::Type OndulatedTool::type() const { return CaptureTool::TYPE_ONDULATED; }

CaptureTool* OndulatedTool::copy(QObject* parent)
{
    auto* tool = new OndulatedTool(parent);
    copyParams(this, tool);
    tool->m_userWavelength = this->m_userWavelength;
    tool->m_wavelength = this->m_wavelength;
    tool->m_userAmplitude = this->m_userAmplitude;
    tool->m_amplitude = this->m_amplitude;
    tool->m_amplitudeMultiplier = this->m_amplitudeMultiplier;
    tool->m_userSize = this->m_userSize;
    tool->m_compactness = this->m_compactness;
    tool->m_phaseOffset = this->m_phaseOffset;
    return tool;
}

void OndulatedTool::pressed(CaptureContext& context) { Q_UNUSED(context) }

void OndulatedTool::drawStart(const CaptureContext& context)
{
    onColorChanged(context.color);
    AbstractTwoPointTool::drawStart(context);
    onSizeChanged(context.toolSize);

    m_initialWavelength = m_userWavelength;
    m_wavelength = m_initialWavelength;
    m_initialAmplitude = m_userAmplitude;
    m_amplitude = m_initialAmplitude;

    m_userSize = context.toolSize;
    m_userSizeBackup = m_userSize;

#ifdef HAS_CAPTURECONTEXT_STARTPOS
    m_startMouse = context.startMousePos;
#else
    m_startMouse = points().first;
#endif

    m_shiftAdjustActive = false;
    m_shiftBaselineWavelength = 0.0;
    m_shiftBaselineAmplitude = 0.0;

    emit requestAction(CaptureTool::REQ_CLEAR_SELECTION);
}

void OndulatedTool::drawMove(const QPoint& p)
{
    Qt::KeyboardModifiers mods = QGuiApplication::queryKeyboardModifiers();
    const bool shift = (mods & Qt::ShiftModifier);

    if (!shift) {
        AbstractTwoPointTool::drawMove(p);
        if (m_shiftAdjustActive) {
            m_shiftAdjustActive = false;
            m_shiftBaselineWavelength = 0.0;
            m_shiftBaselineAmplitude = 0.0;
        }
    } else {
        if (!m_shiftAdjustActive) {
            m_shiftAdjustActive = true;
            m_shiftBaselineWavelength = (m_wavelength > 0.0) ? m_wavelength : m_initialWavelength;
            m_shiftBaselineAmplitude  = (m_amplitude  > 0.0) ? m_amplitude  : m_initialAmplitude;
        }

        double deltaX = static_cast<double>(p.x() - m_startMouse.x());
        double deltaY = static_cast<double>(p.y() - m_startMouse.y());

        const double horizSensitivity = 0.02;
        const double vertSensitivity  = 0.02;

        double wavelengthFactor = std::exp(deltaX * horizSensitivity);
        double newWavelength = std::clamp(m_shiftBaselineWavelength * wavelengthFactor, 1.0, 10000.0);

        double amplitudeFactor = std::exp((-deltaY) * vertSensitivity);
        double newAmplitude = std::clamp(m_shiftBaselineAmplitude * amplitudeFactor, 0.1, 10000.0);

        m_wavelength = newWavelength;
        m_amplitude = newAmplitude;
    }

    emit requestAction(CaptureTool::REQ_CLEAR_SELECTION);
}

void OndulatedTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p)
    m_userWavelength = m_wavelength;
    m_initialWavelength = m_wavelength;
    m_initialAmplitude = m_amplitude;
    m_userSize = m_userSizeBackup;

    m_shiftAdjustActive = false;
    m_shiftBaselineWavelength = 0.0;
    m_shiftBaselineAmplitude = 0.0;

    emit requestAction(CaptureTool::REQ_CAPTURE_DONE_OK);
}

void OndulatedTool::process(QPainter& painter, const QPixmap& /*pixmap*/)
{
    const QPointF a = QPointF(points().first);
    const QPointF b = QPointF(points().second);
    const int logicalSize = size();
    const QColor col = color();
    QPointF dir = b - a;
    double L_logical = std::hypot(dir.x(), dir.y());
    if (L_logical < 1e-6) {
        QPen pen(col, static_cast<double>(logicalSize));
        pen.setCosmetic(true);
        painter.setPen(pen);
        painter.drawLine(a, b);
        return;
    }
    QPointF unit(dir.x() / L_logical, dir.y() / L_logical);
    QPointF normal(-unit.y(), unit.x());
    const double dpr = painter.device()->devicePixelRatioF();

    const double compact = std::clamp(m_compactness, 0.05, 1.0);

    // compute logical wavelength biased by thickness
    const double thicknessToWavelength = 3.0;
    double userWavelengthLogical = std::max(1.0, m_wavelength);
    double scaledWavelength = std::max(userWavelengthLogical, static_cast<double>(logicalSize) * thicknessToWavelength);
    double wavelengthLogical = std::clamp(scaledWavelength, 1.0, 10000.0);
    double desiredWaveLogical = std::max(1.0, wavelengthLogical * compact);

    // convert to device
    double wavelengthDev = std::max(1.0, desiredWaveLogical * dpr);

    // amplitude: compute in logical coords then convert once to device
    const double baseFactor = 0.5;
    double derivedAmplitude = std::max(0.5, static_cast<double>(logicalSize) * baseFactor);
    double scaledDerived = derivedAmplitude * m_amplitudeMultiplier;
    const double userWeight = 0.4;
    const double derivedWeight = 0.6;
    double amplitudeBaseLogical = (userWeight * m_amplitude) + (derivedWeight * scaledDerived);
    amplitudeBaseLogical = std::clamp(amplitudeBaseLogical, 0.0, 10000.0);
    double ampCompactFactor = 0.5 + 0.5 * compact;
    double usedAmplitudeLogical = std::max(0.0, amplitudeBaseLogical * ampCompactFactor);
    double amplitudeDev = usedAmplitudeLogical * dpr;

    // adaptive sampling: more samples when amplitude is large to keep roundness
    double samplesPerWavelength = std::clamp(8.0 + (usedAmplitudeLogical / 5.0), 8.0, 64.0);

    double totalLenDev = L_logical * dpr;
    int steps = std::max(4, static_cast<int>(std::ceil(totalLenDev / (wavelengthDev / samplesPerWavelength))));
    steps = std::clamp(steps, 4, 5000);
    double stepDev = totalLenDev / steps;

    QVector<QPointF> pts;
    pts.reserve(steps + 1);
    for (int i = 0; i <= steps; ++i) {
        double tDev = i * stepDev;
        double tLogical = tDev / dpr;
        double phase = (2.0 * M_PI * tDev) / wavelengthDev + m_phaseOffset;
        double offsetDev = std::sin(phase) * amplitudeDev;
        QPointF base = a + unit * tLogical;
        QPointF p = base + normal * (offsetDev / dpr);
        pts.append(p);
    }

    QPainterPath path = pathFromPoints(pts);

    QPen pen(col, static_cast<double>(logicalSize), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    // use non-cosmetic pen so joins and caps render at device scale for better symmetry
    pen.setCosmetic(false);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(pen);
    painter.drawPath(path);
}

QVector<QPointF> OndulatedTool::generateOndulated(const QPointF& start, const QPointF& end, double wavelength, double amplitude)
{
    QVector<QPointF> pts;
    QPointF dir = end - start;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) { pts.append(start); pts.append(end); return pts; }
    QPointF unit(dir.x()/L, dir.y()/L);
    QPointF normal(-unit.y(), unit.x());
    int steps = std::max(4, static_cast<int>(std::ceil(L / (wavelength / 8.0))));
    double stepLen = L / steps;
    for (int i=0;i<=steps;++i){
        double t = i * stepLen;
        double phase = (2.0 * M_PI * t) / wavelength + m_phaseOffset;
        double offsetAmount = std::sin(phase) * amplitude;
        QPointF base = start + unit * t;
        pts.append(base + normal * offsetAmount);
    }
    return pts;
}

QVector<QPointF> OndulatedTool::generateOndulatedBalanced(const QPointF& start, const QPointF& end, double wavelength, double amplitude)
{
    QVector<QPointF> pts;
    QPointF dir = end - start;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) { pts.append(start); pts.append(end); return pts; }
    QPointF unit(dir.x()/L, dir.y()/L);
    QPointF normal(-unit.y(), unit.x());

    const double compact = std::clamp(m_compactness, 0.05, 1.0);

    const double thicknessToWavelength = 2.0;
    double userWavelengthLogical = std::max(1.0, m_wavelength);
    double scaledWavelength = std::max(userWavelengthLogical, static_cast<double>(m_userSize) * thicknessToWavelength);
    double wavelengthLogical = std::clamp(scaledWavelength, 1.0, 10000.0);
    double desiredWaveLogical = std::max(1.0, wavelengthLogical * compact);

    const double baseFactor = 0.5;
    double derivedAmplitude = std::max(0.5, static_cast<double>(m_userSize) * baseFactor);
    double scaledDerived = derivedAmplitude * m_amplitudeMultiplier;
    const double userWeight = 0.4;
    const double derivedWeight = 0.6;
    double amplitudeBaseLogical = (userWeight * m_amplitude) + (derivedWeight * scaledDerived);
    amplitudeBaseLogical = std::clamp(amplitudeBaseLogical, 0.0, 10000.0);
    double ampCompactFactor = 0.5 + 0.5 * compact;
    double usedAmplitudeLogical = std::max(0.0, amplitudeBaseLogical * ampCompactFactor);

    const double samplesPerWavelength = 8.0;
    int steps = std::max(4, static_cast<int>(std::ceil(L / (desiredWaveLogical / samplesPerWavelength))));
    steps = std::clamp(steps, 4, 2000);
    double stepLen = L / steps;

    pts.reserve(steps + 1);
    for (int i=0;i<=steps;++i){
        double t = i * stepLen;
        double phase = (2.0 * M_PI * t) / desiredWaveLogical + m_phaseOffset;
        double offsetAmount = std::sin(phase) * usedAmplitudeLogical;
        QPointF base = start + unit * t;
        pts.append(base + normal * offsetAmount);
    }

    return pts;
}

QPainterPath OndulatedTool::pathFromPoints(const QVector<QPointF>& pts)
{
    QPainterPath path;
    if (pts.isEmpty()) return path;
    path.moveTo(pts.first());
    for (int i=1;i<pts.size();++i) path.lineTo(pts[i]);
    return path;
}
