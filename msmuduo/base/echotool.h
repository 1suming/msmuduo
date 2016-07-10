#ifndef _echotool_h
#define _echotool_h

enum ECHO_COLOR {
	COLOR_DEFAULT,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW
};

void ColoredPrintf(ECHO_COLOR color, const char* fmt, ...);

#define ECHO_ERR(fmt, ...)		ColoredPrintf(COLOR_RED, fmt"\n", __VA_ARGS__)
#define ECHO_OK(fmt, ...)		ColoredPrintf(COLOR_GREEN, fmt"\n", __VA_ARGS__)
#define ECHO_WARN(fmt, ...)		ColoredPrintf(COLOR_YELLOW, fmt"\n", __VA_ARGS__)
#define ECHO_INFO(fmt, ...)		fprintf(stdout, fmt"\n", __VA_ARGS__);


namespace echotool
{
	// 改变控制台颜色
	void changeToColor(ECHO_COLOR);

	// 恢复控制台颜色
	void changeToOldColor();

	// 根据传入的格式文本和参数返回最终的文本串
	const char* getmsg(const char* fmt, ...);
}




#endif