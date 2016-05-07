/******************************************************************************* 
 *  @file      TransferFile.cpp 2014\9\5 13:18:08 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     传输的文件的操作
 ******************************************************************************/

#include "stdafx.h"
#include "TransferFile.h"
#include "utility/utilCommonAPI.h"
/******************************************************************************/
class TransferFile::FilePrivate
{
public:
	FilePrivate(const CString& path, BOOL bsave);
	~FilePrivate();

public:
	FILE	*file;
	BOOL	readOnly;
	UInt32	size;
};

TransferFile::FilePrivate::~FilePrivate()
{
	if (file)
		fclose(file);
}

TransferFile::FilePrivate::FilePrivate(const CString& path, BOOL bsave) 
:file(0)
,readOnly(TRUE)
{
	if (!path.IsEmpty())
	{
		if (bsave)
		{
			file = _wfopen(path, _T("wb+"));
			if (file)
				readOnly = false;
			else
			{
				file = _wfopen(path, _T("wb"));
			}
		}
		else
		{
			file = _wfopen(path, _T("rb+"));
			if (file)
			{
				readOnly = false;
			}
			else
			{
				file = _wfopen(path, _T("rb"));
			}
		}
	}
	if (!file)
		LOG__(APP,_T("Could not open file %s"), path);
}

// -----------------------------------------------------------------------------
//  TransferFile: Public, Constructor

TransferFile::TransferFile(const CString &path, BOOL bsave)
:m_pfileObject(nullptr)
{
	m_pfileObject = new FilePrivate(path, bsave);
}

// -----------------------------------------------------------------------------
//  TransferFile: Public, Destructor

TransferFile::~TransferFile()
{
	delete m_pfileObject;
	m_pfileObject = nullptr;
}

BOOL TransferFile::writeBlock(UInt32 offset, UInt32 size, void* data)
{
	if (!m_pfileObject->file)
		return FALSE;

	if (m_pfileObject->readOnly)
	{
		LOG__(ERR, _T("attempted to write to a file that is not writable"));
		return FALSE;
	}

	seek(offset, Beginning);
	int count = fwrite(data, sizeof(char), size, m_pfileObject->file);
	if (count < 1)
		return FALSE;

	return TRUE;
}

BOOL TransferFile::readBlock(UInt32 offset, UInt32 size, OUT std::string& data)
{
	if (!m_pfileObject || !m_pfileObject->file)
	{
		LOG__(ERR, _T("Invalid File"));
		return FALSE;
	}
	seek(offset, Beginning);
	data.resize(size);
	int count = fread((void*)data.data(), sizeof(char), size,m_pfileObject->file);
	if (count < 1)
	{
		return FALSE;
	}

	return TRUE;
}

void TransferFile::seek(long offset, Position p /*= Beginning*/)
{
	if (!m_pfileObject->file)
	{
		LOG__(ERR, _T("trying to seek in a file that isn't opened."));
		return;
	}

	switch (p)
	{
	case Beginning:
		fseek(m_pfileObject->file, offset, SEEK_SET);
		break;
	case Current:
		fseek(m_pfileObject->file, offset, SEEK_CUR);
		break;
	case End:
		fseek(m_pfileObject->file, offset, SEEK_END);
		break;
	}
}

UInt32 TransferFile::tell() const
{
	return ftell(m_pfileObject->file);
}

UInt32 TransferFile::length()
{
	if (m_pfileObject->size > 0)
		return m_pfileObject->size;

	if (!m_pfileObject->file)
		return 0;

	long curpos = tell();
	seek(0, End);
	long endpos = tell();

	seek(curpos, Beginning);

	m_pfileObject->size = endpos;
	return endpos;
}

/******************************************************************************/