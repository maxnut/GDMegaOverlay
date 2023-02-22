#include "DiscordManager.h"
#include "Hacks.h"

void DiscordManager::InitDiscord()
{
    if(!core) auto result = discord::Core::Create(id, DiscordCreateFlags_NoRequireDiscord, &core);

    if (!core)
    {
        return;
    }

    timeStart = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if(!hacks.discordRPC) return;

    Hacks::UpdateRichPresence(2);
}