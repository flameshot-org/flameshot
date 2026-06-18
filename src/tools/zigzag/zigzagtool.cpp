// zigzagtool.cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#include "zigzagtool.h"
#include <QPainter>
#include <QDebug>
#include <QGuiApplication>
#include <QPainterPath>
#include <cmath>
#include <algorithm>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

ZigZagTool::ZigZagTool(QObject* parent)
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
      m_compactness(0.6) // 0.05 .. 1.0, lower => shorter arms & smaller amplitude
{
    // Snap by default; Ctrl -> free drawing (invert modifier like LineTool)
    setModifierInvertsConstraint(true);
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

ZigZagTool::~ZigZagTool() = default;

QIcon ZigZagTool::icon(const QColor& background, bool /*inEditor*/) const
{
    return QIcon(iconPath(background) + "zigzag-outline.svg");
}

QString ZigZagTool::name() const { return tr("Zig Zag"); }
QString ZigZagTool::description() const { return tr("Set the ZigZag line as the paint tool"); }
CaptureTool::Type ZigZagTool::type() const { return CaptureTool::TYPE_ZIGZAG; }

CaptureTool* ZigZagTool::copy(QObject* parent)
{
    auto* tool = new ZigZagTool(parent);
    copyParams(this, tool);
    tool->m_userWavelength = this->m_userWavelength;
    tool->m_wavelength = this->m_wavelength;
    tool->m_userAmplitude = this->m_userAmplitude;
    tool->m_amplitude = this->m_amplitude;
    tool->m_amplitudeMultiplier = this->m_amplitudeMultiplier;
    tool->m_userSize = this->m_userSize;
    tool->m_compactness = this->m_compactness;
    tool->setModifierInvertsConstraint(this->modifierInvertsConstraint());
    return tool;
}

void ZigZagTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}

void ZigZagTool::drawStart(const CaptureContext& context)
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

void ZigZagTool::drawMove(const QPoint& p)
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

void ZigZagTool::drawEnd(const QPoint& p)
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

// so the triangle wave doesn't drift from scaling/devicePixelRatio, and ensure exact end alignment.

void ZigZagTool::process(QPainter& painter, const QPixmap& /*pixmap*/)
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

    // Compute logical wavelength (use logical coords for waveform phase)
    const double thicknessToWavelength = 3.0;
    double userWavelengthLogical = std::max(1.0, m_wavelength);
    double scaledWavelength = std::max(userWavelengthLogical, static_cast<double>(logicalSize) * thicknessToWavelength);
    double wavelengthLogical = std::clamp(scaledWavelength, 1.0, 10000.0);

    // amplitude: compute in logical coords and convert later for drawing offset
    const double baseFactor = 0.5;
    double derivedAmplitude = std::max(0.5, static_cast<double>(logicalSize) * baseFactor);
    double scaledDerived = derivedAmplitude * m_amplitudeMultiplier;
    const double userWeight = 0.4;
    const double derivedWeight = 0.6;
    double amplitudeBaseLogical = (userWeight * m_amplitude) + (derivedWeight * scaledDerived);
    amplitudeBaseLogical = std::clamp(amplitudeBaseLogical, 0.0, 10000.0);
    const double amplitudeForDrawing = amplitudeBaseLogical; // logical units

    QVector<QPointF> pts = generateZigzagBalanced(a, b, wavelengthLogical, amplitudeForDrawing);

    // ensure exact end alignment
    if (!pts.isEmpty()) pts.back() = b;

    QPainterPath path = pathFromPoints(pts);

    QPen pen(col, static_cast<double>(logicalSize), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    pen.setCosmetic(true);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(pen);
    painter.drawPath(path);
}

QVector<QPointF> ZigZagTool::generateZigzag(const QPointF& start, const QPointF& end, double wavelength, double amplitude)
{
    QVector<QPointF> pts;
    QPointF dir = end - start;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) { pts.append(start); pts.append(end); return pts; }
    QPointF unit(dir.x()/L, dir.y()/L);
    QPointF normal(-unit.y(), unit.x());

    int segments = std::max(1, static_cast<int>(std::ceil(L / wavelength)));
    double segLen = L / (segments * 1.0);
    pts.append(start);
    bool up = true;
    for (int i = 1; i <= segments; ++i) {
        double t = i * segLen;
        double amp = up ? amplitude : -amplitude;
        QPointF base = start + unit * t;
        pts.append(base + normal * amp);
        up = !up;
    }
    pts.append(end);
    return pts;
}

QVector<QPointF> ZigZagTool::generateZigzagBalanced(const QPointF& start, const QPointF& end, double wavelength, double amplitude)
{
    QVector<QPointF> pts;
    QPointF dir = end - start;
    double L = std::hypot(dir.x(), dir.y());
    if (L < 1e-6) { pts.append(start); pts.append(end); return pts; }
    QPointF unit(dir.x()/L, dir.y()/L);
    QPointF normal(-unit.y(), unit.x());

    // compactness factor (0.05..1.0): lower -> shorter arms and smaller amplitude
    const double compact = std::clamp(m_compactness, 0.05, 1.0);

    // desired arm length = half-wavelength but shortened by compactness
    double desiredArm = std::max(1.0, wavelength * 0.5 * compact);

    // choose number of arms so each arm is <= desiredArm
    int arms = std::max(1, static_cast<int>(std::ceil(L / desiredArm)));

    // make armLen exact so all arms equal
    double armLen = L / static_cast<double>(arms);

    // reduce amplitude slightly when compacted to make angles more acute
    double ampCompactFactor = 0.5 + 0.5 * compact; // in [0.5,1.0]
    double usedAmplitude = std::max(0.0, amplitude * ampCompactFactor);

    // Build points at positions i*armLen for i=0..arms.
    // Offsets: i==0 and i==arms -> baseline; odd i -> peak up; even i (not 0) -> peak down.
    pts.reserve(arms + 1);

    // Choose baseline parity so the interior points produce visible zigzag:
    // compute whether the first interior point should be up or down based on distance from start to the first half-period.
    // We keep integer-indexed zigzag appearance by default, but shift parity when that yields a clipped tip at the far end.
    // Determine parity so that the last interior point (i=arms-1) is opposite sign to the endpoint's desired baseline, producing a sharp tip.
    // Compute simple parity correction:
    bool parityShift = false;
    if (arms >= 2) {
        // If the number of arms is even, the naive alternating by index will make the last interior point
        // have the same sign as the previous peak, which can cause a clipped tip for large amplitude.
        // We flip parity when that would produce a "flat" end: choose parityShift so that the sequence of peaks
        // ends with the opposite sign relative to the previous, giving a sharp last tip.
        parityShift = ((arms % 2) == 0);
    }

    for (int i = 0; i <= arms; ++i) {
        double t = i * armLen;
        QPointF base = start + unit * t;
        if (i == 0 || i == arms) {
            pts.append(base);
        } else {
            // original behavior: odd i => up, even i => down
            bool up = (i % 2 == 1);
            if (parityShift) up = !up;
            double amp = up ? usedAmplitude : -usedAmplitude;
            pts.append(base + normal * amp);
        }
    }

    return pts;
}

QPainterPath ZigZagTool::pathFromPoints(const QVector<QPointF>& pts)
{
    QPainterPath path;
    if (pts.isEmpty()) return path;
    path.moveTo(pts.first());
    for (int i=1;i<pts.size();++i) path.lineTo(pts[i]);
    return path;
}
