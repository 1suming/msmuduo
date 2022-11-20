#include"FileConfReader.h"

char* FileConfReader::getKey(const char* key)
{
	if (!load_ok_) {
		return NULL;
	}

	char* val = NULL;
	map<string, string>::iterator it = config_map_.find(key);
	if (it != config_map_.end()) {
		val = (char*)it->second.c_str();
	}
	return val;
}
int FileConfReader::setVal(const char* key, const char* val)
{
	if (!load_ok_) {
		return NULL;
	}

	map<string, string>::iterator it = config_map_.find(key);
	if (it != config_map_.end()) {
		it->second = val;
	}
	else
	{
		//config_map_[key] = val;
		config_map_.insert(make_pair(key, val));
	}
	return writeFile();
}


void  FileConfReader::loadFile(const char* filename)
{
	config_file_.clear();
	config_file_.append(filename);

	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		printf("can not open %s,erron=%d", filename, errno);
		return;
	}
	char buf[256];
	for (;;)
	{
		//fgets 如果成功，该函数返回相同的 str 参数。如果到达文件末尾或者没有读取到任何字符，str 的内容保持不变，并返回一个空指针。如果发生错误，返回一个空指针。
		char* p = fgets(buf, 256, fp);
		if (!p) {
			break;
		}
		size_t len = strlen(buf);
		if (buf[len - 1] == '\n') {
			buf[len - 1] = 0;//removed \n at the end
		}
		//strchr 返回一个指向该字符串中第一次出现的字符的指针，如果字符串中不包含该字符则返回NULL空指针
		char *ch=strchr(buf,'#'); // remove string start with #
		if (ch) {
			*ch = 0;
		}
		if (strlen(buf) == 0) {
			continue;
		}
		parseLine(buf);
	}
	fclose(fp);
	load_ok_ = true;

}
int   FileConfReader::writeFile(const char* filename)
{
	FILE* fp = NULL;
	if (filename == NULL)
	{
		fp = fopen(config_file_.c_str(), "w");
	}
	else
	{
		fp = fopen(filename, "w");
	}
	if (fp == NULL)
	{
		return -1;
	}

	char szPaire[128];
	map<string, string>::iterator it = config_map_.begin();
	for (; it != config_map_.end(); it++)
	{
		memset(szPaire, 0, sizeof(szPaire));
		snprintf(szPaire, sizeof(szPaire), "%s=%s\n", it->first.c_str(), it->second.c_str());
		uint32_t ret = fwrite(szPaire, strlen(szPaire), 1, fp);
		if (ret != 1)
		{
			fclose(fp);
			return -1;
		}
	}
	fclose(fp);
	return 0;

}
void  FileConfReader::parseLine(char* line)
{
	char* p = strchr(line, '=');
	if (p == NULL) {
		return;
	}
	*p = 0;
	char* key = trimSpace(line);
	char* val = trimSpace(p + 1);
	if (key && val)
	{
		config_map_.insert(make_pair(key, val));
	}

}
char* FileConfReader::trimSpace(char* name)
{
	//remove starting space or tab 
	char* start_pos = name;
	while ((*start_pos == ' ') || (*start_pos == '\t'))
	{
		start_pos++;
	}
	if (strlen(start_pos) == 0) {
		return NULL;
	}

	//remove ending space or tab 
	char* end_pos = name + strlen(name) - 1;
	while ((*end_pos == ' ') || (*end_pos == '\t'))
	{
		*end_pos = 0;
		end_pos--;
	}
	int len = (int)(end_pos - start_pos) + 1;
	if (len <= 0) {
		return NULL;
	}
	return start_pos;
}
