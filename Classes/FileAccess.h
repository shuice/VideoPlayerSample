/*
 *  FileAccess.h
 *  LIB_UTILITY
 *
 *  Created by mbp on 08/05/28.
 *  Copyright 2008 occlusion. All rights reserved.
 *
 */
#ifndef __FILE
#define __FILE
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#if _WIN32
#include <windows.h>
#endif
#include <stdio.h>

#define FILE_READ 0
#define FILE_WRITE 1

#define FILE_SEEK_SET SEEK_SET
#define FILE_SEEK_CUR SEEK_CUR
#define FILE_SEEK_END SEEK_END

#ifndef cpy
#define cpy(x, y, z) memcpy(x, *y, z); *y+=z;
#define alen_cpy(x, y, z) z = sizeof(x); cpy(&x, y, z)
#endif
class FileAccess{
	
#if _WIN32
	HANDLE _file;
#else
	int _file;
#endif
	char *_allReadBuffer;
	unsigned long long _size;
	
	bool _mode;
	
	bool _writeChecker;
	bool _enableCheck;
	long long _writeHeader;
	long long _writeFooter;

	void _open(const char *path, bool mode);
	void _close();
public:
	FileAccess(const char *path, bool mode):_file(NULL), _allReadBuffer(NULL), _size(0), _mode(mode), _writeChecker(false), _enableCheck(false){
		_open(path, mode);
	};
	virtual ~FileAccess(){
		_close();
	};
	
	static bool exist(const char *path);	
	bool isValid();

	int read(void *buf, int size);
	void write(const void *data, unsigned long long size);
	unsigned long long tell();
	void seek(long long pos, int point);
	
	unsigned long long size();
	int getChar();
	void putChar(int chr);
	
	//Read Only
	char *allRead();
	bool checkFile(const long long &header, const long long &footer);

	//Write Only
	void setChecker(const long long &header, const long long &footer);
};

#endif