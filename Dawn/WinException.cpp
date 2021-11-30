#include "WinException.h"
#include <sstream>

WinException::WinException(int line, HRESULT hr) noexcept :
	BasicException(line), hr{ hr } {}

const char* WinException::what() const noexcept
{
	std::ostringstream str;

	str << "[Error Code] 0x" << std::hex << hr << " (" << std::dec << hr << ")\n"
		<< "[Description] " << translateError(hr) << std::endl << getOrigin();

	m_whatBuffer = str.str();

	return m_whatBuffer.c_str();
}

const char* WinException::getType() const noexcept
{
	return "Windows Exception";
}

std::string WinException::translateError(HRESULT hr) noexcept
{
	LPSTR messageBuffer = nullptr;

	const auto size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

	if (size == 0)
		return std::string("Unidentified Error Code");

	std::string errorStr = messageBuffer;

	LocalFree(messageBuffer);

	return errorStr;
}