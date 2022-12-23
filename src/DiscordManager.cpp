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

    discord::Activity activity{};
    activity.SetState("Loading...");
    activity.GetTimestamps().SetStart(timeStart);
    activity.GetAssets().SetLargeImage("cool");
    activity.GetAssets().SetLargeText("Using GDMenu by maxnut");
    activity.SetType(discord::ActivityType::Playing);
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
}