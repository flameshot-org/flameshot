#ifndef IMGUPLOADERTOOL_H
#define IMGUPLOADERTOOL_H

#include "src/tools/abstractactiontool.h"

class ImgUploaderTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit ImgUploaderTool(QObject* parent = nullptr);

    bool closeOnButtonPressed() const override;

    QString name() const override;
    QIcon icon(const QColor& background, bool inEditor) const override;

    void setCapture(const QPixmap& pixmap) override;
    const QPixmap& capture();

public slots:
    void pressed(const CaptureContext& context) override;

protected:
    ToolType nameID() const override;

private:
    QPixmap m_capture;
};

#endif // IMGUPLOADERTOOL_H
