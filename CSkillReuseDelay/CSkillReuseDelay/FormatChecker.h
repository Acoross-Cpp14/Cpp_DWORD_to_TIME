#pragma once

#include <utility>

///////////////////////////////////////////////////////////////////////////
// format char checker

template<char c, class T>
inline constexpr bool IsFormatType()
{
	return false;
}

#define DEFINE_FORMAT(c, type)	template<>	inline constexpr bool IsFormatType<c, type>()	{	return true;	}

DEFINE_FORMAT('c', char);
DEFINE_FORMAT('d', int);
DEFINE_FORMAT('f', float);
DEFINE_FORMAT('f', double);

#define CHECK_FORMAT(c, type)	\
static_assert(IsFormatType<c, type>(), "invalid format character");

/////////////////////////////////////////////////////////////
// format
template<class T>
inline constexpr bool IsFormatType2(const char c)
{
	return false;
}

template<>
inline constexpr bool IsFormatType2<int>(const char fmt_c)
{
	return fmt_c == 'd';
}

template<>
inline constexpr bool IsFormatType2<double>(const char fmt_c)
{
	return fmt_c == 'f';
}

template<>
inline constexpr bool IsFormatType2<char>(const char fmt_c)
{
	return fmt_c == 'c';
}

///////////////////////////////////////////////////////////////////////////
// format string checker

template <std::size_t N, class... Args>
inline constexpr 
bool CheckFormatString(const char(&fmt)[N], const Args...)
{
	static_assert(N == sizeof...(Args)+1, "params count differs from format string");
	return _CheckFormatString_impl<0>(fmt, (Args)(0)...);
}

template <std::size_t n, class... Args, std::size_t N>
inline constexpr
bool _CheckFormatString_impl(const char(&fmt)[N], Args...);

template <std::size_t n, class Arg, class... Args, std::size_t N>
inline constexpr
bool _CheckFormatString_impl(const char(&fmt)[N], Arg, Args...)
{
	return _CheckFormatString_impl<n>(fmt, (Arg)(0))
		? _CheckFormatString_impl<n + 1>(fmt, (Args)(0)...)
		: false;
}

template <std::size_t n, class Arg, std::size_t N>
inline constexpr 
bool _CheckFormatString_impl(const char(&fmt)[N], Arg)
{
	//return _CheckFormatString_One_impl<n, Arg>(fmt);
	static_assert(n < N, "array index >= array size");
	return IsFormatType2<Arg>(fmt[n]);
}

//template <std::size_t n, class Arg, std::size_t N>
//inline constexpr 
//bool _CheckFormatString_One_impl(const char(&fmt)[N])
//{
//	static_assert(n < N, "array index >= array size");
//	return IsFormatType2<Arg>(fmt[n]);
//}

inline 
void TestFormatChecker()
{
	CHECK_FORMAT('c', char);
	CHECK_FORMAT('d', int);
	CHECK_FORMAT('f', float);

	constexpr char arr[] = "ddfcdc";

	////////////////////////////////////////////////
	//static_assert(
	CheckFormatString("ddfcdc", 1, 1, 0.1, 'a', 10, 'b');
			//, "format string differs from param lists");

	int a = 0, b = 0, c = 0;
	double d = .1;
	char e = 'c', f = 'd';
	unsigned int u = 1;
	CheckFormatString("ddfcdc", a, b, d, e, c, f);
}

//////////////////////////////////////////////
// format string 3

