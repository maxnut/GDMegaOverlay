#include "FetchDemonlist.h"
#include "Hacks.h"

void FetchDemonlist::demonlistCallback(CCObject *)
{
	gd::LevelBrowserLayer::scene(gd::GJSearchObject::create(gd::SearchType(3141)));
}

void FetchDemonlist::challengeListCallback(CCObject *)
{
    gd::LevelBrowserLayer::scene(gd::GJSearchObject::create(gd::SearchType(3142)));
}