#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>

class ScriptMerger
{
private:
	using IdMap = std::unordered_map<std::wstring, std::wstring>;

	std::filesystem::path scripts_dir_;
	std::filesystem::path front_pages_dir_;
	std::filesystem::path output_dir_;
	std::wstring script_name_pattern_;
	std::wstring id_map_name_;
	IdMap file_map_;
	bool is_good_ = true;

	static bool IsValidFile(const std::filesystem::directory_entry&);
	static std::filesystem::path ToPath(std::wstring_view, 
		bool skip_check);
	static bool CreatePathIfMissing(const std::filesystem::path& path, 
		std::wostream& os);

	void ParseMapFile(std::wifstream&);
	size_t GetFileTotal() const;
	void MergePDFs(const std::filesystem::path& script,
		const std::filesystem::path& front_page, 
		std::wostream& os);

public:
	ScriptMerger() = delete;
	ScriptMerger(std::wstring_view scripts_dir, 
		std::wstring_view front_pages_dir, 
		std::wstring_view output_dir, 
		std::wstring_view script_name_pattern, 
		std::wstring_view id_map_path);
	~ScriptMerger() = default;

	bool IsGood() const { return is_good_; }

	void ReadIdMap();
	void ProcessPDFs(std::wostream& os = std::wcout);
};