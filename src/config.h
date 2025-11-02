#pragma once

#include "json.h"
#include "io_traits.h"
#include "messages.h"

#include <conio.h>
#include <fstream>
#include <string>
#include <optional>

template <typename T>
struct Config
{
private:
	static std::basic_string<T> Convert(const char* chars)
	{
		return { chars, chars + strlen(chars) };
	}

public:
	// Settings - defaults are set by the constructor
	std::basic_string<T> scripts_dir{};
	std::basic_string<T> front_pages_dir{};
	std::basic_string<T> output_dir{};

	std::basic_string<T> id_map_name{};
	std::basic_string<T> script_name_pattern{};

	Config();
	~Config() = default;

	template <typename S>
	void Read(S&& s);
	void Read();

	template <typename S>
	bool Save(S&& s, std::basic_ostream<T>& tos = io::traits<T>::tcout);
	bool Save(std::basic_ostream<T>& tos = io::traits<T>::tcout);

	void Print(std::basic_ostream<T>& tos = io::traits<T>::tcout) noexcept;
	bool Update(std::basic_istream<T>& tis = io::traits<T>::tcin, 
		std::basic_ostream<T>& tos = io::traits<T>::tcout) noexcept;
};

template<typename T>
template<typename S>
void Config<T>::Read(S&& s)
{
	using namespace std::string_literals;

	// Opening the config file
	std::basic_ifstream<T> ifs(std::forward<S>(s));
	if (!ifs.is_open()) return;

	// Extracting the dictionary with options
	json::Dict<T> json_config;
	{
		// Are the options in the right format (json::Dict)?
		json::Document json_doc = json::Load(ifs);

		if (!json_doc.GetRoot().IsMap()) return;
		json_config.swap(json_doc.GetRoot().AsMap());
	}
	
	typename json::Dict<T>::const_iterator pos;

	pos = json_config.find(Convert("Scripts dir"));
	if (pos != json_config.end()) scripts_dir = pos->second.AsString();

	pos = json_config.find(Convert("Front pages dir"));
	if (pos != json_config.end()) front_pages_dir = pos->second.AsString();

	pos = json_config.find(Convert("Output dir"));
	if (pos != json_config.end()) output_dir = pos->second.AsString();

	pos = json_config.find(Convert("Map file"));
	if (pos != json_config.end()) id_map_name = pos->second.AsString();

	pos = json_config.find(Convert("Script name pattern"));
	if (pos != json_config.end()) script_name_pattern = pos->second.AsString();
}

template<typename T>
Config<T>::Config() :
	scripts_dir{ Convert(".\\Scripts") },
	front_pages_dir{ Convert(".\\Front pages") },
	output_dir{ Convert(".\\Output") }
{
}

template<typename T>
void Config<T>::Read()
{
	return Read(".\\config.json"); // ifstream() is not defined for wstring => no ""s...
}

template<typename T>
template<typename S>
bool Config<T>::Save(S&& s, 
	std::basic_ostream<T>& tos)
{
	using namespace std::string_literals;
	using namespace messages;

	if (!PostBinaryPrompt("**Would you like to save the settings?", 
		tos, true)) return false;

	json::Document<T> json_doc{ json::Dict<T>{} };
	json::Dict<T>& json_config = json_doc.GetRoot().AsMap();

	json_config[Convert("Scripts dir")] = scripts_dir;
	json_config[Convert("Front pages dir")] = front_pages_dir;
	json_config[Convert("Output dir")] = output_dir;
	json_config[Convert("Map file")] = id_map_name;
	json_config[Convert("Script name pattern")] = script_name_pattern;

	std::basic_ofstream<T> ofs(std::forward<S>(s));
	if (!ofs.is_open())
	{
		PostVoidPrompt<wchar_t>("Error while saving the configuration file!", tos);
		return false;
	}

	json_doc.Print(ofs);
	PostVoidPrompt<wchar_t>("Configuration file has been saved!", tos);
	return true;
}

template <typename T>
bool Config<T>::Save(std::basic_ostream<T>& tos)
{
	return Save(".\\config.json", tos); // basic_ofstream() is not defined for wstring => no ""s...
}

template<typename T>
void Config<T>::Print(std::basic_ostream<T>& tos) noexcept
{
	tos << "  Scripts directory = [" << scripts_dir << "]\r\n";
	tos << "  Front page directory = [" << front_pages_dir << "]\r\n";
	tos << "  Output directory = [" << output_dir << "]\r\n";
	tos << "  Map from emails to Ids = [" << id_map_name << "]\r\n";
	tos << "  Script name pattern = [" << script_name_pattern << "]\r\n\r\n";
}

template<typename T>
bool Config<T>::Update(std::basic_istream<T>& tis, 
	std::basic_ostream<T>& tos) noexcept
{
	using namespace std::string_view_literals;
	using namespace messages;

	if (!PostBinaryPrompt("Would you like to edit the settings?", tos, true))
	{
		return false;
	}

	{
		std::optional<std::basic_string<T>> maybe_val = 
			PostPathPrompt("Enter the new path to the directory with scripts",
				tis, tos, true);
		scripts_dir = maybe_val.has_value() ? *maybe_val : scripts_dir;
	}

	{
		std::optional<std::basic_string<T>> maybe_val =
			PostPathPrompt("Enter the new path to the directory with front pages", 
				tis, tos, true);
		front_pages_dir = maybe_val.has_value() ? *maybe_val : front_pages_dir;
	}

	/* String prompt rather than a path prompt, as it can be missing
	and is expected to be created then when output files are generated */
	{
		std::optional<std::basic_string<T>> maybe_val =
			PostStringPrompt("Enter the new path to the output directory",
				tis, tos, false, true);
		output_dir = maybe_val.has_value() ? *maybe_val : output_dir;
	}

	{
		std::optional<std::basic_string<T>> maybe_val =
			PostStringPrompt("Enter the new name of the mapping file",
				tis, tos, true, true);
		id_map_name = (maybe_val.has_value()) ? *maybe_val : id_map_name;
	}

	{
		std::optional<std::basic_string<T>> maybe_val =
			PostStringPrompt("Enter the new naming mask for scripts",
				tis, tos, true, true);
		script_name_pattern = (maybe_val.has_value()) ? *maybe_val : script_name_pattern;
	}

	return true;
}