#pragma once

#include "io_traits.h"

#include <iostream>
#include <string>
#include <conio.h>

#include <filesystem>
#include <optional>

namespace messages
{
	template <typename T, typename Str>
	inline void PostVoidPrompt(const Str& message, 
		std::basic_ostream<T>& tos = io::traits<T>::tcout, 
		bool add_skip = false)
	{
		tos << "**" << message << (add_skip ? "\r\n\r\n" : "\r\n");
	}

	template <typename T, typename Str>
	inline bool PostBinaryPrompt(const Str& message,
		std::basic_ostream<T>& tos = io::traits<T>::tcout, 
		bool add_skip = false)
	{
		tos << "**" << message << "\r\n";
		tos << "Press Y/y/Enter for Yes: ";

		T response = _getch();

		switch (response)
		{
		case 'Y':
		case 'y':
		case '\r':
			tos << (add_skip ? "Yes\r\n\r\n" : "Yes\r\n");
			return true;

		default:
			tos << (add_skip ? "No\r\n\r\n" : "No\r\n");
			return false;
		}
	}

	template <typename T, typename Str>
	inline std::optional<std::basic_string<T>>
		PostStringPrompt(const Str& message,
			std::basic_istream<T>& tis = io::traits<T>::tcin,
			std::basic_ostream<T>& tos = io::traits<T>::tcout,
			bool can_be_empty = false,
			bool add_skip = false)
	{
		std::basic_string<T> out;

		tos << "**" << message << "\r\n";

		do
		{
			tos << "Type the value here"
				<< (!can_be_empty ? " (Tab + Enter to skip, otherwise cannot be empty): " : "(Tab + Enter to skip): ");

			std::getline(tis, out);
			tos << (add_skip ? "\r\n" : "");

			if (out.size() == 1 &&
				out[0] == '\t') return std::nullopt;
			if (can_be_empty) return out;
			if (out.size()) return out;

			tos << (!add_skip ? "\r\n" : "");
			PostVoidPrompt<T>("This parameter cannot be empty!");
		} while (true);

		return out;
	}

	template <typename T, typename Str>
	static std::optional<std::basic_string<T>>
		PostPathPrompt(const Str& message,
			std::basic_istream<T>& tis = io::traits<T>::tcin,
			std::basic_ostream<T>& tos = io::traits<T>::tcout, 
			bool add_skip = false)
	{
		std::optional<std::basic_string<T>> out;

		while (true)
		{
			out = PostStringPrompt(message, tis, tos, false, add_skip); // cannot be empty
			if (!out.has_value()) break;
			if (std::filesystem::exists(*out)) break;

			tos << (!add_skip ? "\r\n" : "");
			if (!PostBinaryPrompt("The directory does not exist! Would you like to retry?",
				tos, add_skip)) return std::nullopt;
			else tos << (!add_skip ? "\r\n" : "");
		}

		return out;
	}
}