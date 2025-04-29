#include "pch.h"
#include "eLogger.h"
#include <sstream>
#include <iomanip>

Logger::Logger(const std::string& name, int flushIntervalsec, bool displayLog): asyncLog_(name, flushIntervalsec), displayLog_(displayLog)
{
	asyncLog_.Start();
}

Logger::~Logger()
{
	asyncLog_.Stop();
}

//log them in format [TIME] [THREAD_ID] [LEVEL] message
void Logger::LogInfo(const std::string& message)
{
	std::ostringstream oss;
	std::string currentTime = GetCurrentTimeFormatted();
	oss << "[" << currentTime << "] "
		<< "[Thread-" << std::this_thread::get_id() << "] "
		<< "[INFO] "
		<< message << "\n";
	LogStream stream;
	stream << oss.str();
	asyncLog_.Append(stream.buffer().data(), stream.buffer().length());
	if (displayLog_) {
		printf("%s", stream.buffer().data());
	}
}

void Logger::LogError(const std::string& message)
{
	std::ostringstream oss;
	std::string currentTime = GetCurrentTimeFormatted();
	oss << "[" << currentTime << "] "
		<< "[Thread-" << std::this_thread::get_id() << "] "
		<< "[ERROR] "
		<< message << "\n";
	LogStream stream;
	stream << oss.str();
	asyncLog_.Append(stream.buffer().data(), stream.buffer().length());
	if (displayLog_) {
		printf("%s", stream.buffer().data());
	}
}

void Logger::LogWarning(const std::string& message)
{
	std::ostringstream oss;
	std::string currentTime = GetCurrentTimeFormatted();
	oss << "[" << currentTime << "] "
		<< "[Thread-" << std::this_thread::get_id() << "] "
		<< "[WARNING] "
		<< message << "\n";
	LogStream stream;
	stream << oss.str();
	asyncLog_.Append(stream.buffer().data(), stream.buffer().length());
	if (displayLog_) {
		printf("%s", stream.buffer().data());
	}
}

void Logger::LogDebug(const std::string& message)
{
	std::ostringstream oss;
	std::string currentTime = GetCurrentTimeFormatted();
	oss << "[" << currentTime << "] "
		<< "[Thread-" << std::this_thread::get_id() << "] "
		<< "[DEBUG] "
		<< message << "\n";
	LogStream stream;
	stream << oss.str();
	asyncLog_.Append(stream.buffer().data(), stream.buffer().length());
	if (displayLog_) {
		printf("%s", stream.buffer().data());
	}
}

void Logger::LogGeneral(const std::string& message)
{
	std::ostringstream oss;
	std::string currentTime = GetCurrentTimeFormatted();
	oss << "[" << currentTime << "] "
		<< "[Thread-" << std::this_thread::get_id() << "] "
		<< "[GENERAL] "
		<< message << "\n";
	LogStream stream;
	stream << oss.str();
	asyncLog_.Append(stream.buffer().data(), stream.buffer().length());
	if (displayLog_) {
		printf("%s", stream.buffer().data());
	}
}

void Logger::LogCritical(const std::string& message)
{
	std::ostringstream oss;
	std::string currentTime = GetCurrentTimeFormatted();
	oss << "[" << currentTime << "] "
		<< "[Thread-" << std::this_thread::get_id() << "] "
		<< "[CRITICAL] "
		<< message << "\n";
	LogStream stream;
	stream << oss.str();
	asyncLog_.Append(stream.buffer().data(), stream.buffer().length());
	if (displayLog_) {
		printf("%s", stream.buffer().data());
	}
}

std::string Logger::GetCurrentTimeFormatted() const
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto nowAsTimeT = std::chrono::system_clock::to_time_t(now);
	auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	std::tm bt;
#ifdef _WIN32
	localtime_s(&bt, &nowAsTimeT);
#else
	localtime_r(&nowAsTimeT, &bt);
#endif // _WIN32
	std::ostringstream oss;
	oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
	oss << ',' << std::setfill('0') << std::setw(3) << nowMs.count();
	return oss.str();
}

AsyncLogging::AsyncLogging(const std::string& basename, int flushInterval):basename_(basename),
flushInterval_(flushInterval),
running_(false),
currentBuffer_(new Buffer),
nextBuffer_(new Buffer),
latch_(1)
{
	// reserve 20 message buffer.
	buffers_.reserve(20);
}

AsyncLogging::~AsyncLogging()
{
}

void AsyncLogging::Append(const char* logline, int len)
{
	// Append message to the buffer
	// if it is full push it to the buffer vec
	// copy the next available buffer or create a new one..
	std::scoped_lock<std::mutex>lock(mutex_);
	if (currentBuffer_->available() > len) {
		currentBuffer_->append(logline, len);
	}
	else {
		buffers_.push_back(std::move(currentBuffer_));
		if (nextBuffer_) {
			currentBuffer_ = std::move(nextBuffer_);
		}
		else {
			currentBuffer_.reset(new Buffer);
		}
		currentBuffer_->append(logline, len);
		cond_.notify_all();
	}
}

void AsyncLogging::Stop()
{
	running_ = false;
	cond_.notify_all();
	thread_.join();
}

void AsyncLogging::Start()
{
	running_ = true;
	thread_ = std::thread(&AsyncLogging::threadFunc, this);
	latch_.wait();
}

void AsyncLogging::threadFunc()
{
	latch_.countDown();
	LogFile outputLog(basename_);
	BufferPtr buf1(new Buffer);
	BufferPtr buf2(new Buffer);
	BufferVector buffersToWrite;
	buffersToWrite.reserve(20);
	while (running_) {
		{
			std::unique_lock<std::mutex> lock(mutex_);
			// If no new buffers yet, wait for either new log data or timeout 
			if (buffers_.empty()) {
				cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
			}
			buffers_.push_back(std::move(currentBuffer_));
			currentBuffer_ = std::move(buf1);
			buffersToWrite.swap(buffers_);
			if (!nextBuffer_) {
				nextBuffer_ = std::move(buf2);
			}
		}
		for (const BufferPtr& buffer : buffersToWrite) {
			outputLog.append(buffer->data(), buffer->length());
		}
		buffersToWrite.clear();
		outputLog.flush();
		if (buffersToWrite.capacity() > static_cast<unsigned long long>(2) * 20) {
			buffersToWrite.shrink_to_fit();
		}
		buf1.reset(new Buffer);
		buf2.reset(new Buffer);

	}
	outputLog.flush();
}
