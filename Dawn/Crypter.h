#pragma once

#include <array>

template <size_t size>
class _String {

	std::array<char, size> _data{};
	char _key;

public:
	constexpr _String(char* data) : _key{ __TIME__[4] } {

		crypt(data);
	}

	operator char* () {

		crypt(_data.data());

		return &_data[0];
	}

	operator std::string_view() {

		return (char*)*this;
	}

private:

	constexpr void crypt(char* data) {

		for (auto i = 0u; i < size; i++) {

			_data[i] = data[i] ^ (_key * 4);
		}
	}
};

#define String(str) []() { \
	static constexpr auto string = _String<sizeof(str) / sizeof(str[0])>((std::remove_const_t<std::remove_reference_t<decltype(str[0])>>*)str); \
	return string; } ()


template <size_t size>
class _wString {

	std::array<wchar_t, size> _data{};
	char _key;

public:
	constexpr _wString(wchar_t* data) : _key{ __TIME__[4] } {

		crypt(data);
	}

	operator wchar_t* () {

		crypt(_data.data());

		return &_data[0];
	}

	operator std::wstring_view() {

		return (wchar_t*)*this;
	}

private:

	constexpr void crypt(wchar_t* data) {

		for (auto i = 0u; i < size; i++) {

			_data[i] = data[i] ^ (_key * 4);
		}
	}
};

#define wString(str) []() { \
	static constexpr auto string = _wString<sizeof(str) / sizeof(str[0])>((std::remove_const_t<std::remove_reference_t<decltype(str[0])>>*)str); \
	return string; } ()