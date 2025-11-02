#include "script_merger.h"
#include "messages.h"

#include <format>
#include <fstream>

#include "PoDoFo/podofo.h"

#ifdef PODOFO_STATIC
#undef PODOFO_STATIC
#endif // !PODOFO_STATIC

#ifndef PODOFO_SHARED
#define PODOFO_SHARED
#endif // !PODOFO_SHARED

bool ScriptMerger::IsValidFile(const std::filesystem::directory_entry& file)
{
	if (!file.is_regular_file()) return false;

	std::basic_string_view ext = file.path().c_str();
	ext = ext.substr(std::max(ext.size(), (size_t)4) - 4);
	return ext.size() >= 4 && 
		ext[0] == '.' && ext[1] == 'p' &&
		ext[2] == 'd' && ext[3] == 'f';
}

std::filesystem::path ScriptMerger::ToPath(std::wstring_view path, 
	bool skip_check = false)
{
	using namespace std::filesystem;
	using namespace messages;

	if (skip_check) return path;

	bool success = true;
	while (!exists(path))
	{
		PostVoidPrompt<wchar_t>("Error while accessing the folder!");

		if (!PostBinaryPrompt<wchar_t>("Would you like to retry?"))
		{
			success = false;
			break;
		}
	}

	return success ? path : std::filesystem::path{};
}

bool ScriptMerger::CreatePathIfMissing(const std::filesystem::path& path, 
	std::wostream& os)
{
	using namespace std::filesystem;
	using namespace messages;

	while (!exists(path))
	{
		PostVoidPrompt<wchar_t>("The destination directory for merged files is missing.", os);
		if (!PostBinaryPrompt<wchar_t>("Would you like the program to try to create it?")) return false;
		create_directory(path);
	}

	return true;
}

void ScriptMerger::ParseMapFile(std::wifstream& ifs)
{
	std::wstring Id1;
	std::wstring Id2;
	std::wstring script_file_leaf;

	while (std::getline(ifs, Id1))
	{
		script_file_leaf = script_name_pattern_;

		// Tab has to be the separator
		size_t pos = Id1.find_first_of('\t');
		if (pos == std::string::npos) continue;

		Id2 = Id1.substr(pos + 1);
		Id1 = Id1.substr(0, pos);

		// Turn Id1 into the file name by inserting it at '*' in the mask.
		// If there is no '*', Id1 is simply prepended by the mask.
		pos = script_file_leaf.find_first_of('*');
		if (pos != std::string::npos)
		{
			script_file_leaf = script_file_leaf.replace(pos, 1, Id1) + L".pdf";
		}
		else script_file_leaf += std::move(Id1);

		file_map_[script_file_leaf] = Id2 + L".pdf";
	}
}

size_t ScriptMerger::GetFileTotal() const
{
	using namespace std::filesystem;
	size_t out = 0;

	for (const directory_entry& file :
		recursive_directory_iterator(scripts_dir_))
	{
		out += IsValidFile(file) ? 1 : 0;
	}

	return out;
}

void ScriptMerger::MergePDFs(const std::filesystem::path& script, 
	const std::filesystem::path& front_page, 
	std::wostream& os)
{
	using namespace std::filesystem;
	using namespace messages;

	path new_script = output_dir_ / front_page.filename();

	// Remove the merged file if it is already there
	if (exists(new_script))
	{
		while (!remove(new_script))
		{
			PostVoidPrompt<wchar_t>("Error while trying to replace the old merged file!", os);

			if (!PostBinaryPrompt<wchar_t>("Would you like to retry?", os))
			{
				PostVoidPrompt<wchar_t>("Giving up on the file...");
				return;
			}
		}
	}

	PoDoFo::PdfMemDocument old_pdf;
	PoDoFo::PdfMemDocument new_pdf;

	new_pdf.Load(front_page.string());
	old_pdf.Load(script.string());
	new_pdf.GetPages().AppendDocumentPages(old_pdf);

	new_pdf.Save(new_script.string());

	if (exists(new_script)) PostVoidPrompt<wchar_t>("File is formed and saved!", os);
	else PostVoidPrompt<wchar_t>("Unknown error while saving the file.", os);
}

ScriptMerger::ScriptMerger(std::wstring_view scripts_dir,
	std::wstring_view front_pages_dir,
	std::wstring_view output_dir,
	std::wstring_view script_name_pattern,
	std::wstring_view file_map_name)
{
	scripts_dir_ = ToPath(scripts_dir);
	if (scripts_dir_.empty()) goto MISSING_PATH;

	front_pages_dir_ = ToPath(front_pages_dir);
	if (front_pages_dir_.empty()) goto MISSING_PATH;

	output_dir_ = ToPath(output_dir, true);
	script_name_pattern_ = script_name_pattern;
	id_map_name_ = file_map_name;

	return;

MISSING_PATH:
	is_good_ = false;
}

void ScriptMerger::ReadIdMap()
{
	using namespace std::string_view_literals;
	using namespace messages;
	
	// Skip the step if there is no Id map file
	if (id_map_name_.empty()) return;

	std::wifstream ifs(L".//" + id_map_name_);

	bool success = true;
	while (!ifs.is_open())
	{
		PostVoidPrompt<wchar_t>("Error while trying to open the map file!");

		if (!PostBinaryPrompt<wchar_t>("Would you like to retry?"))
		{
			success = false;
			break;
		}
	}
	if (!success)
	{
		PostVoidPrompt<wchar_t>("Skipping the map file...");
		return;
	}

	ParseMapFile(ifs);
}

void ScriptMerger::ProcessPDFs(std::wostream& os)
{
	using namespace std::filesystem;
	using namespace messages;

	PostVoidPrompt<wchar_t>("Processing started...", os);

	// Retrieving the script total
	size_t n_files = GetFileTotal();
	PostVoidPrompt<wchar_t>(std::format(L"{0} pdf files found in the folder.", n_files), os);

	// Creating the output folder if it is missing
	if (!CreatePathIfMissing(output_dir_, os)) return;
	
	// Merging files with front pages
	size_t i = 1;
	for (const directory_entry& script : 
		recursive_directory_iterator(scripts_dir_))
	{
		if (!IsValidFile(script)) continue;

		PostVoidPrompt<wchar_t>(std::format(L"Attaching the front page to file {0} out of {1}:", 
			i, n_files), os);


		path front_page;

		if (id_map_name_.size())
		{
			std::wstring script_file_name = script.path().filename().wstring();

			IdMap::const_iterator pos = file_map_.find(script_file_name);

			if (pos != file_map_.end()) front_page = front_pages_dir_ / pos->second;
			else
			{
				PostVoidPrompt<wchar_t>("Cannot find the front page! An error in the mapping file.",
					os, true);
				continue;
			}
		}
		else front_page = script;

		if (!exists(front_page))
		{
			PostVoidPrompt<wchar_t>("Cannot find the front page! The file is missing.",
				os, true);
			continue;
		}

		// Merging pdfs
		try
		{
			PoDoFo::PdfMemDocument old_pdf;
			PoDoFo::PdfMemDocument new_pdf;

			new_pdf.Load(front_page.string());
			old_pdf.Load(script.path().string());
			new_pdf.GetPages().AppendDocumentPages(old_pdf);

			path new_script = output_dir_ / front_page.filename();
			new_pdf.Save(new_script.string());

			if (exists(new_script))
			{
				PostVoidPrompt<wchar_t>("File is formed and saved!", os);
			}
		}
		catch (const std::exception&)
		{
		}
	}
}