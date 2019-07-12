#pragma once

#include <fstream>

class BufferedFile : protected std::fstream
{
	static const size_t bufferSize = 1024;
	size_t currentBufferPos = 0;
	char* buffer;
public:
	BufferedFile();
	void Open(const std::string& fileName, int mode);
	void Write(const char* data, size_t dataSize);
	void Close();
	bool IsOpen() const;
	~BufferedFile();
};
