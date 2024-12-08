## Simple cross-platform Console IRC Client

Windows | Linux | Azure (Linux/macOS)
:------------: | :------------: | :------------:
[![Windows Build status](https://ci.appveyor.com/api/projects/status/pn55ra5fr2c1b6t7?svg=true)](https://ci.appveyor.com/project/fredimachado/ircclient) | [![Linux Build Status](https://travis-ci.org/fredimachado/IRCClient.svg?branch=master)](https://travis-ci.org/fredimachado/IRCClient) | [![Azure Build Status (Linux/macOS)](https://dev.azure.com/FrediMachado/IRCClient/_apis/build/status/fredimachado.IRCClient)](https://dev.azure.com/FrediMachado/IRCClient/_build/latest?definitionId=1)

- It works on windows and linux (haven't tested on mac)
- Can be used as an IRC bot
- It has a simple hook system where you can do whatever you want  when
  receiving an IRC command.
- Example in Main.cpp

### Connecting to IRC:
There are two ways to connect:
1) with commanl line arguments:
    ./ircxx host port nick user password auto // if auto provided, client connects with no connection parameters confirmation

    ./ircxx your.znc.host 7755 your_nick your_username@ZNC_client_identfier/ZNC_Network_name ZNC_password auto // connects to host (ZNC) with nick aion, ident xion, identifier ircxx, password lamodrom, no confirmation (auto)

    ./ircxx irc.rizon.net 7000 zima xion nopass auto // connects automatically with nick zima, username xion, no confirmation

    ./ircxx irc.rizon.net 7000 zima xion // connects with confirmation, nick=zima, ident=xion, asks confirmation to connect

    #### Not public samples:
           
        ./ircxx host port nick user password auto // if auto provided, client connects with no connection parameters confirmation

        ./ircxx 45.155.205.106 7788 aion xion@ircxx/rusnet -password- auto // connects to host (ZNC) with nick aion, ident xion, identifier ircxx, password -password-, no confirmation (auto)

        ./ircxx irc.rizon.net 7000 zima xion nopass auto // connects automatically with no confirmation

        ./ircxx irc.rizon.net 7000 zima xion // connects with confirmation, nick=zima, ident=xion

2) with connection parameters, read from irc.conf file

    Config file should be named irc.conf. Config file samples are licated in set/ folder.



### Hooking IRC commands:
First create a function (name it whatever you want) with two arguments, an IRCMessage and a pointer to IRCClient:

```cpp
void onPrivMsg(IRCMessage message, IRCClient* client)
{
    // Check who can "control" us
    if (message.prefix.nick != "YourNick")
        return;
    
    // received text
    std::string text = message.parameters.at(message.parameters.size() - 1);
    
    if (text == "join #channel")
        client->SendIRC("JOIN #channel");
    if (text == "leave #channel")
        client->SendIRC("PART #channel");
    if (text == "quit now")
        client->SendIRC("QUIT");
}
```

Then, after you create the IRCClient instance, you can hook it:

```cpp
IRCClient client;

// Hook PRIVMSG
client.HookIRCCommand("PRIVMSG", &onPrivMsg);
```

### Building on windows with Mingw:

- Edit Makefile
- Replace "-lpthread" with "-lws2_32" (no quotes) in LDFLAGS on line 3.
- Add ".exe" extension (no quotes) to the EXECUTABLE filename (line 8).

## Contribution
Just send a pull request! :)

## GNU LGPL
> IRCClient is free software; you can redistribute it and/or
> modify it under the terms of the GNU Lesser General Public
> License as published by the Free Software Foundation; either
> version 3.0 of the License, or any later version.
>
> This program is distributed in the hope that it will be useful,
> but WITHOUT ANY WARRANTY; without even the implied warranty of
> MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
> Lesser General Public License for more details.
>
> http://www.gnu.org/licenses/lgpl.html
