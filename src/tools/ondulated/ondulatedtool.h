// ondulatedtool.h
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "tools/abstracttwopointtool.h"
#include <QPoint>
#include <QIcon>
#include <QVector>
#include <QPointF>
#include <QPainterPath>

class OndulatedTool : public AbstractTwoPointTool {
    Q_OBJECT
public:
    explicit OndulatedTool(QObject* parent = nullptr);
    ~OndulatedTool() override;

    QIcon icon(const QColor& background, bool inEditor) const override;
    QString name() const override;
    QString description() const override;
    CaptureTool::Type type() const override;
    CaptureTool* copy(QObject* parent) override;

    void pressed(CaptureContext& context) override;
    void drawStart(const CaptureContext& context) override;
    void drawMove(const QPoint& p) override;
    void drawEnd(const QPoint& p) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;

private:
    QVector<QPointF> generateOndulated(const QPointF& start, const QPointF& end, double wavelength, double amplitude);
    QVector<QPointF> generateOndulatedBalanced(const QPointF& start, const QPointF& end, double wavelength, double amplitude);
    QPainterPath pathFromPoints(const QVector<QPointF>& pts);

    double m_userWavelength;
    double m_userAmplitude;
    int    m_userSize;

    double m_wavelength;
    double m_amplitude;
    int    m_userSizeBackup;

    QPoint m_startMouse;
    double m_initialWavelength;
    double m_initialAmplitude;
    bool   m_shiftAdjustActive;
    double m_shiftBaselineWavelength;
    double m_shiftBaselineAmplitude;

    double m_amplitudeMultiplier;
    double m_compactness;

    // ensure phase offset exists and is used for symmetry
    double m_phaseOffset;
};
