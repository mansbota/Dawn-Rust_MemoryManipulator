#pragma once
#include <string>
#include <stdexcept>

class BasicException : public std::exception
{
protected:
	int m_line;
	mutable std::string m_whatBuffer;
	std::string getOrigin() const noexcept;
public:
	BasicException(const int line) noexcept;
	virtual const char* getType() const noexcept;
};

