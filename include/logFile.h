#pragma once
#include <fstream>
#include <mutex>

class LogFile {
public:
	explicit LogFile(const std::string& basename);
	~LogFile();
	void append(const char* data, std::size_t len);
	void flush();
	void close();
private:
	std::string basename_;
	std::ofstream stream_;
	std::mutex mutex_;
};