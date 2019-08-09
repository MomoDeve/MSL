#pragma once

#include <fstream>

namespace MSL
{
	/*
	wrapper-class of std::fstream class with buffer for Write() method
	*/
	class BufferedFile : protected std::fstream
	{
		/*
		buffer size for Write() method
		*/
		static const size_t bufferSize = 1024;
		/*
		pos in output buffer
		*/
		size_t currentBufferPos = 0;
		/*
		pointer to buffer byte-array with size `bufferSize`
		*/
		char* buffer;
	public:
		/*
		creates empty BufferedFile object. Open() can be used to open file
		*/
		BufferedFile();
		/*
		opens file with mode and name provided
		*/
		void Open(const std::string& fileName, int mode);
		/*
		writes data as byte-array
		*/
		void Write(const char* data, size_t dataSize);
		/*
		closes file. Automatically called on object destruction if not called before
		*/
		void Close();
		/*
		checks if file is opened
		*/
		bool IsOpen() const;
		/*
		destroys object and closes opened file
		*/
		~BufferedFile();
	};
}