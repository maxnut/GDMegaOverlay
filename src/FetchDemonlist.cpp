#include "FetchDemonlist.h"
#include "Hacks.h"

/*#include "json.hpp"
using json = nlohmann::json;

const size_t fetch_size = 50, list_size = 150;
size_t offset = 0, list = 0;
static std::vector<std::string> demonIds, challengeIds;

gd::GJSearchObject *ob1;

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char *)contents, newLength);
    }
    catch (std::bad_alloc &e)
    {
        // handle memory problem
        return 0;
    }
    return newLength;
}

void FetchDemonlist::fetch()
{
    ob1 = gd::GJSearchObject::create(gd::SearchType(19));
    std::string level_id = "";

    std::string currentUrl = "";

    switch (list)
    {
    case 0:
        currentUrl = "https://pointercrate.com/api/v2/demons/listed/";
        break;
    case 1:
        currentUrl = "https://challengelist.gd/api/v1/demons/";
        break;
    }

    if (list == 0 && !demonIds[offset / fetch_size].empty() || list == 1 && !challengeIds[offset / fetch_size].empty())
    {
        level_id = list == 0 ? demonIds[offset / fetch_size] : challengeIds[offset / fetch_size];
        ob1->m_sSearchQuery = level_id;
        load();
        return;
    }

    curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl;
    CURLcode res;

    std::string readBuffer;

    curl = curl_easy_init();

    if (curl)
    {
        std::string info;

        std::stringstream ss;
        ss << currentUrl << "?limit=" << fetch_size << "&after=" << offset;
        curl_easy_setopt(curl, CURLOPT_URL, ss.str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        info = readBuffer.c_str();
        level_id = "";
        json data = json::parse(info);

        for (int i = 0; i < fetch_size; i++)
        {
            if (data[i]["level_id"].dump() != "null")
            {
                int id = data[i]["level_id"];
                level_id += i == fetch_size - 1 ? std::to_string(id) : std::to_string(id) + ",";
            }
            else if (data[i]["name"].get<std::string>() == "Acheron" && data[i]["level_id"].dump() == "null")
            {
                level_id += "73667628,";
            }
        }
    }

    list == 0 ? demonIds[offset / fetch_size] = level_id : challengeIds[offset / fetch_size] = level_id;

    ob1->m_sSearchQuery = level_id;

    load();
}*/

void FetchDemonlist::demonlistCallback(CCObject *)
{
	gd::LevelBrowserLayer::scene(gd::GJSearchObject::create(gd::SearchType(3141)));
}

void FetchDemonlist::challengeListCallback(CCObject *)
{
    gd::LevelBrowserLayer::scene(gd::GJSearchObject::create(gd::SearchType(3142)));
}