#include "record.hpp"
#include "Hacks.h"
#include "PlayLayer.h"
#include "ReplayPlayer.h"
#include "bools.h"
#include "portaudio.h"
#include "sndfile.h"
#include <CCGL.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>

extern struct HacksStr hacks;

Recorder::Recorder() : m_width(1280), m_height(720), m_fps(60)
{
}

void Recorder::start()
{
	/*static bool has_console = false;
	if (!has_console)
	{
		has_console = true;
		AllocConsole();
		static std::ofstream conout("CONOUT$", std::ios::out);
		std::cout.rdbuf(conout.rdbuf());
	}*/
	m_width = hacks.videoDimenstions[0];
	m_height = hacks.videoDimenstions[1];
	m_fps = hacks.videoFps;
	if (hacks.codec != "")
		m_codec = hacks.codec;
	if (hacks.extraArgs != "")
		m_extra_args = hacks.extraArgs;
	if (hacks.extraArgsAfter != "")
		m_extra_args_after = hacks.extraArgsAfter;
	m_recording = true;
	m_frame_has_data = false;
	m_current_frame.resize(m_width * m_height * 3, 0);
	m_finished_level = false;
	m_last_frame_t = m_extra_t = 0;
	m_after_end_extra_time = 0.f;
	m_after_end_duration = hacks.afterEndDuration;
	m_renderer.m_width = m_width;
	m_renderer.m_height = m_height;
	m_renderer.begin();
	auto gm = gd::GameManager::sharedState();
	auto play_layer = gm->getPlayLayer();
	PlayLayer::resetLevelHook(play_layer, 0);
	auto song_file = play_layer->m_level->getAudioFileName();
	auto fade_in = play_layer->m_pLevelSettings->m_fadeIn;
	auto fade_out = play_layer->m_pLevelSettings->m_fadeOut;
	auto bg_volume = gm->m_fBackgroundMusicVolume;
	auto sfx_volume = gm->m_fEffectsVolume;
	if (play_layer->m_level->songID == 0)
		song_file = CCFileUtils::sharedFileUtils()->fullPathForFilename(song_file.c_str(), false);
	auto is_testmode = play_layer->m_isTestMode;
	auto song_offset = m_song_start_offset;

	if (!std::filesystem::is_directory("GDMenu/renders/" + play_layer->m_level->levelName + " - " +
									   std::to_string(play_layer->m_level->levelID)) ||
		!std::filesystem::exists("GDMenu/renders/" + play_layer->m_level->levelName + " - " +
								 std::to_string(play_layer->m_level->levelID)))
	{
		std::filesystem::create_directory("GDMenu/renders/" + play_layer->m_level->levelName + " - " +
										  std::to_string(play_layer->m_level->levelID));
	}

	std::thread([&, song_file, fade_in, fade_out, bg_volume, sfx_volume, is_testmode, song_offset, play_layer]() {
		char pBuf[256];
		size_t len = sizeof(pBuf);
		int bytes = GetModuleFileName(NULL, pBuf, len);
		std::filesystem::path p = pBuf;
		p = p.parent_path();

		auto gdpath = p.string();
		auto finalpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
						  std::to_string(play_layer->m_level->levelID) + "/" + play_layer->m_level->levelName + ".mp4");
		auto notfinalpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
							 std::to_string(play_layer->m_level->levelID) + "/" + "rendered_video.mp4");
		auto notfinalpath2 = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
							  std::to_string(play_layer->m_level->levelID) + "/" + "rendered_video_noise.mp4");
		auto clickpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
						  std::to_string(play_layer->m_level->levelID) + "/" + "rendered_clicks.wav");

		auto noisepath = gdpath + "/GDMenu/clickpacks/" + ExternData::clickpacks[hacks.currentClickpack] + "/noise.wav";

		{
			std::stringstream stream;
			stream << '"' << m_ffmpeg_path << '"' << " -y -f rawvideo -pix_fmt rgb24 -s " << m_width << "x" << m_height
				   << " -r " << m_fps;
			if (!m_extra_args.empty())
			{
				stream << " " << m_extra_args;
			}

			stream << " -i - ";
			if (!m_codec.empty())
				stream << "-c:v " << m_codec << " ";
			if (!m_bitrate.empty())
				stream << "-b:v " << hacks.bitrate << " ";
			if (!m_extra_args_after.empty())
				stream << m_extra_args_after << " ";
			else
				stream << "-pix_fmt yuv420p ";

			stream << "-vf \"vflip\" -an \"" << notfinalpath << "\" "; // i hope just putting it in "" escapes it
			auto process = subprocess::Popen(stream.str());
			while (m_recording || m_frame_has_data)
			{
				m_lock.lock();
				if (m_frame_has_data)
				{
					const auto frame = m_current_frame; // copy it
					m_frame_has_data = false;
					m_lock.unlock();
					process.m_stdin.write(frame.data(), frame.size());
				}
				else
					m_lock.unlock();
			}

			try
			{
				process.close();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << '\n';
			}
		}

		wchar_t buffer[MAX_PATH];
		if (!GetTempFileNameW(Hacks::widen(std::filesystem::temp_directory_path().string()).c_str(), L"rec", 0, buffer))
		{
			return;
		}
		auto temp_path = Hacks::narrow(buffer) + "." + std::filesystem::path(notfinalpath).filename().string();
		std::filesystem::rename(buffer, temp_path);
		auto total_time = m_last_frame_t; // 1 frame too short?

		if (m_include_audio && std::filesystem::exists(song_file))
		{
			std::stringstream stream;
			stream << '"' << m_ffmpeg_path << '"' << " -y -ss " << song_offset << " -i \"" << song_file << "\" -i \""
				   << notfinalpath << "\" -t " << total_time << " -c:v copy ";
			if (!m_extra_audio_args.empty())
				stream << m_extra_audio_args << " ";
			stream << "-filter:a \"volume=" << hacks.renderMusicVolume << "[0:a]";
			if (fade_in && !is_testmode)
				stream << ";[0:a]afade=t=in:d=2[0:a]";
			if (fade_out && m_finished_level)
				stream << ";[0:a]afade=t=out:d=2:st=" << (total_time - m_after_end_duration - 3.5f) << "[0:a]";
			std::cout << "in " << fade_in << " out " << fade_out << std::endl;
			stream << "\" \"" << temp_path << "\"";
			auto process = subprocess::Popen(stream.str());

			try
			{
				process.close();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << '\n';
			}
		}

		std::filesystem::remove(Hacks::widen(notfinalpath));
		std::filesystem::rename(temp_path, Hacks::widen(notfinalpath));

		if (hacks.includeNoise)
		{
			std::stringstream f;
			f << '"' << m_ffmpeg_path << '"' << " -y -i " << '"' << notfinalpath << '"' << " -stream_loop -1 -i " << '"'
			  << noisepath << '"' << " -filter_complex "
			  << "\"[0:a]volume=1.0[a0];[1:a]volume=" << hacks.renderNoiseVolume * 10.0f
			  << "[a1];[a0][a1]amix=inputs=2:duration=shortest\""
			  << " -map 0:v -c:v copy -b:a 192k " << '"' << notfinalpath2 << '"';
			auto process = subprocess::Popen(f.str());
			try
			{
				process.close();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << '\n';
			}
			std::filesystem::remove(Hacks::widen(notfinalpath));
			std::filesystem::rename(notfinalpath2, Hacks::widen(notfinalpath));
		}

		if (!hacks.includeClicks)
			return;

		generate_clicks();

		{
			std::stringstream f;
			f << '"' << m_ffmpeg_path << '"' << " -y -i " << '"' << notfinalpath << '"' << " -i " << '"' << clickpath
			  << '"' << " -filter_complex "
			  << "\"[0:a]volume=1.0[a0];[1:a]volume=" << hacks.renderClickVolume
			  << "[a1];[a0][a1]amix=inputs=2:duration=longest\""
			  << " -map 0:v -c:v copy -b:a 192k " << '"' << finalpath << '"';
			auto process = subprocess::Popen(f.str());
			try
			{
				process.close();
			}
			catch (const std::exception& e)
			{
				std::cout << e.what() << '\n';
			}
		}
	}).detach();
}

