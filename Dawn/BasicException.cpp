#include "BasicException.h"
#include <sstream>

BasicException::BasicException(const int line) noexcept : m_line{ line } {}

const char* BasicException::getType() const noexcept
{
	return "Basic Exception";
}

std::string BasicException::getOrigin() const noexcept
{
	std::ostringstream str;
	str << "[Line] " << m_line << '\n';
	m_whatBuffer = str.str();
	return m_whatBuffer;
}
