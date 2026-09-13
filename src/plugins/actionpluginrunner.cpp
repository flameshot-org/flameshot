// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionpluginrunner.h"

#include "core/flameshotdaemon.h"
#include "utils/systemnotification.h"

#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmap>
#include <QProcessEnvironment>
#include <QStandardPaths>

namespace {
constexpr qsizetype MaxStandardOutputSize = 1024 * 1024;
constexpr qsizetype MaxStandardErrorSize = 256 * 1024;
}

ActionPluginRunner* ActionPluginRunner::run(const ActionPlugin& plugin,
                                            const QPixmap& capture)
{
    auto* runner = new ActionPluginRunner(plugin, capture);
    QTimer::singleShot(0, runner, [runner]() { runner->start(); });
    return runner;
}

ActionPluginRunner::ActionPluginRunner(const ActionPlugin& plugin,
                                       const QPixmap& capture)
  : QObject(qApp)
  , m_plugin(plugin)
{
    QBuffer buffer(&m_png);
    if (!buffer.open(QIODevice::WriteOnly) || !capture.save(&buffer, "PNG")) {
        m_png.clear();
    }

    m_timeout.setSingleShot(true);
    connect(
      &m_timeout, &QTimer::timeout, this, &ActionPluginRunner::processTimedOut);
    connect(&m_process, &QProcess::started, this, [this]() {
        m_process.write(m_png);
        m_process.closeWriteChannel();
    });
    connect(&m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            &ActionPluginRunner::processFinished);
    connect(&m_process,
            &QProcess::errorOccurred,
            this,
            [this](QProcess::ProcessError) { processFailed(); });
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        collectOutput();
    });
    connect(&m_process, &QProcess::readyReadStandardError, this, [this]() {
        collectOutput();
    });
}

void ActionPluginRunner::start()
{
    if (m_png.isEmpty()) {
        finishWithError(tr("Could not encode the capture as PNG."));
        return;
    }

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.remove("LD_PRELOAD");
    environment.insert("FLAMESHOT_PLUGIN_API_VERSION",
                       QString::number(ActionPluginLoader::ApiVersion));
    environment.insert("FLAMESHOT_PLUGIN_ID", m_plugin.id);
    environment.insert("FLAMESHOT_PLUGIN_DIR",
                       QFileInfo(m_plugin.manifestPath).absolutePath());
    const QString configDirectory =
      QDir(
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation))
        .filePath(QStringLiteral("flameshot/plugins/%1").arg(m_plugin.id));
    QDir().mkpath(configDirectory);
    environment.insert("FLAMESHOT_PLUGIN_CONFIG_DIR", configDirectory);
    m_process.setProcessEnvironment(environment);
    m_process.setWorkingDirectory(
      QFileInfo(m_plugin.manifestPath).absolutePath());
    m_process.setProgram(m_plugin.executable);
    m_process.setArguments(m_plugin.arguments);
    m_timeout.start(m_plugin.timeoutMs);
    m_process.start();
}

void ActionPluginRunner::processFinished(int exitCode,
                                         QProcess::ExitStatus exitStatus)
{
    if (m_finished) {
        return;
    }
    m_timeout.stop();
    if (!collectOutput()) {
        return;
    }

    const QString standardError = QString::fromUtf8(m_standardError).trimmed();
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        finishWithError(standardError.isEmpty()
                          ? tr("Plugin %1 failed with exit code %2.")
                              .arg(m_plugin.name)
                              .arg(exitCode)
                          : standardError);
        return;
    }
    finishSuccessfully(m_standardOutput);
}

bool ActionPluginRunner::collectOutput()
{
    if (m_finished) {
        return false;
    }
    m_standardOutput.append(m_process.readAllStandardOutput());
    m_standardError.append(m_process.readAllStandardError());
    if (m_standardOutput.size() > MaxStandardOutputSize ||
        m_standardError.size() > MaxStandardErrorSize) {
        m_process.kill();
        finishWithError(
          tr("Plugin %1 produced too much output.").arg(m_plugin.name));
        return false;
    }
    return true;
}

