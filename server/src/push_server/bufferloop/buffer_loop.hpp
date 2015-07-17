/**	@file buffer_Loop.hpp
 *	@note 
 *	@brief 环形存储块的类
 *
 *	@author		shiwei
 *	@date		2013/11/23
 *
 *	@note 实现一个环形内存区的类，该类线程不安全，需要外部加锁保护,但是对于分别只有一个
 *        读和写的线程是安全的
 *	@note 历史记录：
 *	@note V1.0.0  创建文件
 */
#ifndef _BUFFER_LOOP_HPP
#define _BUFFER_LOOP_HPP


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <malloc.h>

class CBufferLoop
{
public:
	CBufferLoop(void)
	{
		m_pbuffer = NULL;
		m_nbuffer_size = 0;
		m_nhead_pos = 0;
		m_ntail_pos = 0;
		m_nused_size = 0;
	}
	virtual ~CBufferLoop(void)
	{
		destroy_buffer();
	}

	bool create_buffer(int nsize)
	{
		destroy_buffer();
        int nrealsize = (nsize / 4 + 1) * 4;
		m_pbuffer = (char*)malloc(nrealsize);
		memset(m_pbuffer, 0, nrealsize);
		m_nbuffer_size = nrealsize;
		m_nused_size = 0;
		return true;
	}


	void destroy_buffer()
	{
		if (m_pbuffer != NULL)
		{
			free(m_pbuffer);
			m_pbuffer = NULL;
			m_nhead_pos = 0;
			m_ntail_pos = 0;
			m_nused_size = 0;
		}
	}

	int get_buffer_size() { return m_nbuffer_size; }
	int get_used_size() { return m_nused_size; }
	int get_rest_size() { return get_buffer_size() - get_used_size(); }
    
	/**	@fn	char CBufferLoop::get_buffer_tmp(char* pbuffer, int nbuffer_size, int* nreal_buffer_size)
	*	@brief 获取指定大小的数据，但是不会清空该数据
	*	@param[in] pbuffer 数据存放指针
	*	@param[in] nbuffer_size 想要获取的大小
	*	@param[in] nreal_buffer_size 实际获取的大小
	*	@return	
	*/
	char get_buffer_tmp(char* pbuffer, int nbuffer_size, int* nreal_buffer_size)
	{
		bool bret = true;
		int nrealsize = (nbuffer_size < get_used_size() ? nbuffer_size : get_used_size());
		if (m_nhead_pos <= m_ntail_pos)
		{
			memcpy(pbuffer, &m_pbuffer[m_nhead_pos], nrealsize);
		}
		else
		{
			int nrestsize = m_nbuffer_size - m_nhead_pos;
			//到末尾的长度大于需要的长度
			if (nrealsize <= nrestsize)
			{
				memcpy(pbuffer, &m_pbuffer[m_nhead_pos], nrealsize);
			}
			else
			{
				//需要分两次获取,先把到末尾的数据全部获取
				memcpy(pbuffer, &m_pbuffer[m_nhead_pos], nrestsize);
				//然后从起始处获取剩余的数据
				memcpy(&pbuffer[nrestsize], &m_pbuffer[0], nrealsize - nrestsize);
			}
		}
		*nreal_buffer_size = nrealsize;
		return bret;
	}

	/**	@fn	bool CBufferLoop::append_buffer(const char* pbuffer, int nbuffer_size)
	*	@brief 
	*	@param[in] pbuffer 
	*	@param[in] nbuffer_size 
	*	@return	
	*/
	bool append_buffer(const char* pbuffer, int nbuffer_size)
	{
		bool bret = false;
		if (get_rest_size()  < nbuffer_size)
		{
			return bret;
		}

		if (m_ntail_pos < m_nhead_pos)
		{
			memcpy(&m_pbuffer[m_ntail_pos], pbuffer, nbuffer_size);
		}
		else   //m_ntail_pos >= m_nhead_pos
		{
			//剩余多少个字节
			int nrest_tail = m_nbuffer_size - m_ntail_pos;
			if (nrest_tail >= nbuffer_size)
			{
				memcpy(&m_pbuffer[m_ntail_pos], pbuffer, nbuffer_size);
			}
			else
			{

				//分两段进行拷贝
				memcpy(&m_pbuffer[m_ntail_pos], pbuffer, nrest_tail);
				memcpy(&m_pbuffer[0], &pbuffer[nrest_tail], nbuffer_size - nrest_tail);

			}
		}
		m_ntail_pos = (m_ntail_pos + nbuffer_size) % m_nbuffer_size;
		m_nused_size += nbuffer_size;
		return bret;
	}

