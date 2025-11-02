#ifdef PODOFO_STATIC
#undef PODOFO_STATIC
#endif // !PODOFO_STATIC

#ifndef PODOFO_SHARED
#define PODOFO_SHARED
#endif // !PODOFO_SHARED

#include "config.h"
#include "messages.h"
#include "script_merger.h"

#include <iostream>

// #include "..\..\external\PoDoFo\headers\PoDoFo\podofo.h"

// Routine for populating the mapping from one set of Ids to another.
/*std::unordered_map<std::wstring, std::wstring>
	ReadMap(std::wifstream& file, std::wstring_view mask)
{
	using namespace std;

	unordered_map<wstring, wstring> out;
	wstring Id1;
	wstring Id2;
	wstring entry;

	while (std::getwline(file, Id1))
	{
		entry = mask;

		size_t pos = Id1.find_first_of('\t');
		if (pos == std::string::npos) continue;

		Id2 = Id1.substr(pos + 1);
		Id1 = Id1.substr(0, pos);

		// Turn Id1 into the file name by inserting it at '*' in the mask.
		// If there is no '*', Id1 is prepended by the mask.
		pos = entry.find_first_of('*');
		if (pos != wstring::npos)
		{
			entry = entry.replace(pos, 1, Id1) + L".pdf";
		}
		else entry += Id1;

		out[entry] = Id2;
	}

	return out;
}*/

Config<wchar_t> config;

int wmain()
{
	using namespace std::string_view_literals;
	using namespace messages;

	// Reading the config file, filling in defaults if missing.
	config.Read();

	PostVoidPrompt<wchar_t>("Current settings");
	config.Print();

	if (config.Update())
	{
		PostVoidPrompt<wchar_t>("New settings");
		config.Print();
		config.Save();
	}

	ScriptMerger script_merger(config.scripts_dir, 
		config.front_pages_dir, config.output_dir, 
		config.script_name_pattern, config.id_map_name);
	if (!script_merger.IsGood()) return EXIT_FAILURE;

	// Mapping emails to student Ids
	script_merger.ReadIdMap();

	// Processing PDFs
	script_merger.ProcessPDFs();

	/*std::wstring_view mask = config["Script name pattern"];
	bool use_mask = !mask.empty() && !(mask.find_first_of('*') == std::string::npos);

	// Merging scripts with title pages.
	int i = 0; std::wstring_view scFileName; std::wstring front_page_name;
	for (const std::filesystem::path& file : std::filesystem::directory_iterator(scripts_path))
	{
		if (std::filesystem::is_regular_file(file) && 
			file.extension() == ".pdf")
		{

			if (use_mapping)
			{
				if (emails_to_ids.count(scFileName) > 0)
					front_page_name = emails_to_ids.at(scFileName) + L".pdf";
				else {}
			}
			else front_page_name = scFileName;

			if (!std::filesystem::exists(front_pages_path / front_page_name))
			{
				PostMessage("Missing the front page file!"sv, "  "sv);
				continue;
			}

			// Delete the output file if it is already there
			success = true;
			std::filesystem::path merged_file_name = output_path / front_page_name;
			if (std::filesystem::exists(merged_file_name))
			{
				while (!std::filesystem::remove(merged_file_name))
				{
					PostMessage("Error while trying to replace the old merged file!"sv, "  "sv);

					if (!PostMessageYN("Would you like to retry?"sv, "  "sv))
					{
						success = false;
						break;
					}
				}
			}

			// Skipping on to the next one if saving was not successful
			if (!success)
			{
				PostMessage("Giving up on the file..."sv, "  "sv);
				continue;
			}

			// Merging pdfs here
			PoDoFo::PdfMemDocument pdf_in;
			PoDoFo::PdfMemDocument pdf_out;

			pdf_out.Load((front_pages_path / front_page_name).string());
			pdf_in.Load((scripts_path / scFileName).string());

			pdf_out.GetPages().AppendDocumentPages(pdf_in);
			pdf_out.Save((output_path / front_page_name).string());

			if (std::filesystem::exists(front_pages_path / front_page_name))
			{
				PostMessage("File is formed and saved!"sv, "  "sv);
			}
		}
	}

	PostMessage(""sv);*/
	std::cout << "Done!\r\n"sv;
	std::cout << "Please press any key to exit..."sv;
	_getch();

	return EXIT_SUCCESS;
}