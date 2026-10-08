// base64.h
// Base64 Encoding/Decoding Header File
//
// This implementation of Base64 encoding is based on Anders Molin's
// AMMimeUtils implementation at http://www.codeproject.com/string/ammimeutils.asp#xx266398xx.
// Here are the major changes from his original source:
//    - Added functions to calculate encode and decode buffer sizes
//    - Removed CR/LF insertion every 76th character
//    - Changed signature of Encode and Decode functions
//    - Changed data types from signed int and char to unsigned BYTE and ULONG
//    - Applied a couple of bug fixes:
//        - Fixed a bug that caused incorrect length of decoded output returned,
//	      - Fixed bug that caused incorrect length of encoded output returned
//        - Fixed a bug that caused input strings less than 4 bytes long to be
//          encoded as '===='
//	  - Changed names of some variables and constants to make them more intuitive
//
class CBase64Utils {
public:
	static ULONG CalculateEncodeBufferSize(ULONG inputsize);
	static ULONG CalculateDecodeBufferSize(ULONG inputsize);
    static void Encode(BYTE *input, BYTE *output, ULONG inlength);
	static ULONG Decode(BYTE *input, BYTE *output, ULONG inlength);
};