void Recorder::stop()
{
	m_renderer.end();
	m_recording = false;
}

void MyRenderTexture::begin()
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);

	m_texture = new CCTexture2D;
	{
		auto data = malloc(m_width * m_height * 3);
		memset(data, 0, m_width * m_height * 3);
		m_texture->initWithData(data, kCCTexture2DPixelFormat_RGB888, m_width, m_height,
								CCSize(static_cast<float>(m_width), static_cast<float>(m_height)));
		free(data);
	}

	glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &m_old_rbo);

	glGenFramebuffersEXT(1, &m_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->getName(), 0);

	m_texture->setAliasTexParameters();

	// m_texture->autorelease();

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_old_rbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
}

void MyRenderTexture::capture(std::mutex& lock, std::vector<u8>& data, volatile bool& lul)
{
	glViewport(0, 0, m_width, m_height);

	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &m_old_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

	auto director = CCDirector::sharedDirector();
	auto scene = director->getRunningScene();
	scene->visit();

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	lock.lock();
	lul = true;
	glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
	lock.unlock();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
	director->setViewport();
}

void MyRenderTexture::end()
{
	m_texture->release();
}

void Recorder::capture_frame()
{
	while (m_frame_has_data)
	{
	}
	m_renderer.capture(m_lock, m_current_frame, m_frame_has_data);
}

