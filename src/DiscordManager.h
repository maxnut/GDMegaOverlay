#pragma once
#include "pch.h"
#include <discord.h>

class DiscordManager
{
    

public:
    discord::Core *core{};
    discord::ClientId id = 1055528380956672081;

    long long timeStart = 0;

    void InitDiscord();
};