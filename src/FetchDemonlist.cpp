#include "FetchDemonlist.h"
#include <curl.h>
#include "Hacks.h"
#include "json.hpp"
using json = nlohmann::json;

static std::string ids = "";

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

void FetchDemonlist::callback(CCObject *)
{

    std::string info;
    std::string level_id = "70000000";

    if (ids.empty())
    {
        curl_global_init(CURL_GLOBAL_ALL);

        CURL *curl;
        CURLcode res;

        std::string readBuffer;

        curl = curl_easy_init();

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, "https://pointercrate.com/api/v2/demons/listed/?limit=100");
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

            for (int i = 0; i < 100; i++)
            {
                if (data[i]["level_id"].dump() != "null")
                {
                    int id = data[i]["level_id"];
                    level_id += i == 99 ? std::to_string(id) : std::to_string(id) + ",";
                }
            }
        }

        // pointercrate only allows 100 levels to be fetched so i just make another request
        CURL *curl2;
        CURLcode res2;

        std::string readBuffer2;

        curl2 = curl_easy_init();

        if (curl2)
        {
            curl_easy_setopt(curl2, CURLOPT_URL, "https://pointercrate.com/api/v2/demons/listed/?after=100&limit=50");
            curl_easy_setopt(curl2, CURLOPT_HTTPGET, 1L);
            curl_easy_setopt(curl2, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl2, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
            curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &readBuffer2);
            curl_easy_setopt(curl2, CURLOPT_VERBOSE, 1L);
            res2 = curl_easy_perform(curl2);
            curl_easy_cleanup(curl2);

            info = readBuffer2.c_str();
            json data = json::parse(info);

            for (int i = 0; i < 50; i++)
            {
                if (data[i]["level_id"].dump() != "null")
                {
                    int id = data[i]["level_id"];
                    level_id += i == 49 ? std::to_string(id) : std::to_string(id) + ",";
                }
            }
        }

        ids = level_id;
    }
    else
        level_id = ids;

    auto scene = CCScene::create();
    auto obj = gd::GJSearchObject::create(gd::SearchType(10));
    obj->m_sSearchQuery = level_id;
    auto layer = gd::LevelBrowserLayer::create(obj);
    scene->addChild(layer);
    CCDirector::sharedDirector()->popScene();
    CCDirector::sharedDirector()->pushScene(scene);
}