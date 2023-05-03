#pragma once
#include <functional>
#include <string>
#include <vector>

namespace GDMO
{
    void addImGuiFunc(std::function<void()> func);

    void ImBegin(const char* name);
    void ImEnd();
    void ImText(const char* text);

    void Marker(const char* marker, const char* desc);
    bool ImHotkey(const char* label, int* k);
    bool ImCheckbox(const char* label, bool* v, bool canMakeShortcut = true);
    bool ImButton(const char* label, bool canMakeShortcut = true);
    bool ImInputFloat(const char* label, float* v);
    bool ImInputInt(const char* label, int* v, int step = 1);
    bool ImInputInt2(const char* label, int* v);
    bool ImInputText(const char* label, char* buf, size_t buf_size);
    bool ImCombo(const char* label, int* current_item, const char* const* items, int items_count);
    bool ImColorEdit3(const char* label, float* col, int flags);

    template <class T> void WriteRef(uint32_t vaddress, const T& value);
    template <class T> bool Write(uint32_t vaddress, const T& value);
    std::vector<uint8_t> ReadBytes(uint32_t vaddress, size_t size);
    template <class T> T Read(uint32_t vaddress);
    bool writeBytes(std::uintptr_t const address, std::vector<uint8_t> const& bytes);
    void writeOutput(std::string out);
    void writeOutput(int out);
    void writeOutput(float out);
    void writeOutput(double out);
};