	/**	@fn	bool CBufferLoop::get_buffer(char* pbuffer, int nbuffer_size, int* nreal_buffer_size)
	*	@brief 获取指定长度的数据，并且会清空获取的数据
	*	@param[in] pbuffer 数据存放指针
	*	@param[in] nbuffer_size 想要获取的数据长度
	*	@param[out] nreal_buffer_size  实际获得的数据长度
	*	@return	
	*/
	bool get_buffer(char* pbuffer, int nbuffer_size, int* nreal_buffer_size)
	{
		bool bret = true;
		int nrealsize = (nbuffer_size < get_used_size() ? nbuffer_size : get_used_size());
		if (m_nhead_pos <= m_ntail_pos)
		{
			memcpy(pbuffer, &m_pbuffer[m_nhead_pos], nrealsize);
		}
		else
		{
			int nrestsize = m_nbuffer_size - m_nhead_pos;
			//到末尾的长度大于需要的长度
			if (nrealsize <= nrestsize)
			{
				memcpy(pbuffer, &m_pbuffer[m_nhead_pos], nrealsize);
			}
			else
			{
				//需要分两次获取,先把到末驳氖萑炕袢?
				memcpy(pbuffer, &m_pbuffer[m_nhead_pos], nrestsize);
				//然后从起始处获取剩余的数据
				memcpy(&pbuffer[nrestsize], &m_pbuffer[0], nrealsize - nrestsize);
			}
		}
		m_nused_size -= nrealsize;
		m_nhead_pos = (m_nhead_pos + nrealsize) % m_nbuffer_size;
		*nreal_buffer_size = nrealsize;
		return bret;
	}
    
private:
	char* m_pbuffer;
	int m_nbuffer_size;

	int m_nhead_pos;			//从0开始
	int m_ntail_pos;			//从0开始
	int m_nused_size;			//使用的量
};

class CSimpleBuffer
{
public:
	CSimpleBuffer()
    {
        m_pbuffer = NULL;
        m_nalloc_size = 0;
        m_nwrite_offset = 0;
    }
	~CSimpleBuffer()
    {
        m_nalloc_size = 0;
        m_nwrite_offset = 0;
        if (m_pbuffer)
        {
            free(m_pbuffer);
            m_pbuffer = NULL;
        }
    }
	char*  GetBuffer() { return m_pbuffer; }
    const char* GetBuffer() const { return m_pbuffer; }
	int GetAllocSize() { return m_nalloc_size; }
	int GetWriteOffset() { return m_nwrite_offset; }
	void IncWriteOffset(int nlen) { m_nwrite_offset += nlen; }
    
	void Extend(int nlen)
    {
        m_nalloc_size = m_nwrite_offset + nlen;
        m_nalloc_size += m_nalloc_size >> 2;	// increase by 1/4 allocate size
        char* new_buf = (char*)realloc(m_pbuffer, m_nalloc_size);
        if(new_buf != NULL)
        {
            m_pbuffer = new_buf;
        }
    }
    
	int Write(const char* pbuf, int nlen)
    {
        if (m_nwrite_offset + nlen > m_nalloc_size)
        {
            Extend(nlen);
        }
        
        if (pbuf)
        {
            memcpy(m_pbuffer + m_nwrite_offset, pbuf, nlen);
        }
        
        m_nwrite_offset += nlen;
        
        return nlen;
    }
    
	int Read(char* pbuf, int nlen)
    {
        if (nlen > m_nwrite_offset)
            nlen = m_nwrite_offset;
        
        if (pbuf)
            memcpy(pbuf, m_pbuffer, nlen);
        
        m_nwrite_offset -= nlen;
        memmove(m_pbuffer, m_pbuffer + nlen, m_nwrite_offset);
        return nlen;
    }
private:
	char* m_pbuffer;
	int	m_nalloc_size;
	int	m_nwrite_offset;
};

#endif