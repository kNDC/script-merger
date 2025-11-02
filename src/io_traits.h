#pragma once
#include <iostream>

namespace
{
	namespace io
	{
		template <typename T>
		struct traits
		{
			static std::basic_istream<T>& tcin;
			static std::basic_ostream<T>& tcout;
			static std::basic_ostream<T>& tcerr;
			static std::basic_ostream<T>& tclog;
		};

		template<> std::istream& traits<char>::tcin = std::cin;
		template<> std::ostream& traits<char>::tcout = std::cout;
		template<> std::ostream& traits<char>::tcerr = std::cerr;
		template<> std::ostream& traits<char>::tclog = std::clog;

		template<> std::wistream& traits<wchar_t>::tcin = std::wcin;
		template<> std::wostream& traits<wchar_t>::tcout = std::wcout;
		template<> std::wostream& traits<wchar_t>::tcerr = std::wcerr;
		template<> std::wostream& traits<wchar_t>::tclog = std::wclog;
	}
}