void Recorder::handle_recording(gd::PlayLayer* play_layer, float dt)
{
	if (!play_layer->m_hasLevelCompleteMenu || m_after_end_extra_time < m_after_end_duration)
	{
		if (play_layer->m_hasLevelCompleteMenu)
		{
			m_after_end_extra_time += dt;
			m_finished_level = true;
		}
		auto frame_dt = 1. / static_cast<double>(m_fps);
		auto time = play_layer->m_time + m_extra_t - m_last_frame_t;
		if (time >= frame_dt)
		{
			if (!play_layer->m_hasCompletedLevel)
			{
				float f = play_layer->timeForXPos(play_layer->m_pPlayer1->getPositionX());
				float offset = play_layer->m_pLevelSettings->m_songStartOffset * 1000;
				gd::FMODAudioEngine::sharedEngine()->m_pGlobalChannel->setPosition(
					static_cast<uint32_t>(f * 1000) + static_cast<uint32_t>(offset), FMOD_TIMEUNIT_MS);
			}
			m_extra_t = time - frame_dt;
			m_last_frame_t = play_layer->m_time;
			capture_frame();
		}
	}
	else
	{
		stop();
	}
}

void Recorder::update_song_offset(gd::PlayLayer* play_layer)
{
	// from what i've checked rob doesnt store the timeforxpos result anywhere, so i have to calculate it again
	m_song_start_offset =
		play_layer->m_pLevelSettings->m_songStartOffset + play_layer->timeForXPos(play_layer->m_pPlayer1->m_position.x);
}

