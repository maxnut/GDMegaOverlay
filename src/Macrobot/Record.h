#pragma once

#include <cocos2d.h>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_map>
#include <subprocess.hpp>
#include <string>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace cocos2d;

using u8 = uint8_t;

class MyRenderTexture
{
public:
	unsigned m_width, m_height;
	int m_old_fbo, m_old_rbo;
	unsigned m_fbo;
	CCTexture2D* m_texture;
	void begin();
	void end();
	void capture(std::mutex& lock, std::vector<u8>& data, volatile bool& lul);
};

class Recorder
{
public:
	Recorder();
	std::vector<u8> m_current_frame;
	volatile bool m_frame_has_data;
	std::mutex m_lock;
	MyRenderTexture m_renderer;
	unsigned m_width, m_height;
	unsigned m_fps;
	bool m_recording = false;
	double m_last_frame_t, m_extra_t;
	bool m_until_end = true;
	std::string m_codec = "", m_bitrate = "30M", m_extra_args = "", m_extra_args_after = "", m_extra_audio_args = "";
	float m_after_end_duration;
	float m_after_end_extra_time;
	float m_song_start_offset;
	bool m_finished_level;
	bool m_include_audio = true;
	std::string m_ffmpeg_path = "ffmpeg";

	bool m_recording_audio = false;

	float tfx = 0;

	int sampleRate = 0;
	int numChannels = 0;
	int sampleFormat = 0;

	void start();
	void stop();
	void stop_audio();
	void capture_frame();
	void handle_recording(GJBaseGameLayer*, float dt);
	void handle_recording_audio(GJBaseGameLayer*, float dt);
};


namespace Record
{
	inline Recorder recorder;

	void renderWindow();
}