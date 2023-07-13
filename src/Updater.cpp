#include "Updater.h"
#include "API.h"
#include "Hacks.h"
#include "json.hpp"
#include "miniz.h"
#include "pch.h"
#include <atomic>
#include <condition_variable>
#include <curl.h>
#include <mutex>

json savedRequest;
std::string zipUrl, dllUrl;

size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	return fwrite(ptr, size, nmemb, stream);
}

size_t CurlWrite_CallbackFunc_StdString(void* contents, size_t size, size_t nmemb, std::string* s)
{
	size_t newLength = size * nmemb;
	try
	{
		s->append((char*)contents, newLength);
	}
	catch (std::bad_alloc& e)
	{
		// handle memory problem
		return 0;
	}
	return newLength;
}

json ver;

void Updater::CheckUpdate()
{

	std::ifstream version;
	std::stringstream buffer;

	version.open("GDMenu/version.json");

	if (version.is_open())
	{

		buffer << version.rdbuf();
		ver = json::parse(buffer.str());
		version.close();
		buffer.str("");
		buffer.clear();
	}
	else
	{
		ver = json::object();
		ver["release_id"] = 0;
		ver["dll_id"] = 0;
		ver["zip_id"] = 0;
		ver["ver"] = "unknown";
	}

	ExternData::version = ver["ver"];

	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl;
	CURLcode res;

	std::string readBuffer;

	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/maxnut/GDMegaOverlay/releases/latest");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "maxnut");
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		std::string info = readBuffer.c_str();
		savedRequest = json::parse(info);

		ver["ver"] = savedRequest["name"];
		ExternData::description = savedRequest["body"];

		if (savedRequest["id"].get<int>() != ver["release_id"].get<int>())
		{
			ExternData::newRelease = true;
			for (json jsonObj : savedRequest["assets"])
			{
				if (jsonObj["content_type"] == "application/x-zip-compressed" &&
					jsonObj["id"].get<int>() != ver["zip_id"].get<int>())
				{
					ver["zip_id"] = jsonObj["id"];
					zipUrl = jsonObj["browser_download_url"];
				}
				else if (jsonObj["content_type"] == "application/x-msdownload" &&
						 jsonObj["id"].get<int>() != ver["dll_id"].get<int>())
				{
					ver["dll_id"] = jsonObj["id"];
					dllUrl = jsonObj["browser_download_url"];
				}
			}
		}
		else
		{
			// give priority to zips
			bool foundZip = false;
			for (json jsonObj : savedRequest["assets"])
			{
				if (jsonObj["content_type"] == "application/x-zip-compressed" && ver["zip_id"].get<int>() > 0 &&
					jsonObj["id"].get<int>() != ver["zip_id"].get<int>())
				{
					ver["zip_id"] = jsonObj["id"];
					foundZip = true;
					ExternData::updatedZip = true;
					zipUrl = jsonObj["browser_download_url"];
				}
			}
			for (json jsonObj : savedRequest["assets"])
			{
				if (jsonObj["content_type"] == "application/x-msdownload" &&
					jsonObj["id"].get<int>() != ver["dll_id"].get<int>())
				{
					ver["dll_id"] = jsonObj["id"];
					if (!foundZip)
						ExternData::updatedDll = true;
					dllUrl = jsonObj["browser_download_url"];
				}
			}
		}
	}

	ver["release_id"] = savedRequest["id"];
}

size_t WriteCallback(void* pOpaque, mz_uint64 file_ofs, const void* pBuf, size_t size)
{
	std::ofstream* pOutputFile = reinterpret_cast<std::ofstream*>(pOpaque);
	pOutputFile->write(reinterpret_cast<const char*>(pBuf), static_cast<std::streamsize>(size));
	return size;
}

