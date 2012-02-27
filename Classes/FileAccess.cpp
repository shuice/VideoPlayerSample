/*
 *  FileAccess.cpp
 *  LIB_UTILITY
 *
 *  Created by mbp on 08/05/28.
 *  Copyright 2008 occlusion. All rights reserved.
 *
 */
#include "FileAccess.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if _WIN32
#include <windows.h>
#include <tchar.h>
//Using old functions
#pragma warning( disable : 4996 )  
#else
#include <fcntl.h>
#include <unistd.h>

#endif // WIN32


bool FileAccess::exist(const char *path){
	bool flg;
	FILE *fp = fopen(path, "rb");
	if(fp){
		flg = true;
		fclose(fp);
	}
	else
		flg = false;
	return flg;
}
void FileAccess::_open(const char *path, bool mode){
	
#if _WIN32
	TCHAR *name;
	
#ifndef UNICODE
/*    const size_t newsize = 200;
    wchar_t wcstring[newsize];
	memset(wcstring, 0, sizeof(wcstring));
	MultiByteToWideChar(CP_ACP, 0, path, strlen(path), wcstring, sizeof(wcstring));
	name = reinterpret_cast<TCHAR *>(wcstring);
	*/
	name = (TCHAR *)path;
#else
    const size_t newsize = 200;
    wchar_t wcstring[newsize];
	memset(wcstring, 0, sizeof(wcstring));
	MultiByteToWideChar(CP_ACP, 0, path, strlen(path), wcstring, sizeof(wcstring));
	name = reinterpret_cast<TCHAR *>(wcstring);
#endif
	
	switch (mode) {
		case FILE_READ:
			_file = CreateFile(name,  GENERIC_READ, FILE_SHARE_READ, NULL,
							   OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			break;
		case FILE_WRITE:
			_file = CreateFile(name,  GENERIC_WRITE, FILE_SHARE_READ, NULL,
							   CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			break;
	}
	
#else
	switch (mode) {
		case FILE_READ:
			_file = open(path, O_RDONLY);
//			_file = fopen(path, "rb");
			break;
		case FILE_WRITE:
			_file = open(path, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
//			_file = fopen(path, "wb");
			break;
	}
#endif
	if(!this->isValid())	return;
	if(mode == FILE_READ){
		this->seek(0, FILE_SEEK_END);
		_size = tell();
		this->seek(0, FILE_SEEK_SET);
	}
}

void FileAccess::_close(){
	if(this->isValid()){
		if(_writeChecker)	this->write(&_writeFooter, sizeof(_writeFooter));
#if _WIN32
		CloseHandle((HANDLE)_file);
#else
		close(_file);
//		fclose((FILE *)_file);
#endif
	}
	if(_allReadBuffer){
		free(_allReadBuffer);
	}
}

#if _WIN32
#else
static inline size_t _read(int fd, void *buf, size_t size){
	return read(fd, buf, size);
}
#endif

int FileAccess::read(void *buf, int size){
	if(this->isValid()){
#if _WIN32
		int len;
		ReadFile((HANDLE)_file, buf, (DWORD)size, (LPDWORD)&len, NULL) ;
		return len;
#else
		return _read(_file, buf, size);
//		return read(buf, size);
//		return fread(buf, size, 1, (FILE *)_file);
#endif
	}
	return 0;
}

#if _WIN32
#else
static inline size_t _write(int fd, const void *buf, size_t size){
	return write(fd, buf, size);
}
#endif
void FileAccess::write(const void *data, unsigned long long size){
	if(this->isValid()){
#if _WIN32
		int len;
		WriteFile((HANDLE)_file, data, (DWORD)size, (LPDWORD)&len, NULL);
		_size += len;
#else
		_size += _write(_file, data, size);
//		_size += fwrite(data, size, 1, (FILE *)_file);
#endif
	}
}

unsigned long long FileAccess::tell(){
	if(this->isValid()){
#if _WIN32
		return SetFilePointer((HANDLE)_file, 0, NULL, FILE_CURRENT);
#else
//		return tell(_file);
		return lseek(_file, 0, SEEK_CUR);
//		return ftello((FILE *)_file);
#endif
	}
	return 0;
}

void FileAccess::seek(long long pos, int point){
	if(this->isValid()){
#if _WIN32
		if(point == FILE_SEEK_SET)
			point = FILE_BEGIN;
		else if(point == FILE_SEEK_CUR)
			point = FILE_CURRENT;
		else if(point == FILE_SEEK_END)
			point = FILE_END;
		
		SetFilePointer((HANDLE)_file, pos, NULL, point);
#else
		lseek(_file, pos, point);
//		fseeko((FILE *)_file, pos, point);
#endif
	}
}
unsigned long long FileAccess::size(){
	return _size;
}

bool FileAccess::isValid(){
#ifdef WIN32
	return (_file != INVALID_HANDLE_VALUE);
#else
	return (_file != -1);
#endif
}

void FileAccess::setChecker(const long long &header,const long long &footer){
	if(!_size && _mode == FILE_WRITE){
		_writeChecker = true;
		_writeFooter = footer;
		_writeHeader = header;
		this->write(&_writeHeader, sizeof(_writeHeader));
	}
}

bool FileAccess::checkFile(const long long &header, const long long &footer){
	if(!this->isValid() || _mode == FILE_WRITE || _size < 16)	return false;
	long long readedHeader, readedFooter;
	if(_allReadBuffer){
		memcpy(&readedHeader, _allReadBuffer, sizeof(long long));
		memcpy(&readedFooter, &_allReadBuffer[_size - sizeof(long long)], sizeof(long long));		
	}
	else{
		int pos = tell();
		this->seek(0, FILE_SEEK_CUR);	
		this->read(&readedHeader, sizeof(long long));
		this->seek(-8, FILE_SEEK_END);	
		this->read(&readedFooter, sizeof(long long));
		this->seek(pos, FILE_SEEK_CUR);	
	}
	_enableCheck = (readedHeader == header && readedFooter == footer);
	return _enableCheck;
}


char *FileAccess::allRead(){
	if(this->isValid()){
		int pos = tell();
		this->seek(0, FILE_SEEK_SET);
		_allReadBuffer = (char *)malloc(size());
		read(_allReadBuffer, _size);
		this->seek(pos, FILE_SEEK_SET);
		return (char *)(_allReadBuffer+_enableCheck*8);
	}
	return NULL;
}

int FileAccess::getChar(){
	unsigned char ch;
//	int ch = EOF;
	if(this->read(&ch, 1))	return ch;
	return EOF;
};
void FileAccess::putChar(int chr){
	unsigned char ch = chr;
	this->write(&ch, 1);
}

