// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "commandlineparser.h"
#include "abstractlogger.h"
#include "src/utils/globalvalues.h"
#include <QApplication>
#include <QTextStream>

CommandLineParser::CommandLineParser()
  : m_description(qApp->applicationName())
{}

namespace {

AbstractLogger out =
  AbstractLogger::info(AbstractLogger::Stderr).enableMessageHeader(false);
AbstractLogger err = AbstractLogger::error(AbstractLogger::Stderr);

auto versionOption =
  CommandOption({ "v", "version" },
                QStringLiteral("Displays version information"));
auto helpOption =
  CommandOption({ "h", "help" }, QStringLiteral("Displays this help"));

QString optionsToString(const QList<CommandOption>& options,
                        const QList<CommandArgument>& arguments)
{
    int size = 0; // track the largest size
    QStringList dashedOptionList;
    // save the dashed options and its size in order to print the description
    // of every option at the same horizontal character position.
    for (auto const& option : options) {
        QStringList dashedOptions = option.dashedNames();
        QString joinedDashedOptions = dashedOptions.join(QStringLiteral(", "));
        if (!option.valueName().isEmpty()) {
            joinedDashedOptions +=
              QStringLiteral(" <%1>").arg(option.valueName());
        }
        if (joinedDashedOptions.length() > size) {
            size = joinedDashedOptions.length();
        }
        dashedOptionList << joinedDashedOptions;
    }
    // check the length of the arguments
    for (auto const& arg : arguments) {
        if (arg.name().length() > size) {
            size = arg.name().length();
        }
    }
    // generate the text
    QString result;
    if (!dashedOptionList.isEmpty()) {
        result += QObject::tr("Options") + ":\n";
        QString linePadding =
          QStringLiteral(" ").repeated(size + 4).prepend("\n");
        for (int i = 0; i < options.length(); ++i) {
            result += QStringLiteral("  %1  %2\n")
                        .arg(dashedOptionList.at(i).leftJustified(size, ' '))
                        .arg(options.at(i).description().replace(
                          QLatin1String("\n"), linePadding));
        }
        if (!arguments.isEmpty()) {
            result += QLatin1String("\n");
        }
    }
    if (!arguments.isEmpty()) {
        result += QObject::tr("Arguments") + ":\n";
    }
    for (const auto& argument : arguments) {
        result += QStringLiteral("  %1  %2\n")
                    .arg(argument.name().leftJustified(size, ' '))
                    .arg(argument.description());
    }
    return result;
}

} // unnamed namespace

bool CommandLineParser::processArgs(const QStringList& args,
                                    QStringList::const_iterator& actualIt,
                                    Node*& actualNode)
{
    QString argument = *actualIt;
    bool ok = true;
    bool isValidArg = false;
    for (Node& n : actualNode->subNodes) {
        if (n.argument.name() == argument) {
            actualNode = &n;
            isValidArg = true;
            break;
        }
    }
    if (isValidArg) {
        auto nextArg = actualNode->argument;
        m_foundArgs.append(nextArg);
        // check next is help
        ++actualIt;
        ok = processIfOptionIsHelp(args, actualIt, actualNode);
        --actualIt;
    } else {
        ok = false;
        err << QStringLiteral("'%1' is not a valid argument.").arg(argument);
    }
    return ok;
}