bool ExtractZipFile(const std::string& zipFilePath, const std::string& destPath)
{
	mz_zip_archive zipArchive;
	std::memset(&zipArchive, 0, sizeof(zipArchive));

	if (!mz_zip_reader_init_file(&zipArchive, zipFilePath.c_str(), 0))
	{
		std::cout << "Failed to open ZIP file" << std::endl;
		return false;
	}

	mz_uint numFiles = mz_zip_reader_get_num_files(&zipArchive);
	for (mz_uint i = 0; i < numFiles; ++i)
	{
		mz_zip_archive_file_stat fileStat;
		if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
		{
			std::cout << "Failed to get file info from ZIP" << std::endl;
			mz_zip_reader_end(&zipArchive);
			return false;
		}

		std::string outputPath = destPath + fileStat.m_filename;

		if (outputPath.find("mod/") != std::string::npos || outputPath.find("windows.json") != std::string::npos)
			continue;

		if (fileStat.m_is_directory)
		{
			std::filesystem::create_directory(outputPath.c_str());
		}
		else
		{
			// File entry, extract file
			mz_zip_archive_file_stat fileStat;
			if (!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
			{
				std::cout << "Failed to get file info from ZIP" << std::endl;
				mz_zip_reader_end(&zipArchive);
				return false;
			}

			std::ofstream outputFile(outputPath, std::ios::binary);
			if (outputFile)
			{
				mz_zip_reader_extract_to_callback(&zipArchive, i, WriteCallback, &outputFile, 0);

				outputFile.close();
			}
			else
			{
				std::cout << "Failed to create output file: " << outputPath << std::endl;
				mz_zip_reader_end(&zipArchive);
				return false;
			}
		}
	}

	mz_zip_reader_end(&zipArchive);
	return true;
}

std::mutex downloadMutex;
std::condition_variable downloadFinished;
bool isDownloadFinished = false;

int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	if (dltotal > 0)
	{
		if (static_cast<double>(dlnow) / static_cast<double>(dltotal) > 0.99)
			ExternData::downloadProgress =
				((static_cast<double>(dlnow) / static_cast<double>(dltotal)) - 0.99) * 10000.0;
	}

	return 0;
}

void DownloadThread(bool zip, std::function<void(bool)> callback)
{
	CURL* curl;

	curl = curl_easy_init();

	CURLcode res;

	FILE* fp;
	std::stringstream stream;

	std::string path =
		CCFileUtils::sharedFileUtils()->getWritablePath2() + (zip ? "GDMenu\\update.zip" : "GDMenu\\GDMenu.dll");

	if (curl && !std::filesystem::exists(path))
	{
		fp = fopen(path.c_str(), "wb");
		curl_easy_setopt(curl, CURLOPT_URL, zip ? zipUrl : dllUrl);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "maxnut");
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
	}

	{
		std::lock_guard<std::mutex> lock(downloadMutex);
		isDownloadFinished = true;
	}
	downloadFinished.notify_one();

	callback(zip);
}

void AfterDownload(bool zip)
{
	std::ofstream f;

	f.open("GDMenu/version.json");
	if (f)
		f << ver.dump(4);
	f.close();

	if (zip)
	{
		std::string path =
			CCFileUtils::sharedFileUtils()->getWritablePath2() + (zip ? "GDMenu\\update.zip" : "GDMenu\\GDMenu.dll");

		std::string pathToExtract = CCFileUtils::sharedFileUtils()->getWritablePath2() + ".tempextract\\";

		std::filesystem::create_directory(pathToExtract);
		ExtractZipFile(path, pathToExtract);

		std::filesystem::remove(path);

		f.open("move.bat");
		f << "@echo off" << std::endl;
		f << "set \"sourceFolder=" << pathToExtract << "\"" << std::endl;
		f << "set \"destinationFolder=" << CCFileUtils::sharedFileUtils()->getWritablePath2() << "\"" << std::endl;
		f << "timeout /T " << 2 << " /NOBREAK" << std::endl;
		f << "xcopy /Y /Q /E \"%sourceFolder%\\*\" \"%destinationFolder%\"" << std::endl;
		f << "echo Updated successfully! You can now close this window." << std::endl;
		f << "rmdir /S /Q \"%sourceFolder%\"" << std::endl;
		f << "del \"%~f0\"" << std::endl;
		f.close();

		std::string command =
			"start /B \"\" \"" + (CCFileUtils::sharedFileUtils()->getWritablePath2() + "move.bat") + "\"";
		std::system(command.c_str());
	}
	else
	{
		f.open("move.bat");
		f << "@echo off" << std::endl;
		f << "set \"sourceFile=" << CCFileUtils::sharedFileUtils()->getWritablePath2() + "GDMenu\\GDMenu.dll"
		  << "\"" << std::endl;
		f << "set \"destinationFolder=" << CCFileUtils::sharedFileUtils()->getWritablePath2() << "\"" << std::endl;
		f << "timeout /T " << 2 << " /NOBREAK" << std::endl;
		f << "move /Y \"%sourceFile%\" \"%destinationFolder%\"" << std::endl;
		f << "echo Updated successfully! You can now close this window." << std::endl;
		f << "del \"%~f0\"" << std::endl;
		f.close();

		std::string command =
			"start /B \"\" \"" + (CCFileUtils::sharedFileUtils()->getWritablePath2() + "move.bat") + "\"";
		std::system(command.c_str());
	}

	std::exit(0);
}

void Updater::Download(bool zip)
{
	std::thread downloadThread(DownloadThread, zip, AfterDownload);
	downloadThread.detach();
}