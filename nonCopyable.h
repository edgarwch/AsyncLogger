#pragma once
// prevent it from calling the copy constructor..
class nonCopyable
{
public:
	nonCopyable() = default;
private:
	nonCopyable(const nonCopyable&) = delete;
	nonCopyable& operator=(const nonCopyable&) = delete;
};