bool CommandLineParser::processOptions(const QStringList& args,
                                       QStringList::const_iterator& actualIt,
                                       Node* const actualNode)
{
    QString arg = *actualIt;
    bool ok = true;
    // track values
    int equalsPos = arg.indexOf(QLatin1String("="));
    QString valueStr;
    if (equalsPos != -1) {
        valueStr = arg.mid(equalsPos + 1); // right
        arg = arg.mid(0, equalsPos);       // left
    }
    // check format -x --xx...
    bool isDoubleDashed = arg.startsWith(QLatin1String("--"));
    ok = isDoubleDashed ? arg.length() > 3 : arg.length() == 2;
    if (!ok) {
        err << QStringLiteral("the option %1 has a wrong format.").arg(arg);
        return ok;
    }
    arg = isDoubleDashed ? arg.remove(0, 2) : arg.remove(0, 1);
    // get option
    auto endIt = actualNode->options.cend();
    auto optionIt = endIt;
    for (auto i = actualNode->options.cbegin(); i != endIt; ++i) {
        if ((*i).names().contains(arg)) {
            optionIt = i;
            break;
        }
    }
    if (optionIt == endIt) {
        QString argName = actualNode->argument.name();
        if (argName.isEmpty()) {
            argName = qApp->applicationName();
        }
        err << QStringLiteral("the option '%1' is not a valid option "
                              "for the argument '%2'.")
                 .arg(arg)
                 .arg(argName);
        ok = false;
        return ok;
    }
    // check presence of values
    CommandOption option = *optionIt;
    bool requiresValue = !(option.valueName().isEmpty());
    if (!requiresValue && equalsPos != -1) {
        err << QStringLiteral("the option '%1' contains a '=' and it doesn't "
                              "require a value.")
                 .arg(arg);
        ok = false;
        return ok;
    } else if (requiresValue && valueStr.isEmpty()) {
        // find in the next
        if (actualIt + 1 != args.cend()) {
            ++actualIt;
        } else {
            err << QStringLiteral("Expected value after the option '%1'.")
                     .arg(arg);
            ok = false;
            return ok;
        }
        valueStr = *actualIt;
    }
    // check the value correctness
    if (requiresValue) {
        ok = option.checkValue(valueStr);
        if (!ok) {
            QString msg = option.errorMsg();
            if (!msg.endsWith(QLatin1String("."))) {
                msg += QLatin1String(".");
            }
            err << msg;
            return ok;
        }
        option.setValue(valueStr);
    }
    m_foundOptions.append(option);
    return ok;
}

bool CommandLineParser::parse(const QStringList& args)
{
    m_foundArgs.clear();
    m_foundOptions.clear();
    bool ok = true;
    Node* actualNode = &m_parseTree;
    auto it = ++args.cbegin();
    // check  version option
    QStringList dashedVersion = versionOption.dashedNames();
    if (m_withVersion && args.length() > 1 &&
        dashedVersion.contains(args.at(1))) {
        if (args.length() == 2) {
            printVersion();
            m_foundOptions << versionOption;
        } else {
            err << "Invalid arguments after the version option.";
            ok = false;
        }
        return ok;
    }
    // check  help option
    ok = processIfOptionIsHelp(args, it, actualNode);
    // process the other args
    for (; it != args.cend() && ok; ++it) {
        const QString& value = *it;
        if (value.startsWith(QLatin1String("-"))) {
            ok = processOptions(args, it, actualNode);

        } else {
            ok = processArgs(args, it, actualNode);
        }
    }
    if (!ok && !m_generalErrorMessage.isEmpty()) {
        err.enableMessageHeader(false);
        err << m_generalErrorMessage;
        err.enableMessageHeader(true);
    }
    return ok;
}

CommandOption CommandLineParser::addVersionOption()
{
    m_withVersion = true;
    return versionOption;
}

CommandOption CommandLineParser::addHelpOption()
{
    m_withHelp = true;
    return helpOption;
}

bool CommandLineParser::AddArgument(const CommandArgument& arg,
                                    const CommandArgument& parent)
{
    bool res = true;
    Node* n = findParent(parent);
    if (n == nullptr) {
        res = false;
    } else {
        Node child;
        child.argument = arg;
        n->subNodes.append(child);
    }
    return res;
}

bool CommandLineParser::AddOption(const CommandOption& option,
                                  const CommandArgument& parent)
{
    bool res = true;
    Node* n = findParent(parent);
    if (n == nullptr) {
        res = false;
    } else {
        n->options.append(option);
    }
    return res;
}

