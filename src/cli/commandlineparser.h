// Copyright 2017 Alejandro Sirgo Rica
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include "src/cli/commandargument.h"
#include "src/cli/commandoption.h"
#include <QMap>

class CommandLineParser
{
public:
    CommandLineParser();

    bool parse(const QStringList &args);

    CommandArgument rootArgument() const { return CommandArgument(); }

    CommandOption addVersionOption();
    CommandOption addHelpOption();

    bool AddArgument(const CommandArgument &arg,
                     const CommandArgument &parent = CommandArgument());

    bool AddOption(const CommandOption &option,
                     const CommandArgument &parent = CommandArgument());

    bool AddOptions(const QList<CommandOption> &options,
                     const CommandArgument &parent = CommandArgument());

    void setGeneralErrorMessage(const QString &msg);
    void setDescription(const QString &description);

    bool isSet(const CommandArgument &arg) const;
    bool isSet(const CommandOption &option) const;
    QString value(const CommandOption &option) const;

private:
    bool m_withHelp = false;
    bool m_withVersion = false;
    QString m_description;
    QString  m_generalErrorMessage;

    struct Node {
        Node(const CommandArgument &arg) : argument(arg) {}
        Node() {}
        bool operator==(const Node &n) const {
            return argument == n.argument &&
                    options == n.options &&
                    subNodes == n.subNodes;
        }
        CommandArgument argument;
        QMap<QStringList, CommandOption> options;
        QMap<QString, Node> subNodes;
    };

    Node m_parseTree;
    QList<CommandOption> m_foundOptions;
    QList<CommandArgument> m_foundArgs;

    // helper functions
    inline void printVersion();
    void printHelp(QStringList args, const Node *node);
    Node* findParent(const CommandArgument &parent);
    Node* recursiveParentSearch(const CommandArgument &parent,
                                Node &node) const;
    inline bool processIfOptionIsHelp(const QStringList &args,
                                QStringList::const_iterator &actualIt,
                                Node * &actualNode);
    inline bool processArgs(const QStringList &args,
                            QStringList::const_iterator &actualIt,
                            Node * &actualNode);
    bool processOptions(const QStringList &args,
                        QStringList::const_iterator &actualIt,
                        Node *const actualNode);

};

#endif // COMMANDLINEPARSER_H
