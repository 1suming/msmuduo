#pragma once
#include<map>

class FileConfReader {
public:
	FileConfReader(const char* filename) {
		loadFile(filename);
	}
	~FileConfReader(){}

	char* getKey(const char* key);
	int setVal(const char* key, const char* val);

private:
	void  loadFile(const char* filename);
	int   writeFile(const char* filename = NULL);
	void  parseLine(char* line);
	char* trimSpace(char* name);

	bool load_ok_;
	map<string, string> config_map_;
	string config_file_;


};