bool CommandLineParser::AddOptions(const QList<CommandOption>& options,
                                   const CommandArgument& parent)
{
    bool res = true;
    for (auto const& option : options) {
        if (!AddOption(option, parent)) {
            res = false;
            break;
        }
    }
    return res;
}

void CommandLineParser::setGeneralErrorMessage(const QString& msg)
{
    m_generalErrorMessage = msg;
}

void CommandLineParser::setDescription(const QString& description)
{
    m_description = description;
}

bool CommandLineParser::isSet(const CommandArgument& arg) const
{
    return m_foundArgs.contains(arg);
}

bool CommandLineParser::isSet(const CommandOption& option) const
{
    return m_foundOptions.contains(option);
}

QString CommandLineParser::value(const CommandOption& option) const
{
    QString value = option.value();
    for (const CommandOption& fOption : m_foundOptions) {
        if (option == fOption) {
            value = fOption.value();
            break;
        }
    }
    return value;
}

void CommandLineParser::printVersion()
{
    out << GlobalValues::versionInfo();
}

void CommandLineParser::printHelp(QStringList args, const Node* node)
{
    args.removeLast(); // remove the help, it's always the last
    QString helpText;

    // add usage info
    QString argName = node->argument.name();
    if (argName.isEmpty()) {
        argName = qApp->applicationName();
    }
    QString argText =
      node->subNodes.isEmpty() ? "" : "[" + QObject::tr("arguments") + "]";
    helpText += (QObject::tr("Usage") + ": %1 [%2-" + QObject::tr("options") +
                 QStringLiteral("] %3\n\n"))
                  .arg(args.join(QStringLiteral(" ")))
                  .arg(argName)
                  .arg(argText);

    // short section about default behavior
    helpText += QObject::tr("Per default runs Flameshot in the background and "
                            "adds a tray icon for configuration.");
    helpText += "\n\n";

    // add command options and subarguments
    QList<CommandArgument> subArgs;
    for (const Node& n : node->subNodes) {
        subArgs.append(n.argument);
    }
    auto modifiedOptions = node->options;
    if (m_withHelp) {
        modifiedOptions << helpOption;
    }
    if (m_withVersion && node == &m_parseTree) {
        modifiedOptions << versionOption;
    }
    helpText += optionsToString(modifiedOptions, subArgs);
    // print it
    out << helpText;
}

CommandLineParser::Node* CommandLineParser::findParent(
  const CommandArgument& parent)
{
    if (parent == CommandArgument()) {
        return &m_parseTree;
    }
    // find the parent in the subNodes recursively
    Node* res = nullptr;
    for (auto& subNode : m_parseTree.subNodes) {
        res = recursiveParentSearch(parent, subNode);
        if (res != nullptr) {
            break;
        }
    }
    return res;
}

CommandLineParser::Node* CommandLineParser::recursiveParentSearch(
  const CommandArgument& parent,
  Node& node) const
{
    Node* res = nullptr;
    if (node.argument == parent) {
        res = &node;
    } else {
        for (auto& subNode : node.subNodes) {
            res = recursiveParentSearch(parent, subNode);
            if (res != nullptr) {
                break;
            }
        }
    }
    return res;
}

bool CommandLineParser::processIfOptionIsHelp(
  const QStringList& args,
  QStringList::const_iterator& actualIt,
  Node*& actualNode)
{
    bool ok = true;
    auto dashedHelpNames = helpOption.dashedNames();
    if (m_withHelp && actualIt != args.cend() &&
        dashedHelpNames.contains(*actualIt)) {
        if (actualIt + 1 == args.cend()) {
            m_foundOptions << helpOption;
            printHelp(args, actualNode);
            actualIt++;
        } else {
            err << "Invalid arguments after the help option.";
            ok = false;
        }
    }
    return ok;
}
