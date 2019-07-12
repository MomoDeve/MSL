#include "bufferedFile.h"

BufferedFile::BufferedFile()
{
	buffer = new char[bufferSize];
}

void BufferedFile::Open(const std::string& fileName, int mode)
{
	std::fstream::open(fileName, mode);
}

void BufferedFile::Write(const char* data, size_t dataSize)
{
	if (dataSize + currentBufferPos < bufferSize)
	{
		std::memcpy(buffer + currentBufferPos, data, dataSize);
		currentBufferPos += dataSize;
		return; // push to buffer and do not write to file
	}

	if (currentBufferPos > 0) // clearing buffer before inserting data
	{
		std::fstream::write(buffer, currentBufferPos);
		currentBufferPos = 0;
	}

	if (dataSize < bufferSize) // copy to buffer
	{
		std::memcpy(buffer, data, dataSize);
		currentBufferPos = dataSize;
	}
	else // if data is enough big (greater than bufferSize), just write it to file
	{
		std::fstream::write(data, dataSize);
	}
}

void BufferedFile::Close()
{
	if (is_open())
	{
		if (currentBufferPos > 0)
		{
			std::fstream::write(buffer, currentBufferPos);
		}
		std::fstream::close();
	}
}

bool BufferedFile::IsOpen() const
{
	return is_open();
}

BufferedFile::~BufferedFile()
{
	if (is_open())
	{
		close();
	}
	delete[] buffer;
}
