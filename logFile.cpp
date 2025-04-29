#include "pch.h"
#include "logFile.h"
#include <filesystem>
namespace fs = std::filesystem;


LogFile::LogFile(const std::string& basename): basename_(basename)
{
	// it should create a file if the file is not exist!
	fs::path logPath(basename_);
	fs::path parentDir = logPath.parent_path();
	if (!parentDir.empty() && !fs::exists(parentDir)) {
		fs::create_directories(parentDir);
	}
	stream_.open(basename_, std::ofstream::out | std::ofstream::app);
	if (!stream_){
		throw std::runtime_error("Failed to open log file: " + basename_);
	}
}

LogFile::~LogFile()
{
	stream_.close();
}

void LogFile::append(const char* data, std::size_t len)
{
	std::scoped_lock<std::mutex> lock(mutex_);
	stream_.write(data, len);
	if (!stream_)
	{
		throw std::runtime_error("Failed to write to log file: " + basename_);
	}
}

void LogFile::flush()
{
	std::scoped_lock<std::mutex> lock(mutex_);
	stream_.flush();
}

void LogFile::close()
{
	std::scoped_lock<std::mutex> lock(mutex_);
	if (stream_){
		stream_.close();
	}

}







