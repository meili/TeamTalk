/*******************************************************************************
 *  @file      TransferFile.h 2014\9\5 13:18:03 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     传输的文件的操作
 ******************************************************************************/

#ifndef TRANSFERFILE_4DABC92A_C413_4E6C_9C7C_5B43437B7076_H__
#define TRANSFERFILE_4DABC92A_C413_4E6C_9C7C_5B43437B7076_H__

#include "GlobalDefine.h"
/******************************************************************************/
enum Position
{
	Beginning,
	Current,
	End
};

/**
 * The class <code>传输文件操作</code> 
 *
 */
class TransferFile final
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	TransferFile(const CString &path, BOOL bsave);
    /**
     * Destructor
     */
    ~TransferFile();
    //@}
	BOOL readBlock(UInt32 offset, UInt32 size, OUT std::string& data);
	BOOL writeBlock(UInt32 offset,UInt32 size,void* data);
	UInt32 length();

private:
	UInt32 tell() const;
	void seek(long offset, Position p = Beginning);

private:
	class FilePrivate;
	FilePrivate*				m_pfileObject;
};
/******************************************************************************/
#endif// TRANSFERFILE_4DABC92A_C413_4E6C_9C7C_5B43437B7076_H__