void ActionPluginRunner::processFailed()
{
    if (!m_finished) {
        finishWithError(m_process.errorString());
    }
}

void ActionPluginRunner::processTimedOut()
{
    if (m_finished) {
        return;
    }
    m_process.kill();
    finishWithError(tr("Plugin %1 timed out.").arg(m_plugin.name));
}

void ActionPluginRunner::finishWithError(const QString& message)
{
    if (m_finished) {
        return;
    }
    m_finished = true;
    m_timeout.stop();
    SystemNotification().sendMessage(message, tr("Action Plugin Error"), {});
    emit finished(false);
    deleteLater();
}

void ActionPluginRunner::finishSuccessfully(const QByteArray& standardOutput)
{
    if (m_finished) {
        return;
    }
    m_timeout.stop();

    if (m_plugin.outputMode == ActionPlugin::OutputMode::ClipboardText) {
        const QString text = QString::fromUtf8(standardOutput).trimmed();
        if (text.isEmpty()) {
            finishWithError(
              tr("Plugin %1 returned no text.").arg(m_plugin.name));
            return;
        }
        FlameshotDaemon::copyToClipboard(
          text, tr("%1 output copied to clipboard.").arg(m_plugin.name));
    } else if (m_plugin.outputMode == ActionPlugin::OutputMode::Json) {
        if (!handleJsonResult(standardOutput)) {
            return;
        }
    } else {
        SystemNotification().sendMessage(
          tr("Plugin finished successfully."), m_plugin.name, {});
    }
    m_finished = true;
    emit finished(true);
    deleteLater();
}

bool ActionPluginRunner::handleJsonResult(const QByteArray& standardOutput)
{
    QJsonParseError parseError;
    const QJsonDocument document =
      QJsonDocument::fromJson(standardOutput, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        const QString detail = parseError.error == QJsonParseError::NoError
                                 ? tr("the root value is not an object")
                                 : parseError.errorString();
        finishWithError(tr("Plugin %1 returned invalid result JSON: %2")
                          .arg(m_plugin.name, detail));
        return false;
    }

    const QJsonObject root = document.object();
    if (root.value("protocol_version").toInt(-1) !=
        ActionPluginLoader::ApiVersion) {
        finishWithError(
          tr("Plugin %1 returned an unsupported protocol version.")
            .arg(m_plugin.name));
        return false;
    }

    const QString status = root.value("status").toString();
    if (status == QLatin1String("error")) {
        const QString message = root.value("message").toString().trimmed();
        finishWithError(
          message.isEmpty()
            ? tr("Plugin %1 reported an error.").arg(m_plugin.name)
            : message);
        return false;
    }
    if (status != QLatin1String("success")) {
        finishWithError(tr("Plugin %1 returned an invalid result status.")
                          .arg(m_plugin.name));
        return false;
    }

    const QJsonObject result = root.value("result").toObject();
    const QString type = result.value("type").toString();
    if (type == QLatin1String("clipboard-text")) {
        const QString text = result.value("text").toString();
        if (text.isEmpty()) {
            finishWithError(
              tr("Plugin %1 returned no text.").arg(m_plugin.name));
            return false;
        }
        FlameshotDaemon::copyToClipboard(
          text, tr("%1 output copied to clipboard.").arg(m_plugin.name));
        return true;
    }
    if (type == QLatin1String("notification")) {
        const QString text = result.value("text").toString();
        const QString title = result.value("title").toString(m_plugin.name);
        if (text.isEmpty()) {
            finishWithError(tr("Plugin %1 returned an empty notification.")
                              .arg(m_plugin.name));
            return false;
        }
        SystemNotification().sendMessage(text, title, {});
        return true;
    }
    if (type == QLatin1String("none")) {
        return true;
    }

    finishWithError(tr("Plugin %1 returned unsupported result type '%2'.")
                      .arg(m_plugin.name, type));
    return false;
}
