#pragma once

#include "pch.h"
#include <string>

namespace gd
{
	typedef void *(__cdecl *AllocType)(unsigned int);
	typedef void(__cdecl *DeallocType)(void *);

	extern AllocType alloc;
	extern DeallocType dealloc;

	class string
	{
	public:
		string();
		string(const char *);
		string(const std::string &);
		string(const string &);

		__declspec(noinline) ~string();

		string &operator=(const char *);
		string &operator=(const std::string &);
		string &operator=(const string &);

		const char *c_str() const;

	protected:
		void str_alloc(std::size_t);
		void set_str(const char *);

		union
		{
			char m_str[0x10];
			char *m_ptr;
		};

		size_t m_strLen, m_allocLen;
	};
};
