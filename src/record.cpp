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
		auto clickpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
						  std::to_string(play_layer->m_level->levelID) + "/" + "rendered_clicks.wav");

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
				std::cerr << e.what() << '\n';
			}
		}

		if (!m_include_audio || !std::filesystem::exists(song_file))
			return;
		wchar_t buffer[MAX_PATH];
		if (!GetTempFileNameW(Hacks::widen(std::filesystem::temp_directory_path().string()).c_str(), L"rec", 0, buffer))
		{
			return;
		}
		auto temp_path = Hacks::narrow(buffer) + "." + std::filesystem::path(notfinalpath).filename().string();
		std::filesystem::rename(buffer, temp_path);
		auto total_time = m_last_frame_t; // 1 frame too short?

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
				std::cerr << e.what() << '\n';
			}
		}

		std::filesystem::remove(Hacks::widen(notfinalpath));
		std::filesystem::rename(temp_path, Hacks::widen(notfinalpath));

		generate_clicks();

		{
			std::stringstream f;
			f << '"' << m_ffmpeg_path << '"' << " -y -i " << '"' << notfinalpath << '"' << " -i " << '"' << clickpath
			  << '"' << " -filter_complex "
			  << "\"[0:a]volume=1.0[a0];[1:a]volume=" << hacks.renderClickVolume
			  << "[a1];[a0][a1]amix=inputs=2:duration=longest:weights=1 " << hacks.renderClickVolume << "\""
			  << " -map 0:v -c:v copy -b:a 192k " << '"' << finalpath << '"';
			auto process = subprocess::Popen(f.str());
			try
			{
				process.close();
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what() << '\n';
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

// Structure to hold the audio data
struct AudioData
{
	const char* filename;
	int16_t* samples;
	size_t numSamples;
};

// Callback function to fill the output buffer with audio data
int paCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
			   const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{

	AudioData* audioData = (AudioData*)userData;
	int16_t* out = (int16_t*)outputBuffer;

	// Check if all samples have been played
	if (audioData->numSamples == 0)
		return paComplete;

	// Copy audio samples to the output buffer
	size_t samplesToCopy =
		(size_t)framesPerBuffer < audioData->numSamples ? (size_t)framesPerBuffer : audioData->numSamples;
	std::memcpy(out, audioData->samples, samplesToCopy * sizeof(int16_t));

	// Update the sample pointer and remaining sample count
	audioData->samples += samplesToCopy;
	audioData->numSamples -= samplesToCopy;

	return paContinue;
}

//i love you chatgpt
void Recorder::generate_clicks()
{
	ReplayPlayer* rp = &ReplayPlayer::getInstance();

	char pBuf[256];
	size_t len = sizeof(pBuf);
	int bytes = GetModuleFileName(NULL, pBuf, len);
	std::filesystem::path p = pBuf;
	p = p.parent_path();

	auto gdpath = p.string();

	std::string path = gdpath + "\\GDmenu\\clicks\\";

	// Initialize PortAudio
	Pa_Initialize();

	// List of input .wav files
	std::vector<std::string> clicks, mediumclicks, releases;

	for (size_t i = 0; i < ExternData::amountOfClicks; i++)
		clicks.push_back(path + "clicks\\" + std::to_string(i) + ".wav");

	for (size_t i = 0; i < ExternData::amountOfMediumClicks; i++)
		mediumclicks.push_back(path + "mediumclicks\\" + std::to_string(i) + ".wav");

	for (size_t i = 0; i < ExternData::amountOfReleases; i++)
		releases.push_back(path + "releases\\" + std::to_string(i) + ".wav");

	// Assume common audio format for all input files
	int sampleRate = 44100;
	int numChannels = 2;
	PaSampleFormat sampleFormat = paInt16;

	auto gm = gd::GameManager::sharedState();
	auto play_layer = gm->getPlayLayer();

	// Open the output file using libsndfile
	const char* outputFile = (gdpath + "/GDMenu/renders/" + play_layer->m_level->levelName + " - " +
							  std::to_string(play_layer->m_level->levelID) + "/" + "rendered_clicks.wav")
								 .c_str();
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

	// Concatenate the input files and write to the output file
	double previousTimestamp = 0.0;

	for (const Action& ac : rp->GetReplay()->getActions())
	{
		// Open the input file using libsndfile
		AudioData audioData;

		const char* filename = (path + (ac.press ? "clicks\\" : "releases\\") +
								std::to_string(rand() % ExternData::amountOfClicks + 1) + ".wav")
								   .c_str();

		audioData.filename = filename;

		SF_INFO inputSfInfo;
		SNDFILE* inputSndFile = sf_open(filename, SFM_READ, &inputSfInfo);
		if (!inputSndFile)
		{
			std::cerr << "Failed to open input file: " << filename << std::endl;
			continue;
		}

		// Calculate the new duration of the audio file
		double duration = inputSfInfo.frames / static_cast<double>(sampleRate);

		// Calculate the new timestamp for the audio file
		double timestamp = ac.frame / rp->GetReplay()->fps;

		// Calculate the number of silent frames to insert between files
		double timeDiff = timestamp - previousTimestamp - duration;

		if (timeDiff < 0)
			timeDiff = 0;

		size_t numSilenceFrames = static_cast<size_t>(std::round(timeDiff * sampleRate));

		// Calculate the number of silent samples based on the total number of channels
		size_t numSilenceSamples = numSilenceFrames * numChannels;

		// Write silent samples to the output file
		int16_t* silenceSamples = new int16_t[numSilenceSamples];
		std::memset(silenceSamples, 0, numSilenceSamples * sizeof(int16_t));
		sf_writef_short(sndFile, silenceSamples, numSilenceFrames);
		delete[] silenceSamples;

		// Update the previous timestamp and the total time
		previousTimestamp = timestamp;

		// Update the sample rate of the input file to match the output file
		inputSfInfo.samplerate = sampleRate;

		// Create a temporary output buffer
		int16_t* outputBuffer = new int16_t[inputSfInfo.frames * inputSfInfo.channels];

		// Read audio samples into memory
		sf_readf_short(inputSndFile, outputBuffer, inputSfInfo.frames);

		sf_close(inputSndFile);

		sf_count_t frameTime = static_cast<sf_count_t>(timestamp * sampleRate);
		sf_seek(sndFile, frameTime, SEEK_SET);

		// Write audio samples to the output file
		sf_writef_short(sndFile, outputBuffer, inputSfInfo.frames);
		delete[] outputBuffer;
	}

	// Cleanup and close the output file
	sf_close(sndFile);
	Pa_Terminate();
}