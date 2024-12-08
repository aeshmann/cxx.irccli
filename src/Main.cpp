/*
 * Copyright (C) 2011 Fredi Machado <https://github.com/fredimachado>
 * IRCClient is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * http://www.gnu.org/licenses/lgpl.html 
 */

#include <iostream>
#include <fstream>
#include <signal.h>
#include <cstdlib>
#include <map>
#include <algorithm>
#include "Thread.h"
#include "IRCClient.h"

volatile bool running;

void signalHandler(int signal)
{
    running = false;
}

class ConsoleCommandHandler
{
public:
    bool AddCommand(std::string name, int argCount, void (*handler)(std::string /*params*/, IRCClient* /*client*/))
    {
        CommandEntry entry;
        entry.argCount = argCount;
        entry.handler = handler;
        std::transform(name.begin(), name.end(), name.begin(), towlower);
        _commands.insert(std::pair<std::string, CommandEntry>(name, entry));
        return true;
    }

    void ParseCommand(std::string command, IRCClient* client)
    {
        if (_commands.empty())
        {
            std::cout << "No commands available." << std::endl;
            return;
        }

        if (command[0] == '/')
            command = command.substr(1); // Remove the slash

        std::string name = command.substr(0, command.find(" "));
        std::string args = command.substr(command.find(" ") + 1);
        int argCount = std::count(args.begin(), args.end(), ' ');

        std::transform(name.begin(), name.end(), name.begin(), towlower);

        std::map<std::string, CommandEntry>::const_iterator itr = _commands.find(name);
        if (itr == _commands.end())
        {
            std::cout << "Command not found." << std::endl;
            return;
        }

        if (++argCount < itr->second.argCount)
        {
            std::cout << "Insuficient arguments." << std::endl;
            return;
        }

        (*(itr->second.handler))(args, client);
    }

private:
    struct CommandEntry
    {
        int argCount;
        void (*handler)(std::string /*arguments*/, IRCClient* /*client*/);
    };

    std::map<std::string, CommandEntry> _commands;
};

ConsoleCommandHandler commandHandler;

void msgCommand(std::string arguments, IRCClient* client)
{
    std::string to = arguments.substr(0, arguments.find(" "));
    std::string text = arguments.substr(arguments.find(" ") + 1);

    std::cout << "To " + to + ": " + text << std::endl;
    client->SendIRC("PRIVMSG " + to + " :" + text);
};

void joinCommand(std::string channel, IRCClient* client)
{
    if (channel[0] != '#')
        channel = "#" + channel;

    client->SendIRC("JOIN " + channel);
}

void partCommand(std::string channel, IRCClient* client)
{
    if (channel[0] != '#')
        channel = "#" + channel;

    client->SendIRC("PART " + channel);
}

void ctcpCommand(std::string arguments, IRCClient* client)
{
    std::string to = arguments.substr(0, arguments.find(" "));
    std::string text = arguments.substr(arguments.find(" ") + 1);

    std::transform(text.begin(), text.end(), text.begin(), towupper);

    client->SendIRC("PRIVMSG " + to + " :\001" + text + "\001");
}

ThreadReturn inputThread(void* client)
{
    std::string command;

    commandHandler.AddCommand("msg", 2, &msgCommand);
    commandHandler.AddCommand("join", 1, &joinCommand);
    commandHandler.AddCommand("part", 1, &partCommand);
    commandHandler.AddCommand("ctcp", 2, &ctcpCommand);

    while (true)
    {
        getline(std::cin, command);
        if (command == "")
            continue;

        if (command[0] == '/')
        {
            commandHandler.ParseCommand(command, (IRCClient *)client);
        }
        else if (command == "quit")
        {
            std::cout << "Disconnecting\n";
            break;
        }
        else
        {
            ((IRCClient *)client)->SendIRC(command);
        }
    }

#ifdef _WIN32
    _endthread();
#else
    pthread_exit(NULL);
#endif
}

