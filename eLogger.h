#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef LOGGER_EXPORTS
#define LOGGER_API __declspec(dllexport)
#else
#define LOGGER_API __declspec(dllimport)
#endif
#else
#define LOGGER_API __attribute__((visibility("default")))
#endif

#include "nonCopyable.h"
#include <string>
#include <memory>
#include <mutex>
#include "logFile.h"
#include <thread>
#include <vector>

template <int SIZE>
class FixedBuffer {
	// use fixed size buffer to store data
	// use pointer to manage current pos and append data
public:
	FixedBuffer() : cur_(data_) {}
	~FixedBuffer() {}
	void append(const char* buf, size_t len) {
		// if we still have space then add it to the end
		if (available() > len) {
			memcpy(cur_, buf, len);
			cur_ += len;
			*cur_ = '\0';
		}
	}
	int available() { return static_cast<int>(end() - cur_); }
	int length() const { return static_cast<int>(cur_ - data_); }
	char* current() { return cur_; }
	void reset() { cur_ = data_; }
	void add(size_t len) { cur_ += len; }
	char* data() { return data_; }
	const char* data() const { return data_; }
private:
	char* cur_;
	char data_[SIZE];
	const char* end() const { return data_ + sizeof data_; }
};

class LogStream : nonCopyable {
public:
	typedef FixedBuffer<4096> Buffer;
	LogStream& operator<<(const char* str) {
		if (str) {
			buffer_.append(str, strlen(str));
		}
		else {
			buffer_.append("(null)", 6);
		}
		return *this;
	}

	LogStream& operator<<(const std::string& str) {
		buffer_.append(str.c_str(), str.size());
		return *this;
	}

	void append(const char* data, int len) {
		buffer_.append(data, len);
	}

	const Buffer& buffer() const
	{
		return buffer_;
	}

	void resetBuffer()
	{
		buffer_.reset();
	}

private:
	Buffer buffer_;
};


class CountDownLatch {
public:
	explicit CountDownLatch(int count) : count_(count) {}
	// if it is not zero we wait!
	void wait() {
		std::unique_lock<std::mutex> lock(mutex_);
		if (count_ > 0) {
			cond_.wait(lock);
		}
	}

	void countDown() {
		std::unique_lock<std::mutex> lock(mutex_);
		if (count_ > 0) {
			count_--;
			if (count_ == 0) {
				cond_.notify_all();
			}
		}
	}

	int getCount() const {
		std::unique_lock<std::mutex> lock(mutex_);
		return count_;
	}

private:
	mutable std::mutex mutex_;
	std::condition_variable cond_;
	int count_;
};


class AsyncLogging : nonCopyable
{
public:
	AsyncLogging(const std::string& basename, int flushInterval = 2);
	~AsyncLogging();
	void Append(const char* logline, int len);
	void Stop();
	void Start();
private:
	typedef FixedBuffer<1024> Buffer;
	typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
	typedef std::shared_ptr<Buffer> BufferPtr;
	void threadFunc();
	const int flushInterval_;
	bool running_;
	const std::string basename_;
	std::thread thread_;
	std::condition_variable cond_;
	std::mutex mutex_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
	CountDownLatch latch_;
};

class LOGGER_API Logger {
public:
	Logger(const std::string& name, int flushIntervalsec = 2, bool displayLog = false);
	~Logger();
	void LogInfo(const std::string& message);
	void LogError(const std::string& message);
	void LogWarning(const std::string& message);
	void LogDebug(const std::string& message);
	void LogGeneral(const std::string& message);
	void LogCritical(const std::string& message);


private:
	AsyncLogging asyncLog_;
	std::string GetCurrentTimeFormatted() const;
	bool displayLog_;
};