void Recorder::generate_clicks()
{
	ReplayPlayer* rp = &ReplayPlayer::getInstance();

	char pBuf[256];
	size_t len = sizeof(pBuf);
	int bytes = GetModuleFileName(NULL, pBuf, len);
	std::filesystem::path p = pBuf;
	p = p.parent_path();

	auto gdpath = p.string();

	std::string path = gdpath + "/GDMenu/clickpacks/" + ExternData::clickpacks[hacks.currentClickpack];

	Pa_Initialize();

	int sampleRate = 0;
	int numChannels = 0;
	PaSampleFormat sampleFormat = 0;

	for (size_t i = 1; i <= ExternData::amountOfClicks; i++)
	{
		const char* filenameDummy = (path + "\\clicks\\" + std::to_string(i) + ".wav").c_str();

		SF_INFO inputSfInfoDummy;
		SNDFILE* inputSndFileDummy = sf_open(filenameDummy, SFM_READ, &inputSfInfoDummy);

		if (sampleRate != 0 && inputSfInfoDummy.samplerate != sampleRate)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same sample rate! clicks/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		if (numChannels != 0 && inputSfInfoDummy.channels != numChannels)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same channel number! clicks/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		if (sampleFormat != 0 && inputSfInfoDummy.format != sampleFormat)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same sample format! clicks/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		sampleRate = inputSfInfoDummy.samplerate;
		numChannels = inputSfInfoDummy.channels;
		sampleFormat = inputSfInfoDummy.format;

		sf_close(inputSndFileDummy);
	}

	for (size_t i = 1; i <= ExternData::amountOfSoftClicks; i++)
	{
		const char* filenameDummy = (path + "\\softclicks\\" + std::to_string(i) + ".wav").c_str();

		SF_INFO inputSfInfoDummy;
		SNDFILE* inputSndFileDummy = sf_open(filenameDummy, SFM_READ, &inputSfInfoDummy);

		if (sampleRate != 0 && inputSfInfoDummy.samplerate != sampleRate)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same sample rate! softclicks/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		if (numChannels != 0 && inputSfInfoDummy.channels != numChannels)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same channel number! softclicks/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		if (sampleFormat != 0 && inputSfInfoDummy.format != sampleFormat)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same sample format! softclicks/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		sampleRate = inputSfInfoDummy.samplerate;
		numChannels = inputSfInfoDummy.channels;
		sampleFormat = inputSfInfoDummy.format;

		sf_close(inputSndFileDummy);
	}

	for (size_t i = 1; i <= ExternData::amountOfReleases; i++)
	{
		const char* filenameDummy = (path + "\\releases\\" + std::to_string(i) + ".wav").c_str();

		SF_INFO inputSfInfoDummy;
		SNDFILE* inputSndFileDummy = sf_open(filenameDummy, SFM_READ, &inputSfInfoDummy);

		if (sampleRate != 0 && inputSfInfoDummy.samplerate != sampleRate)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same sample rate! releases/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		if (numChannels != 0 && inputSfInfoDummy.channels != numChannels)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same channel number! releases/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		if (sampleFormat != 0 && inputSfInfoDummy.format != sampleFormat)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 "Make sure all of your files have the same sample format! releases/" +
										 std::to_string(i) + ".wav")
				->show();
			sf_close(inputSndFileDummy);
			return;
		}

		sampleRate = inputSfInfoDummy.samplerate;
		numChannels = inputSfInfoDummy.channels;
		sampleFormat = inputSfInfoDummy.format;

		sf_close(inputSndFileDummy);
	}

	auto gm = gd::GameManager::sharedState();
	auto play_layer = gm->getPlayLayer();

	const char* outputFile = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
							  std::to_string(play_layer->m_level->levelID) + "/" + "rendered_clicks.wav")
								 .c_str();

	std::filesystem::create_directories(std::filesystem::path(outputFile).parent_path());

	SF_INFO sfInfo;
	sfInfo.samplerate = sampleRate;
	sfInfo.channels = numChannels;
	sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	SNDFILE* sndFile = sf_open(outputFile, SFM_WRITE, &sfInfo);
	if (!sndFile)
	{
		std::cerr << "Failed to create output file: " << outputFile << std::endl;
		return;
	}

	double previousTimestamp = 0.0;

	std::vector<int16_t> mixedBuffer;

	for (const Action& ac : rp->GetReplay()->getActions())
	{
		double timestamp = ac.frame / rp->GetReplay()->fps;

		int amt = ac.press ? (timestamp - previousTimestamp <= hacks.playSoftClicksAt ? ExternData::amountOfSoftClicks
																					  : ExternData::amountOfClicks)
						   : ExternData::amountOfReleases;

		const char* filename =
			(path +
			 (ac.press ? (timestamp - previousTimestamp <= hacks.playSoftClicksAt ? "\\softclicks\\" : "\\clicks\\")
					   : "\\releases\\") +
			 std::to_string(rand() % amt + 1) + ".wav")
				.c_str();

		SF_INFO inputSfInfo;
		SNDFILE* inputSndFile = sf_open(filename, SFM_READ, &inputSfInfo);
		if (!inputSndFile)
		{
			std::cerr << "Failed to open input file: " << filename << std::endl;
			continue;
		}

		double duration = inputSfInfo.frames / static_cast<double>(sampleRate);

		double timeDiff = timestamp - previousTimestamp - duration;

		if (timeDiff < 0)
			timeDiff = 0;

		previousTimestamp = timestamp;

		inputSfInfo.samplerate = sampleRate;

		int16_t* outputBuffer = new int16_t[inputSfInfo.frames * inputSfInfo.channels];

		sf_readf_short(inputSndFile, outputBuffer, inputSfInfo.frames);

		sf_close(inputSndFile);

		long long frameTime = static_cast<long long>(timestamp * sampleRate * numChannels);

		if (mixedBuffer.size() < frameTime + (inputSfInfo.frames * inputSfInfo.channels))
			mixedBuffer.resize(frameTime + (inputSfInfo.frames * inputSfInfo.channels), 0);

		for (long long i = 0; i < inputSfInfo.frames * inputSfInfo.channels; ++i)
		{
			mixedBuffer[frameTime + i] += outputBuffer[i];
		}

		delete[] outputBuffer;
	}

	sf_writef_short(sndFile, mixedBuffer.data(), mixedBuffer.size() / numChannels);

	sf_close(sndFile);
	Pa_Terminate();
}