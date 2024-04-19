#pragma once

#include "pch.h"

#include <filesystem>

namespace FileSystem {
	namespace fs = std::filesystem;
	std::string ReadFileFromDisc(const std::string& filepath)
	{

		std::ifstream file (filepath);

		if (!file.is_open()) {
			Console::Warn("Could Not Open File: ", filepath);
			return "";
		}

		std::stringstream data;

		data << file.rdbuf();

		file.close();

		return data.str();
	}

	void CreateIFFNoneExist(const std::string& filepath, const std::string& contents="") {

		auto directory = filepath.substr(0, filepath.find_last_of('/'));

		if (!fs::exists(directory)) {
			if (!fs::create_directories(directory)) {
				Console::Error("Could Not Create Directory! ", directory);
				return;
			}

			Console::Info("Created Directory: ", directory);
		}

		if (!fs::exists(filepath)) {
			std::ofstream file(filepath);
			if (file.is_open()) {
				file << contents;
				file.close();
			}
		}
	}

	void WriteToFile(const std::string& filepath, const std::string& contents) 
	{
		auto directory = filepath.substr(0, filepath.find_last_of('/'));

		if (!fs::exists(directory)) {
			if (!fs::create_directories(directory)) {
				Console::Error("Could Not Create Directory! ", directory);
				return;
			}

			Console::Info("Created Directory: ", directory);
		}

		if (!fs::exists(filepath)) {
			std::ofstream file(filepath);
			if (file.is_open()) {
				file << contents;
				file.close();
			}
		}
		else {
			std::ofstream file(filepath);
			if (file.is_open()) {
				file.clear();
				file << contents;
				file.close();
			}
		}
	}
}