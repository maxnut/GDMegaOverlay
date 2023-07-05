#include "ListFetcher.h"
#include "Hacks.h"

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


std::atomic_bool ListFetcher::isFetching = false;

void ListFetcher::init()
{
	static bool firstLaunch = true;
	isFetching = false;

	if (firstLaunch)
	{
		curl_global_init(CURL_GLOBAL_ALL);

		firstLaunch = false;
	}
}

curlFetchResponse ListFetcher::fetchLink(std::string link)
{
	curlFetchResponse err{ CURLcode::CURLE_HTTP_RETURNED_ERROR, "", {} };

	try
	{
		CURLcode res = CURLcode::CURLE_HTTP_RETURNED_ERROR;
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
				return err;

			try
			{
				data = nlohmann::json::parse(info);
			}
			catch (...)
			{
				return err;
			}
		}

		return { res, info, data };
	}
	catch (...)
	{
		return err;
	}
}

void ListFetcher::getRandomNormalListLevel(int stars, nlohmann::json& json)
{
	if (stars > 10)
	{
		json = {};
		return;
	}
	isFetching = true;

	std::stringstream link;
	link
		<< "https://gdbrowser.com/api/search/*"
		<< "?diff=" << stars
		<< "&starred"
		<< "&page=" << Hacks::randomInt(1, m_normalListMaxPage[stars - 1]);

	curlFetchResponse response = fetchLink(link.str());

	nlohmann::json responseJson = response.jsonResponse;
	// prevent auto levels from appearing in the Easy difficulty
	if (responseJson.size() != 0 && stars == 1)
	{
		for (auto it = responseJson.begin(); it != responseJson.end();)
		{
			if (it.value()["difficulty"].get<std::string>() == "Auto")
				it = responseJson.erase(it);
			else
				it++;
		}
	}

	json = responseJson[Hacks::randomInt(0, responseJson.size() - 1)];

	isFetching = false;
}

void ListFetcher::getRandomDemonListLevel(nlohmann::json& json)
{
	isFetching = true;
	std::stringstream link;
	link
		<< "https://pointercrate.com/api/v2/demons/listed"
		<< "?limit=" << 100
		<< "&after=" << Hacks::randomInt(0, m_demonListMaxPage);

	curlFetchResponse response = fetchLink(link.str());
	int index = Hacks::randomInt(0, response.jsonResponse.size() - 1);

	// like wtf pointercrate
	while (response.jsonResponse[index]["level_id"].is_null())
		index = Hacks::randomInt(0, response.jsonResponse.size() - 1);

	getLevelInfo(response.jsonResponse[index]["level_id"].get<int>(), std::ref(json));

	isFetching = false;
}

// TODO: figure out how to get extended list & the rest of the list (current limit is 50 levels)
void ListFetcher::getRandomChallengeListLevel(nlohmann::json& json)
{
	isFetching = true;
	std::string link = "https://challengelist.gd/api/v1/demons/";

	curlFetchResponse response = fetchLink(link);
	int index = Hacks::randomInt(0, response.jsonResponse.size() - 1);

	while (response.jsonResponse[index]["level_id"].is_null())
		index = Hacks::randomInt(0, response.jsonResponse.size() - 1);

	getLevelInfo(response.jsonResponse[index]["level_id"].get<int>(), std::ref(json));

	isFetching = false;
}

void ListFetcher::getLevelInfo(int levelID, nlohmann::json& json)
{
	std::stringstream link;

	link
		<< "https://gdbrowser.com/api/search/"
		<< levelID;

	curlFetchResponse response = fetchLink(link.str());

	if (response.jsonResponse.size() > 0)
	{
		json = response.jsonResponse[0];
		return;
	}

	json = {};
}