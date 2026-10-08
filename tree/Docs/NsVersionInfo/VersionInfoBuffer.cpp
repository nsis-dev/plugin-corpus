// VersionInfoBuffer.cpp: implementation of the CVersionInfoBuffer class.
//
//////////////////////////////////////////////////////////////////////

#define UNICODE

#include <string>
#include <malloc.h>
#include "stdafx.h"
#include "VersionInfoBuffer.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVersionInfoBuffer::CVersionInfoBuffer()//:m_dwBufSize(1024), m_dwPosition(0) yields a warning ??
{
	m_dwBufSize = 1024;
	m_dwPosition = 0;
	m_lpData = (LPBYTE) new BYTE[m_dwBufSize];
}

CVersionInfoBuffer::~CVersionInfoBuffer()
{
    #ifdef _DEBUG
    OutputDebugString(L"~CVersionInfo()");
    #endif
	delete [] m_lpData;
}

/**
 * Writes data to the buffer
 */
void CVersionInfoBuffer::Write(LPVOID lpData, DWORD dwSize)
{
	if (dwSize+m_dwPosition > m_dwBufSize)
		ReallocBuffer(dwSize+m_dwPosition);

	memcpy(m_lpData + m_dwPosition, lpData, dwSize);
	m_dwPosition += dwSize;
}

void CVersionInfoBuffer::ReallocBuffer(DWORD dwMinimumSize)
{
	//Allocate extra 1k or so
	DWORD dwNewSize = (dwMinimumSize + 0x7ff) & ~0x3ff;

	LPBYTE lpNewData = new BYTE[dwNewSize];

	//Copy everything that is already in the buffer
	memcpy(lpNewData, m_lpData, m_dwPosition);

	delete [] m_lpData;
	m_dwBufSize = dwNewSize;
	m_lpData = lpNewData;
}

/**
 * Aligns to DWORD (pads with 0s)
 */
DWORD CVersionInfoBuffer::PadToDWORD()
{

	if (m_dwPosition % 4)
	{
		DWORD dwNull = 0L;
		Write(&dwNull, 4 - m_dwPosition % 4);
	}

	return m_dwPosition;
}

/**
 * Pads with zeroes
 */
DWORD CVersionInfoBuffer::Pad(WORD wLength)
{
	DWORD dwNull = 0L;
	while (wLength--)
		Write(&dwNull, 1);

	return m_dwPosition;
}

/**
 * Returns current position
 */
DWORD CVersionInfoBuffer::GetPosition()
{
	return m_dwPosition;
}

/**
 *  Writes the difference between specified offset and current length to a WORD at given offset
 *  this writing the structure size wLength
 */
void CVersionInfoBuffer::WriteStructSize(DWORD dwOffsetOfSizeMemember)
{
	WORD wSize = LOWORD(m_dwPosition - dwOffsetOfSizeMemember);

	WORD *pSizeMember = (WORD*) (&m_lpData[dwOffsetOfSizeMemember]);
	*pSizeMember = wSize;
}

/**
 * Writes a WORD to the buffer
 */
void CVersionInfoBuffer::WriteWord(WORD wData)
{
	Write(&wData, sizeof(WORD));
}

/**
 * Writes string to the buffer (converts to Unicode)
 */
WORD CVersionInfoBuffer::WriteString(const wstring &strValue)
{
#ifndef UNICODE
	DWORD dwLength = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), -1, NULL, 0);
	WCHAR *pszwValue = (WCHAR*)_alloca(dwLength * sizeof (WCHAR));
	MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), -1, pszwValue, dwLength);

	Write(pszwValue, dwLength * sizeof (WCHAR));

	return LOWORD(dwLength);
#else
	DWORD dwLength = (strValue.length()+ 1) * sizeof(WCHAR);
	Write ((LPVOID)(LPCWSTR) strValue.c_str(), dwLength);
	return LOWORD(dwLength);
#endif
}

/**
 * Get pointer to data
 * (pointer can not be used after any writes made after calling GetData() due to possible relocation)
 */
const LPBYTE CVersionInfoBuffer::GetData()
{
	return m_lpData;
}
