#include "..\stdafx.h"
#include ".\Base64.h"
#include <Wincrypt.h>

static char *szCompiledFile=__FILE__;

CBase64::CBase64()
{
}

CBase64::~CBase64()
{
}

String &CBase64::Encode(ByteArray &data,String &encodedData) {

	init_func
	DWORD dwStrSize = 0;
	CryptBinaryToString(data.lock(), data.size(), CRYPT_STRING_BASE64, NULL, &dwStrSize);
	encodedData.resize(dwStrSize);
	CryptBinaryToString(data.lock(),data.size(),CRYPT_STRING_BASE64,encodedData.lock(),&dwStrSize);

	return encodedData;
	exit_func
}

ByteDynArray &CBase64::Decode(const char *encodedData,ByteDynArray &data) {
	init_func

	DWORD dwDataSize = 0;
	CryptStringToBinary(encodedData, 0, CRYPT_STRING_BASE64, NULL, &dwDataSize, NULL, NULL);
	data.resize(dwDataSize);
	CryptStringToBinary(encodedData, 0, CRYPT_STRING_BASE64, data.lock(), &dwDataSize, NULL, NULL);

	//CBase64Ex base64;
	//dwDataSize=base64.CalculateRecquiredDecodeOutputBufferSize(encodedData);
	//data.resize(dwDataSize);
	//base64.DecodeBuffer(encodedData,(char *)data.lock());
	
	_return(data)
	exit_func
}
