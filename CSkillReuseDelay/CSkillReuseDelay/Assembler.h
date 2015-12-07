#pragma once


typedef char* packet_t;

template<class... Args>
char* Assemble(char* packet, const char* const format, Args... args);

template<class Arg>
char* Assemble(char* packet, const char* format, Arg arg)
{
	const char format_c = format[0];
	switch (format_c)
	{
	case 'd':
	{
		int* target = (int*)packet;
		*target = arg;
		packet += sizeof(int);
	}
	break;
	default:
		_ASSERT(0);
		break;
	}

	return packet;
}

template<class Arg, class...Args>
char* Assemble(char* packet, const char* const format, Arg arg, Args... args)
{
	packet = Assemble(packet, format, arg);
	packet = Assemble(packet, &format[1], args...);
	return packet;
}


constexpr int const_strlen(const char* const format)
{
	return *format != '\0' ? 1 + const_strlen(format + 1) : 0;
}

template<size_t N, class... Args>
char* Disassemble(char* packet, const char(&format)[N], Args*... pArgs)
{
	static_assert(N == sizeof...(pArgs)+1, "Disassemble assert: format size differs from argument count");
	_ASSERT(N == sizeof...(pArgs)+1);

	return Disassemble_impl(packet, format, pArgs...);
}

template<class Arg, class... Args>
char* Disassemble_impl(char* packet, const char* const format, Arg* pArg, Args*... pArgs);

template<class Arg, class... Args>
char* Disassemble_impl(char* packet, const char* const format, Arg* pArg, Args*... pArgs)
{
	packet = Disassemble_impl(packet, format, pArg);
	packet = Disassemble_impl(packet, &format[1], pArgs...);
	return packet;
}

template<class Arg>
char* Disassemble_impl(char* packet, const char* format, Arg* pArg)
{
	const char format_c = *format;
	switch (format_c)
	{
	case 'd':
	{
		int* target = (int*)packet;
		*pArg = *target;
		packet += sizeof(int);
	}
	break;
	default:
		_ASSERT(0);
		break;
	}

	return packet;
}
