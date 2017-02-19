#include "IOManager.h"

#include <fstream>

namespace Engine {

	bool IOManager::readFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer) {
		std::ifstream file(filePath, std::ios::binary);
		if (file.fail()) {
			perror(filePath.c_str());
			return false;
		}

		file.seekg(0, std::ios::end);

		int fileSize = file.tellg();

		file.seekg(0, std::ios::beg);

		fileSize -= file.tellg(); // reduce file size by any header bytes

		buffer.resize(fileSize);
		file.read((char*)&(buffer[0]), fileSize);
		file.close();

		return true;
	}

	bool IOManager::readFileToBuffer(std::string filePath, std::string& buffer) {
		
		std::ifstream file(filePath, std::ios::binary);
		if (file.fail()) {
			perror(filePath.c_str());
			return false;
		}

		file.seekg(0, std::ios::end);

		int fileSize = file.tellg();

		file.seekg(0, std::ios::beg);

		fileSize -= file.tellg(); // reduce file size by any header bytes

		buffer.resize(fileSize);
		file.read((char*)&(buffer[0]), fileSize);
		file.close();

		return true;
	}

}