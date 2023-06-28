#pragma once
#include "pch.h"
#include <nlohmann/json.hpp>
#define CURL_STATICLIB
#include <curl/curl.h>


struct curlFetchResponse
{
	CURLcode responseCode;
	std::string info;
	nlohmann::json jsonResponse;

	curlFetchResponse(CURLcode responseCode, std::string info, nlohmann::json jsonResponse)
		: responseCode(responseCode), info(info), jsonResponse(jsonResponse) {}
};

class ListFetcher
{
private:
	inline static std::array<int, 10> m_normalListMaxPage{
		67, 160, 958, 1194, 490,
		171, 186, 178, 88, 72
	};
	inline static int m_demonListMaxPage = 490;


	static curlFetchResponse fetchLink(std::string link);

public:
	static std::atomic_bool finishedFetching;

	static void init();

	static void getNormalList(int stars, nlohmann::json& json);
	static void getRandomDemonListLevel(nlohmann::json& json);
	static void getRandomChallengeListLevel(nlohmann::json& json);
	
	static void getLevelInfo(int levelID, nlohmann::json& json);
};
