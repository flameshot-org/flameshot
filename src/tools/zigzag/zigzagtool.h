// zigzagtool.h
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "tools/abstracttwopointtool.h"
#include <QPoint>
#include <QVector>
#include <QPainterPath>

class ZigZagTool : public AbstractTwoPointTool
{
    Q_OBJECT
public:
    explicit ZigZagTool(QObject* parent = nullptr);
    ~ZigZagTool() override;

    QIcon icon(const QColor& background, bool inEditor = false) const override;
    QString name() const override;
    QString description() const override;
    CaptureTool::Type type() const override;
    CaptureTool* copy(QObject* parent) override;

    void pressed(CaptureContext& context) override;
    void drawStart(const CaptureContext& context) override;
    void drawMove(const QPoint& p) override;
    void drawEnd(const QPoint& p) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;

    static QVector<QPointF> generateZigzag(const QPointF& start, const QPointF& end, double wavelength, double amplitude);
    // make generateZigzagBalanced non-static so it can use m_compactness
    QVector<QPointF> generateZigzagBalanced(const QPointF& start, const QPointF& end, double wavelength, double amplitude);
    static QPainterPath pathFromPoints(const QVector<QPointF>& pts);

private:
    // user-configurable
    double m_userWavelength;
    double m_wavelength;
    double m_userAmplitude;
    double m_amplitude;
    double m_amplitudeMultiplier;
    int m_userSize;
    int m_userSizeBackup;

    // interaction state
    QPoint m_startMouse;
    double m_initialWavelength;
    double m_initialAmplitude;
    bool m_shiftAdjustActive;
    double m_shiftBaselineWavelength;
    double m_shiftBaselineAmplitude;

    // internal helpers / state
    double triangleAt(double t, double period, double phase) const;
    double m_phaseOffset;

    // compactness to make segments shorter / angles more acute
    double m_compactness; // 0.05 .. 1.0, lower => shorter arms & smaller amplitude
};
