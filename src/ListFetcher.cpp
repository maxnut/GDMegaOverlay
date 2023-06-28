#include "ListFetcher.h"
#include "Hacks.h"
#include <nlohmann/json.hpp>


std::size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* s)
{
    std::size_t newLength = size * nmemb;

    try
    {
        s->append(reinterpret_cast<char*>(contents), newLength);
    }
    catch (std::bad_alloc& e)
    {
        return 0;
    }

    return newLength;
}


std::atomic_bool ListFetcher::finishedFetching = true;

void ListFetcher::init()
{
	static bool firstLaunch = true;
	finishedFetching = true;

	if (firstLaunch)
	{
		curl_global_init(CURL_GLOBAL_ALL);

		firstLaunch = false;
	}
}

curlFetchResponse ListFetcher::fetchLink(std::string link)
{
	try
	{
		CURLcode res = CURLcode::CURLE_COULDNT_CONNECT;
		std::string readBuffer;
		std::string info;
		nlohmann::json data;

		if (auto curl = curl_easy_init(); curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			info = readBuffer.c_str();

			if (info == "-1")
				return { CURLcode::CURLE_COULDNT_CONNECT, "", {} };

			data = nlohmann::json::parse(info);
		}

		return { res, info, data };
	}
	catch (...)
	{
		return { CURLcode::CURLE_COULDNT_CONNECT, "", {} };
	}
}

void ListFetcher::getNormalList(int stars, nlohmann::json& json)
{
	if (stars > 10)
	{
		json = {};
		return;
	}
	finishedFetching = false;

	std::stringstream link;
	link
		<< "https://gdbrowser.com/api/search/*"
		<< "?diff=" << stars
		<< "&starred"
		<< "&page=" << Hacks::randomInt(1, m_normalListMaxPage[stars - 1]);

	curlFetchResponse response = fetchLink(link.str());

	json = response.jsonResponse;
	// prevent auto levels from appearing in the Easy difficulty
	if (json.size() != 0 && stars == 1)
	{
		for (auto it = json.begin(); it != json.end();)
		{
			if (it.value()["difficulty"].get<std::string>() == "Auto")
				it = json.erase(it);
			else
				it++;
		}
	}
	finishedFetching = true;
}

void ListFetcher::getRandomDemonListLevel(nlohmann::json& json)
{
	finishedFetching = false;
	std::stringstream link;
	link
		<< "https://pointercrate.com/api/v2/demons/listed"
		<< "?limit=" << 100
		<< "&after=" << Hacks::randomInt(0, m_demonListMaxPage);

	curlFetchResponse response = fetchLink(link.str());

	getLevelInfo(response.jsonResponse[Hacks::randomInt(0, response.jsonResponse.size())]["level_id"].get<int>(), std::ref(json));

	finishedFetching = true;
}

// TODO: figure out how to get extended list & the rest of the list (current limit is 50 levels)
void ListFetcher::getRandomChallengeListLevel(nlohmann::json& json)
{
	finishedFetching = false;
	std::string link = "https://challengelist.gd/api/v1/demons/";

	curlFetchResponse response = fetchLink(link);
	
	getLevelInfo(response.jsonResponse[Hacks::randomInt(0, response.jsonResponse.size())]["level_id"].get<int>(), std::ref(json));
	
	finishedFetching = true;
}

void ListFetcher::getLevelInfo(int levelID, nlohmann::json& json)
{
	finishedFetching = false;
	std::stringstream link;

	link
		<< "https://gdbrowser.com/api/search/"
		<< levelID;

	curlFetchResponse response = fetchLink(link.str());

	json = response.jsonResponse[0];

	finishedFetching = true;
}