int main(int argc, char* argv[]) {

    char* host;
    int port;
    std::string nick("");
    std::string user("");
    std::string pass("");
    std::string confCHeck("");

        
    struct usrConf {
        std::string userNick;
        std::string userName;
        std::string userPass;
    } ircUser;

    usrConf usrGlob{"aesh", "ircx"};

    struct ircConf {
        std::string ircHost;
        int ircPort;
        bool runAuto; // if true - connects without confirmation
        usrConf ircUser;
    } ircNetwork;

    if (argc < 3)
    {
    std::cout << "No command line args given: host port [nick] [user]\n";
    std::cout << "Trying to open a configuration file \"irc.conf\"...\n";

    std::fstream fileConf("irc.conf");
    std::string lineStr;
    std::vector<std::string> confVect;
    if (fileConf.is_open() && !fileConf.eof()) {
        while (getline(fileConf, lineStr)) {
            confVect.push_back(lineStr);
            }
    } else {
        std::cout << "Couldn't open configuration file!\n";
        return 1;
    }

    for (int j = 0; j < (int)confVect.size(); j++) {
        std::string confStr(confVect[j]);
        if (confStr.starts_with("host")) {
            ircNetwork.ircHost = confStr.substr(confStr.find('[') + 1, (confStr.find(']') - confStr.find('[')) - 1 );
        } else if (confStr.starts_with("port")) {
            ircNetwork.ircPort = stoi(confStr.substr(confStr.find('[') + 1, (confStr.find(']') - confStr.find('[')) - 1));
        } else if (confStr.starts_with("user")) {
            ircUser.userName = confStr.substr(confStr.find('[') + 1, (confStr.find(']') - confStr.find('[')) - 1);
        } else if (confStr.starts_with("pass")) {
            ircUser.userPass = confStr.substr(confStr.find('[') + 1, (confStr.find(']') - confStr.find('[')) - 1);
        } else if (confStr.starts_with("nick")) {
            ircUser.userNick = confStr.substr(confStr.find('[') + 1, (confStr.find(']') - confStr.find('[')) - 1);
        } else if (confStr.starts_with("auto")) {
            std::string autoRunState = confStr.substr(confStr.find('[') + 1, (confStr.find(']') - confStr.find('[')) - 1);
            if (autoRunState == "true" ) {
                ircNetwork.runAuto = true;
            } else {
                ircNetwork.runAuto = false;
            }
        }
    }
    host = (char*)ircNetwork.ircHost.c_str();
    port = ircNetwork.ircPort;
    user = ircUser.userName;
    nick = ircUser.userNick;
    pass = ircUser.userPass;
    }
    else
    {
        host = argv[1];
        port = atoi(argv[2]);
        nick = "aion";
        user = "ircx";

        if (argc >= 4)
            nick = argv[3];
        if (argc >= 5)
            user = argv[4];
        if (argc >= 6)
            pass = argv[5];
        if (argc >= 7) {
            confCHeck = argv[6];
        }
            if (confCHeck == "auto") {
                ircNetwork.runAuto = true;
            }
           
    }

    if (!ircNetwork.runAuto)
    {
        std::cout << "IRC client will run with following parameters:" << std::endl;
        std::cout << "IRC server host:\t" << host << std::endl;
        std::cout << "IRC server port:\t" << port << std::endl;
        std::cout << "IRC user ident :\t" << user << std::endl;
        std::cout << "IRC server pass:\t" << pass << std::endl;
        std::cout << "IRC user nick  :\t" << nick << std::endl;
        std::cout << "Is this correct? y/n:";
        char runClient;
        std::cin >> runClient;
        if (runClient == 'y')
        {
            std::cout << "Starting IRC client\n";
        }
        else
        {
            std::cout << "Program will close" << std::endl;
            return 0;
        }
    }

    IRCClient client;

    client.Debug(true);

    // Start the input thread
    Thread thread;
    thread.Start(&inputThread, &client);

    if (client.InitSocket())
    {
        std::cout << "Socket initialized. Connecting..." << std::endl;

        if (client.Connect(host, port))
        {
            std::cout << "Connected. Loggin in..." << std::endl;

            if (client.Login(nick, user, pass))
            {
                std::cout << "Logged." << std::endl;

                running = true;
                signal(SIGINT, signalHandler);

                while (client.Connected() && running)
                    client.ReceiveData();
            }

            if (client.Connected())
                client.Disconnect();

            std::cout << "Disconnected." << std::endl;
        }
    }
}
