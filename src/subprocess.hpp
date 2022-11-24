#pragma once
#include <Windows.h>
#include <string>

namespace subprocess {

    struct PipeHandle {
        HANDLE handle;

        void set_inherit(bool value) {
            SetHandleInformation(handle, HANDLE_FLAG_INHERIT, value * HANDLE_FLAG_INHERIT);
        }

        void close() {
            CloseHandle(handle);
        }
    };

    struct PipePair {
        PipeHandle m_read;
        PipeHandle m_write;

        static PipePair create(bool inheritable) {
            SECURITY_ATTRIBUTES security = {};
            security.nLength = sizeof(security);
            security.bInheritHandle = inheritable;
            HANDLE read, write;
            CreatePipe(&read, &write, &security, 0);
            return { { read }, { write } };
        }

        void write(const void* const data, size_t size) {
            WriteFile(m_write.handle, data, size, nullptr, nullptr);
        }

        void close() {
            m_read.close();
            m_write.close();
        }
    };

    class Popen {
    public:
        PipePair m_stdin;
        PipePair m_stdout;
        PROCESS_INFORMATION m_proc_info{};
    public:
        Popen() {}
        Popen(const std::string& command) {
            STARTUPINFOW start_info = {};

            start_info.cb = sizeof(start_info);
            start_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            start_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
            start_info.dwFlags |= STARTF_USESTDHANDLES;

            m_stdin = PipePair::create(true);
            start_info.hStdInput = m_stdin.m_read.handle;
            m_stdin.m_write.set_inherit(false);

            // cant include "utils.hpp" here, so just copy paste code from myself :D
            auto size = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, nullptr, 0);
            auto buffer = new wchar_t[size];
            MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, buffer, size);

            CreateProcessW(nullptr, buffer, nullptr, nullptr, true, 0, nullptr, nullptr, &start_info, &m_proc_info);

            delete[] buffer;

            m_stdin.m_read.close();
        }

        int wait() {
            WaitForSingleObject(m_proc_info.hProcess, INFINITE);
            DWORD exit_code;
            GetExitCodeProcess(m_proc_info.hProcess, &exit_code);
            return exit_code;
        }

        int close(bool should_wait = true) {
            int exit_code = 0;
            m_stdin.close();
            m_stdout.close();
            if (should_wait) exit_code = wait();
            CloseHandle(m_proc_info.hProcess);
            CloseHandle(m_proc_info.hThread);
            return exit_code;
        }
    };
}