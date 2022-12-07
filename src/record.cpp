#include "record.hpp"
#include <sstream>
#include <stdlib.h>
#include <CCGL.h>
#include <chrono>
#include <filesystem>
#include "PlayLayer.h"
#include <fstream>
#include "bools.h";
#include "ReplayPlayer.h"
#include "Hacks.h"

extern struct HacksStr hacks;

Recorder::Recorder() : m_width(1280), m_height(720), m_fps(60) {}

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
    if (play_layer->m_level->m_nSongID == 0)
        song_file = CCFileUtils::sharedFileUtils()->fullPathForFilename(song_file.c_str(), false);
    auto is_testmode = play_layer->m_isTestMode;
    auto song_offset = m_song_start_offset;

    if (!std::filesystem::is_directory("GDMenu/renders/" + play_layer->m_level->m_sLevelName + " - " + std::to_string(play_layer->m_level->m_nLevelID)) || !std::filesystem::exists("GDMenu/renders/" + play_layer->m_level->m_sLevelName + " - " + std::to_string(play_layer->m_level->m_nLevelID)))
    {
        std::filesystem::create_directory("GDMenu/renders/" + play_layer->m_level->m_sLevelName + " - " + std::to_string(play_layer->m_level->m_nLevelID));
    }

    std::thread([&, song_file, fade_in, fade_out, bg_volume, sfx_volume, is_testmode, song_offset, play_layer]()
                {
                    char pBuf[256];
                    size_t len = sizeof(pBuf);
                    int bytes = GetModuleFileName(NULL, pBuf, len);
                    std::filesystem::path p = pBuf;
                    p = p.parent_path();

                    auto gdpath = p.string();
                    auto finalpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->m_sLevelName + " - " + std::to_string(play_layer->m_level->m_nLevelID) + "/" + play_layer->m_level->m_sLevelName + ".mp4");
                    auto notfinalpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->m_sLevelName + " - " + std::to_string(play_layer->m_level->m_nLevelID) + "/" + "rendered_video.mp4");
                    auto clickpath = (gdpath + "/GDMenu/renders/" + play_layer->m_level->m_sLevelName + " - " + std::to_string(play_layer->m_level->m_nLevelID) + "/" + "rendered_clicks.mp3");

                    std::stringstream stream;
                    stream << '"' << m_ffmpeg_path << '"' << " -y -f rawvideo -pix_fmt rgb24 -s " << m_width << "x" << m_height << " -r " << m_fps
                           << " -i - ";
                    if (!m_codec.empty())
                        stream << "-c:v " << m_codec << " ";
                    if (!m_bitrate.empty())
                        stream << "-b:v " << hacks.bitrate << " ";
                    if (!m_extra_args.empty())
                        stream << m_extra_args << " ";
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
                    if (process.close())
                    {
                        //return;
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
                        stream << '"' << m_ffmpeg_path << '"' << " -y -ss " << song_offset << " -i \"" << song_file
                               << "\" -i \"" << notfinalpath << "\" -t " << total_time << " -c:v copy ";
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
                        if (process.close())
                        {
                            return;
                        }
                    }

                    std::filesystem::remove(Hacks::widen(notfinalpath));
                    std::filesystem::rename(temp_path, Hacks::widen(notfinalpath));

                    if (!hacks.includeClicks || !hacks.recording)
                        return;

                    size_t acti = 0, actionTracker = 0, dsophji = 0, nameTracker = 0;

                    auto actions = ReplayPlayer::getInstance().GetReplay()->getActions();
                    float prevClick = 0;

                    {
                        while (actionTracker < ReplayPlayer::getInstance().GetActionsSize())
                        {
                            std::stringstream stream;
                            stream << '"' << m_ffmpeg_path << '"' << " -y";
                            acti = 0;
                            for (size_t i = 0; acti < hacks.clickSoundChunkSize; i++)
                            {
                                if (actionTracker + i >= ReplayPlayer::getInstance().GetActionsSize() || actions[actionTracker + i].dummy)
                                    continue;

                                std::string path = gdpath + "\\GDmenu\\clicks";

                                if (actions[actionTracker + i].press)
                                {
                                    (actions[actionTracker + i].frame / hacks.fps) - prevClick > hacks.playMediumClicksAt * 2.0f ? path += "\\clicks\\" + std::to_string(rand() % Hacks::amountOfClicks + 1) + ".wav" : path += "\\mediumclicks\\" + std::to_string(rand() % Hacks::amountOfMediumClicks + 1) + ".wav";
                                }
                                else
                                {
                                    path += "\\releases\\" + std::to_string(rand() % Hacks::amountOfReleases + 1) + ".wav";
                                }
                                if(actions[actionTracker + i].press) prevClick = actions[actionTracker + i].frame / hacks.fps;
                                stream << " -i " << '"' << path << '"';
                                acti++;
                            }

                            stream << " -filter_complex " << '"';

                            acti = 0;
                            for (size_t i = 0; acti < hacks.clickSoundChunkSize; i++)
                            {
                                if (actionTracker + i >= ReplayPlayer::getInstance().GetActionsSize() || actions[actionTracker + i].dummy)
                                    continue;
                                
                                float volume = ((actions[actionTracker + i].frame / hacks.fps) - prevClick) * 15.0f;
                                if(volume > 1 || volume < 0) volume = 1;
                                if(!actions[actionTracker + i].press) volume *= 2;
                                stream << '[' << acti << ":a]atrim=start=0ms:end=1000ms,adelay=" << (actions[actionTracker + i].frame / hacks.fps) * 1000 << "ms:all=1,volume=" << volume << "[a" << acti << "];";
                                acti++;
                                if(actions[actionTracker + i].press) prevClick = actions[actionTracker + i].frame / hacks.fps;
                            }

                            acti = 0;

                            for (size_t i = 0; acti < hacks.clickSoundChunkSize; i++)
                            {
                                if (actionTracker + i >= ReplayPlayer::getInstance().GetActionsSize() || actions[actionTracker + i].dummy)
                                    continue;
                                stream << "[a" << acti << ']';
                                acti++;
                            }

                            nameTracker += hacks.clickSoundChunkSize;
                            actionTracker += acti;

                            stream << "amix=inputs=" << acti << ":normalize=0";

                            stream << '"' << " -t " << (int)(total_time * 1000.0) << "ms"
                                   << " \"" << gdpath << "/rendered_clicks" << nameTracker << ".mp3";

                            auto process = subprocess::Popen(stream.str());
                            stream.clear();
                            if (process.close())
                            {
                                return;
                            }
                        }

                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(std::chrono::seconds((4 + (actions.size() / 200))));

                        std::stringstream stream;
                        stream << '"' << m_ffmpeg_path << '"' << " -y";

                        for (size_t i = hacks.clickSoundChunkSize; i <= actionTracker; i += hacks.clickSoundChunkSize)
                        {
                            stream << " -i"
                                   << " \"" << gdpath << "/rendered_clicks" << i << ".mp3"
                                   << "\"";
                        }

                        stream << " -filter_complex " << '"';

                        for (size_t i = 0; i < actionTracker / hacks.clickSoundChunkSize; i++)
                        {
                            stream << '[' << i << ":a]volume= " << hacks.renderClickVolume << "[a" << i << "];";
                        }

                        for (size_t i = 0; i < actionTracker / hacks.clickSoundChunkSize; i++)
                        {
                            stream << "[a" << i << ']';
                        }

                        stream << "amix=inputs=" << actionTracker / hacks.clickSoundChunkSize << ":duration=longest:normalize=0";

                        stream << '"' << " -t " << (int)(total_time * 1000.0) << "ms"
                               << " \"" << gdpath << "/GDMenu/renders/" << play_layer->m_level->m_sLevelName << " - " << std::to_string(play_layer->m_level->m_nLevelID) << "/rendered_clicks.mp3"
                               << "\"";

                        auto process = subprocess::Popen(stream.str());
                        if (process.close())
                        {
                            return;
                        }
                    }

                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(std::chrono::seconds((4 + (actions.size() / 200))));

                    {
                        std::stringstream f;
                        f << '"' << m_ffmpeg_path << '"' << " -y -i " << '"' << notfinalpath << '"' << " -i " << '"' << clickpath << '"' << " -filter_complex amix=inputs=2:duration=longest:weights=" << '"' << "1 0.75" << '"' << " -map 0:v -c:v copy -b:a 192k " << '"' << finalpath << '"';
                        Hacks::writeOutput(f.str());
                        auto process = subprocess::Popen(f.str());
                        if (process.close())
                        {
                            return;
                        }
                    }

                    for (size_t i = hacks.clickSoundChunkSize; i <= actionTracker; i += hacks.clickSoundChunkSize)
                    {
                        std::filesystem::remove(Hacks::widen(gdpath + "/rendered_clicks" + std::to_string(i) + ".mp3"));
                    } })
        .detach();
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
        m_texture->initWithData(data, kCCTexture2DPixelFormat_RGB888, m_width, m_height, CCSize(static_cast<float>(m_width), static_cast<float>(m_height)));
        free(data);
    }

    glGetIntegerv(GL_RENDERBUFFER_BINDING_EXT, &m_old_rbo);

    glGenFramebuffersEXT(1, &m_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_texture->getName(), 0);

    m_texture->setAliasTexParameters();

    m_texture->autorelease();

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_old_rbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_old_fbo);
}

void MyRenderTexture::capture(std::mutex &lock, std::vector<u8> &data, volatile bool &lul)
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

void Recorder::handle_recording(gd::PlayLayer *play_layer, float dt)
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
            gd::FMODAudioEngine::sharedEngine()->setMusicTime(
                play_layer->m_time + m_song_start_offset);
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

void Recorder::update_song_offset(gd::PlayLayer *play_layer)
{
    // from what i've checked rob doesnt store the timeforxpos result anywhere, so i have to calculate it again
    m_song_start_offset = play_layer->m_pLevelSettings->m_songStartOffset + play_layer->timeForXPos(
                                                                                play_layer->m_pPlayer1->m_position.x);
}