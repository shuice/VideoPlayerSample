#pragma once

#ifndef _SMI_SUB_TITLE_READER_H_
#define _SMI_SUB_TITLE_READER_H_

#include "SrtSubTitleReader.h"


class CSmiSubTitleReader : public CSrtSubTitleReader
{
public:
    bool LoadFile(const string& strSrtFileName);
private:
    string TripMarkFlags(const string& str);
};

#endif
