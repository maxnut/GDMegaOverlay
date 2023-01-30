#include "gdstring.h"

namespace gd
{
	AllocType alloc = reinterpret_cast<AllocType>(GetProcAddress(LoadLibrary("MSVCR120.DLL"), "??2@YAPAXI@Z"));
	DeallocType dealloc = reinterpret_cast<DeallocType>(GetProcAddress(LoadLibrary("MSVCR120.DLL"), "??3@YAXPAX@Z"));

	string::string()
	{
		this->m_str[0] = '\0';
		this->m_strLen = 0;
		this->m_allocLen = 0;
	}

	string::string(const char *str) : string()
	{
		this->set_str(str);
	}

	string::string(const std::string &str) : string()
	{
		this->set_str(str.c_str());
	}

	string::string(const string &str) : string()
	{
		this->set_str(str.c_str());
	}

	__declspec(noinline) string::~string()
	{
		if (this->m_allocLen >= 0x10)
		{
			dealloc(this->m_ptr);
			this->m_ptr = nullptr;
		}
	}

	string &string::operator=(const char *str)
	{
		this->set_str(str);
		return *this;
	}

	string &string::operator=(const std::string &str)
	{
		this->set_str(str.c_str());
		return *this;
	}

	string &string::operator=(const string &str)
	{
		this->set_str(str.c_str());
		return *this;
	}

	const char *string::c_str() const
	{
		return this->m_strLen < 0x10 ? this->m_str : this->m_ptr;
	}

	void string::str_alloc(std::size_t size)
	{
		if (size <= 0x10)
		{
			if (this->m_allocLen > 0)
			{
				dealloc(this->m_ptr);
				this->m_ptr = nullptr;
				this->m_allocLen = 0;
			}
		}
		else if (this->m_allocLen < size)
		{
			if (this->m_allocLen > 0)
				dealloc(this->m_ptr);
			this->m_ptr = reinterpret_cast<char *>(alloc(size));
			this->m_allocLen = size;
		}
	}

	void string::set_str(const char *str)
	{
		auto len = strlen(str);
		this->str_alloc(len + 1);

		strcpy(len < 0x10 ? this->m_str : this->m_ptr, str);
		this->m_strLen = len;
	}

}