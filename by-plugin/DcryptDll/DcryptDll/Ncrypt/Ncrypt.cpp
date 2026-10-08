#include <Windows.h>
#include <iostream.h>

#include "marc4.h"
#include "md5.h"

// forward declares of functions used
bool DoFCipher(char *in_filename, char *out_filename, unsigned char *key, int keylen);
bool DoHexEncode(char *in_filename, char *out_filename, char *hex_fmt_str);
bool DoMD5Hash(char *in_filename, char *out_filename, bool hex_encode_md5);
bool DoSCipher(char *in_string, char *out_filename, unsigned char *key, int keylen);
bool HexDecode(unsigned char *hex_string, int slen);
bool HexEncode(unsigned char *hex_string, int slen);
bool KeyFromFile( char *key_filename, unsigned char *key_buffer);
HANDLE open_file(char *filename, int mode);
void usage();

// open file enums
enum
{
    _INPUT_,   // mode for open files (read mode)
    _OUTPUT_   //     "     "         (write mode)
};

// Program Description:
// Program provides 5 "functions":
//   1.: Create an encrypted (decrypted) output file from an input file using a key residing in a key file
//   2.: Create an encrypted (decrypted) output file from an input file using a key input as a string
//   3.: Create an output file containing an encrypted (decrypted) string using a key residing in a key file
//   4.: Create an output file containing an encrypted (decrypted) string using a key input as a string
//   5.: Create an output file containing hex encoding of data in an Inputfile, optionally using a format string.
//   6.: Create an output file containing bin encoding of MD5 hash digest of the contents of an input file
//   7.: Create an output file containing hex encoding of MD5 hash digest of the contents of an input file

// Designed to be given ALL of the needed info as command line parms OR will prompt for info if NO parms 
// are provided.

// SINGLE parm of -h causes program to print a "usage" screen and exit.

// Partial or incorrect parms will be treated as if invoked with a SINGLE parm of  -h  
// To invoke with parms, parms MUST be in specific order, incorrect/wrong # of parms: see previous line.

// note using convention where 1st parm is argv[1], 2nd is argv[2], etc. (argv[0], the real 1st parm is program name)

// if invoke with parms ( other than -h ):
// 1st parm: "function" to perform (see usage screen), one of:   -FF | -FS | -HE | -MB | MH | -SF | -SS

// 2nd parm: one of:    HexKeyFileName | HexKeyString | InputFileName depending on 1st parm (see usage screen)
//     HexKeyFileName is name (full path) of file that contains the hex encoded string to use as the KEY
//     HexKeyString   is the actual hex encoded string to use as the KEY (as opposed to reading from a file)
//     InputFileName  is name (full path) of file that will be: Hex Encoded if 1st parm is -HE or file MD5 hash
//                    will be calculated for if 1st parm is -MB or -MH

// 3rd parm:  one of:   InputFileName | InputString | OutputFileName depending on 1st parm (see usage screen)
//     InputFileName  is name (full path) of file that will be encrypted (decrypted) using the KEY
//     InputString    is an actual string that will be encrypted (decrypted) using the KEY
//     OutputFileName is name (full path) of Output file of Hex Encoded InputFile's contents if 1st parm is -HE
//     OutputFilename is name (full path) of Output file containing MD5 hash of input file's contents
//                    if 1st parm is -MB or -MH, file will be hex encoded if 1st parm is -MH

// 4th parm:          OutputFileName
//     OutputFileName  is name (full path) of encrypted (decrypted) InputFile OR file CONTAINING the
//     OptionalFmtStr  is an OPTIONAL format string allowed and used ONLY if 1st parm is -HE
//     encrypted (decrypted) InputString, NOT USED if 1st parm is -MB or -MH

// NOTE when using function 3 and 4 encrypt (decrypt) input string, the input string does NOT need to be a hex
// encoded string (though the KEY strings still do), the output string written to the output file will NOT 
// be hex encoded. Function 5 is provided so schemes that use 1 (hex encoded) key string to encrypt another
// non hex encoded key string can get the 2nd encrypted key string into a file in hex encoded form to allow
// usage of functions 1 and 3 for the decryption phase (which requires the key string to be in hex encoded form).
// If this doesn't make sense, send be BEER!

int main(int argc, char *argv[])
{
    bool interactive = false;     // interactive (prompt for info) OR take from command line parms
    bool key_from_file = false;   // get encryption (decryption) key from file
    bool encrypt_file = false;    // encrypt (decrypt) a file OR a string OR see following bool
    bool hex_encode_file = false; // hex encode a file (key_from_file AND encrypt_from_file ignored if this is true)
    bool calc_md5_hash = false;   // calc md5 hash of a file, write it to an output file
    bool hex_encode_md5 = false;  //  hex encode the MD5 hash that gets written to file if calc_md5_hash == true

    // invoked with no parms (i.e. only program name): interactive:
    if (argc == 1)
        interactive = true;
    else
    {
        // invoked with parms, get/test parms
        if (  lstrcmp(argv[1],"-h") == 0 ||
             (lstrcmp(argv[1], "-FF") != 0 && lstrcmp(argv[1], "-FS") != 0 &&
              lstrcmp(argv[1], "-SF") != 0 && lstrcmp(argv[1], "-SS") != 0 &&
              lstrcmp(argv[1], "-HE") != 0 && lstrcmp(argv[1], "-MB") != 0 && lstrcmp(argv[1], "-MH") != 0
             )
           )
        {
            // 1st parm asking for help or incorrect parms print usage
            usage();
            return(1);
        }

        // 1st parm ok, validate correct # parms
        if ( (lstrcmp(argv[1], "-HE") == 0 && (argc != 4 && argc != 5)) ||
             (lstrcmp(argv[1], "-MB") == 0 && argc != 4) ||
             (lstrcmp(argv[1], "-MH") == 0 && argc != 4) ||
             (lstrcmp(argv[1], "-HE") != 0 && lstrcmp(argv[1], "-MB") != 0  &&
              lstrcmp(argv[1], "-MH") != 0 && argc != 5
             )
           )
        {
            // 1st parm asking for help or incorrect parms print usage
            usage();
            return(1);
        }

        // correct # of parms, validate supported parms (in correct order)
        if ( lstrcmp(argv[1],"-FF") == 0)
        {
            // -FF : use key from a FILE to encrypt (decrypt) an input FILE to an output file
            key_from_file = true;
            encrypt_file = true;
        }
        else
        if ( lstrcmp(argv[1], "-FS") == 0)
        {
            // -FS : use key from a FILE to encrypt (decrypt) an input STRING to an output file
            key_from_file = true;
        }
        else
        if (lstrcmp(argv[1], "-HE") == 0)
        {
            // -HE : Hex Encode an input FILE to an output file
            hex_encode_file = true;
        }
        else
        if (lstrcmp(argv[1], "-MB") == 0)
        {
            // -MD5 : Calc MD5 has of an input FILE and write it to an output file
            calc_md5_hash = true;
        }
        else
        if (lstrcmp(argv[1], "-MH") == 0)
        {
            // -MD5 : Calc MD5 has of an input FILE, hex encode it, and write it to an output file
            calc_md5_hash = true;
            hex_encode_md5 = true;
        }
        else
        if (lstrcmp(argv[1], "-SF") == 0)
        {
            // -SF : use input hex key STRING to encrypt (decrypt) an input FILE to an output file
            encrypt_file = true;
        }
        else
        if (lstrcmp(argv[1], "-SS") != 0)
        {
            // -SS : use input hex key STRING to encrypt (decrypt) an input STRING to an output file
            usage();
            return(1);
        }
    }

    // to hold the Input filename for -FF, -HE,  and -SF OR the string to encrypt for -FS or -SS
	char in_file_or_str[1025];
    FillMemory( (void *) in_file_or_str, 1025, '\0');

    // to hold the Output filename
    char out_filename[1024];
    FillMemory( (void *) out_filename, 1024, '\0');

    // to hold "function" answers in interactive (prompt for) mode
    char ans1[20];
    FillMemory ( (void *) ans1, 20, '\0');

    // to hold the optional hex format string for hex encode file function
    char hex_fmt_str[1024];
    FillMemory( (void *) hex_fmt_str, 1024, '\0');

    // "functionality" output
    cout << "Creates an encrypted(decrypted) file,\na file containing and encrypted(decrypted) string,\n";
    cout << "or a Hex Encoded file.\n\n";

    // if doing interactive, determine whether Encrypting (Decrypting) or Hex Encoding
    if (interactive)
    {
        // determine if getting key from (F)ile or input (S)tring
        cout << "F Encrypts(Decrypts) using a key string from a file,\nH Hex Encodes a file,\n";
        cout << "M calcs MD5 hash of a file,\nS Encrypts(Decrypts) using a key string entered directly.\n\n";
        cout << "F, H, M, or S (CTRL-C to quit): ";
        cin >> ans1;
        if (lstrcmp(ans1, "F") == 0)
            key_from_file = true;
        else
        if (lstrcmp(ans1, "H") == 0)
            hex_encode_file = true;
        else
        if (lstrcmp(ans1, "M") == 0)
            calc_md5_hash = true;
        else
        if (lstrcmp(ans1, "S") != 0)
        {
            usage();
            return(1);
        }
    }

    // Hex encode file:
    if (hex_encode_file)
    {
        if (interactive)
        {
            // prompt for input filename
            cout << "File to Hex Encode (CTRL-C to quit): ";
            cin  >> in_file_or_str;
            // prompt for output filename
            cout << "Hex Encoded Output File (CTRL-C to quit): ";
            cin >> out_filename;
            // prompt for optional format string
            cout << "Use Format String (Y/N)? (CTRL-C to quit): ";
            cin >> ans1;
            if (lstrcmp(ans1, "Y") == 0 || lstrcmp(ans1, "y") == 0)
            {
                cout << "Enter the format string (CTRL-C to quit)\n: ";
                cin >> hex_fmt_str;
            }
        }
        else
        {
            lstrcpy(in_file_or_str, argv[2]);
            lstrcpy(out_filename, argv[3]);
            if (argc == 5)
            {
                lstrcpy(hex_fmt_str, argv[4]);
            }
        }

        // make sure in file and outfile NOT the same file
        if (lstrcmp(in_file_or_str, out_filename) == 0)
        {
            cout << "ERROR: Input File and Output File MUST NOT be the same!\n";
            return(1);
        }

        // do the hex encoding
        if ( DoHexEncode(in_file_or_str, out_filename, hex_fmt_str) )
        {
            cout << "OK, Hex Encoded File " << out_filename << " written.\n";
            return(0);
        }
        else
        {
            // errors will have already been printed return (1)
            return(1);
        }
    }

    // NOT Hex Encoding a file, see if doing MD5 hash
    // Calc MD5 hash of a file
    // Hex encode file:
    if (calc_md5_hash)
    {
        if (interactive)
        {
            // prompt for binary or hex encoding
            FillMemory ( (void *) ans1, 20, '\0');
            cout << "Now Enter B to leave the MD5 hash Binary, H to Hex Encode it.\n";
            cout << "B or H (CTRL-C to quit): ";
            cin >> ans1;
            if (lstrcmp(ans1, "H") == 0)
                hex_encode_md5 = true;
            else
            {
                if (lstrcmp(ans1, "B") != 0)
                {
                    usage();
                    return(1);
                }
            }
            // prompt for input filename
            cout << "File to Calculate MD5 hash on (CTRL-C to quit): ";
            cin  >> in_file_or_str;
            // prompt for output filename
            cout << "Output File to contain MD5 hash (CTRL-C to quit): ";
            cin >> out_filename;
        }
        else
        {
            lstrcpy(in_file_or_str, argv[2]);
            lstrcpy(out_filename, argv[3]);
        }

        // make sure in file and outfile NOT the same file
        if (lstrcmp(in_file_or_str, out_filename) == 0)
        {
            cout << "ERROR: Input File and Output File MUST NOT be the same!\n";
            return(1);
        }

        // do the hex encoding
        if ( DoMD5Hash(in_file_or_str, out_filename, hex_encode_md5) )
        {
            if (hex_encode_md5)
                cout << "OK, File cotaining hex encoded MD5 Hash " << out_filename << " written.\n";
            else
                cout << "OK, File cotaining MD5 Hash " << out_filename << " written.\n";
            return(0);
        }
        else
        {
            // errors will have already been printed return (1)
            return(1);
        }
    }

    // NOT Hex Encoding a file or calcing MD5 hash, Encrypting (Decrypting) a string or file:

    // the Hex encoded KeyString will use to encrypt (decrypt), actual key will be decoded into binary string
    // NOTE MUST be even numbered length as "binary" string is 1/2 the size of hex encoded string
    char inkey[1025];
    FillMemory( (void *) inkey, 1025, '\0');

    // will need unsigned char version of key for hexdecoder
    unsigned char key[1025];
    FillMemory( (void *) key, 1025, '\0');

    // to hold name of file containing hex encoded key string for -FF and -FS
    char key_filename[1024];
    FillMemory( (void *) key_filename, 1024, '\0');

    int i, j, k = 0, hklen = 0, slen = 0;
    char c;

    if (interactive)
    {
        // determine if encrypting a  (F)ile or a (S)tring
        FillMemory ( (void *) ans1, 20, '\0');
        cout << "Now Enter F to encrypt (decrypt) a file, or S to encrypt (decrypt) a String.\n";
        cout << "F or S (CTRL-C to quit): ";
        cin >> ans1;
        if (lstrcmp(ans1, "F") == 0)
            encrypt_file = true;
        else
        {
            if (lstrcmp(ans1, "S") != 0)
            {
                usage();
                return(1);
            }
        }

        // if from file, get filename and attempt to load
        if (key_from_file)
        {
            cout << "Name of File containing Hex Encoded Key String (CTRL-C to quit): ";
            cin >> key_filename;
        }
        else
        {
            // from input string, get the string
            cout << "Hex Encoded Key String: length: 16 min, 1024 max, MUST be an EVEN number. (CTRL-C to quit)\n: ";
            cin >> inkey;
            i = lstrlen(inkey);
            for (j = 0; j < i; j++)
            {
                c = inkey[j];
                if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ) 
                {
                    if (k > 1023)
                    {
                        cout << "ERROR: Processing Key,  string too big! Aborting\n";   // process error 
                        return(1);
                    }
                    // add to key_buffer if a Hex Encoded value
                    key[k++] = (unsigned char) c;
                }
            }
            if ( k < 16 || k % 2 != 0 )
            {
                cout << "Key Length MUST be EVEN, at least 16 hex chars, and not more than 1024 hex chars!\n";
                return(1);
            }

            hklen = k; // save len of key
        }

        // prompt for filename or string to encrypt(decrypt)
        if (encrypt_file)
        {
            cout << "File to Encrypt (CTRL-C to quit): ";
            cin >> in_file_or_str;
        }
        else
        {
            cout << "String: (max length 1024) to Encrypt  (CTRL-C to quit)\n: ";
            cin.getline(in_file_or_str, 1024); // attempt to get the string to encrypt
            if (lstrlen(in_file_or_str) == 0)
                cin.getline(in_file_or_str, 1024); // attempt to get again for getline "phantom newline" bug

            slen = lstrlen(in_file_or_str); // now get length for real for test
            if ( slen < 1 || slen > 1024 )
            {
                cout << "String Length MUST be at least 1 char, and not more than 1024 chars!\n";
                return(1);
            }
        }

        // prompt for output filename
        cout << "Encrypted (DeCrypted) Output File (CTRL-C to quit): ";
        cin >> out_filename;
    }
    else
    {
        // NOT interactive command line should contain parms
        if (key_from_file)
            lstrcpy(key_filename, argv[2]);
        else
        {
            lstrcpy(inkey, argv[2]);
            i = lstrlen(inkey);
            for (j = 0; j < i; j++)
            {
                c = inkey[j];
                if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ) 
                {
                    if (k > 1023)
                    {
                        cout << "ERROR: Processing Key,  string too big! Aborting\n";   // process error 
                        return(1);
                    }
                    // add to key_buffer if a Hex Encoded value
                    key[k++] = (unsigned char) c;
                }
            }
            if ( k  < 16 || k % 2 != 0 )
            {
                cout << "Key Length MUST be EVEN, at least 16 hex chars, and not more than 1024 hex chars!\n";
                return(1);
            }

            hklen = k; // save len of key
        }

        lstrcpy(in_file_or_str, argv[3]);
        lstrcpy(out_filename, argv[4]);
    }

    // if encrypting (decrypting) a file, make sure in file and outfile NOT the same file
    if (encrypt_file)
        if (lstrcmp(in_file_or_str, out_filename) == 0)
        {
            cout << "ERROR: Input File and Output File MUST NOT be the same!\n";
            return(1);
        }

    // parms processed or input, if getting key from filename, get it now
    if ( key_from_file)
    {
        if ( !KeyFromFile(key_filename, key) )
        {
            // if error occurred bail
            return(1);
        }
        else
        {
            hklen = lstrlen( (char *) key);
        }
    }

    // need decoded length (will be 1/2 hex encoded key)
    // note hex encoded key length MUST be EVEN number
    j = hklen / 2;

    // now decode the hex encode key
    if ( !HexDecode(key, hklen) )
        return(1);

    // Do the encryption (or decryption since using symmetric stream cipher ARC4)
    // if no errors output Ok, filename and return 0 
    if (encrypt_file)
    {
        if ( DoFCipher(in_file_or_str, out_filename, key, j) )
        {
            cout << "OK, Encrypted (DeCrypted) File " << out_filename << " written.\n";
            return(0);
        }
    }
    else
    {
        if ( DoSCipher(in_file_or_str, out_filename, key, j) )
        {
            cout << "OK, File containing Encrypted (DeCrypted) String " << out_filename << " written.\n";
            return(0);
        }
    }

    // if got here then errors occurred, messages already printed, return 1 (for error)
    return(1);

    // thaaaats all folks!
}

// Encrypt (DeCrypt) in_filename to out_filename using key
bool DoFCipher(char *in_filename, char *out_filename, unsigned char *key, int keylen)
{
    // try to open the input file for reading
    HANDLE hInFile = open_file(in_filename, _INPUT_);
    // print error if could not open and return
    if (hInFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open In_File Failed! Aborting\n";   // process error 
        return false;
    }

    // try to open the output file for writing
    HANDLE hOutFile = open_file(out_filename, _OUTPUT_);
    // print error &  close in file if could not open and return
    if (hOutFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open Out_File Failed! Aborting\n";   // process error 
        CloseHandle(hInFile); // close the input file
        return false;
    }

    // get Input File size
    unsigned int dwInFileSize = GetFileSize(hInFile, NULL);
    // if an error occurred print message, delete useless out file and bail
    if ( dwInFileSize == 0xFFFFFFFF)
    {
        cout << "ERROR: GetFileSize Failed! Aborting\n";   // process error 
        CloseHandle(hInFile);     // close the input file
        CloseHandle(hOutFile);    // close the output file
        DeleteFile(out_filename); // delete the file since it will NOT be complete
        return false;
    }

    // generate encryptor/decryptor
    mrc4_ctx arc4;
    mrc4_keysetup(&arc4, key, keylen);

    // process the file in 4k chunks
    unsigned char buffer[4096];
    unsigned long  bytesRead, bytesWritten;

    do
    {
        if ( !ReadFile(hInFile, buffer, 4096, &bytesRead, NULL) )
        {
            // if an error,  print error, close the files, delete useless out file and bail
            cout << "ERROR: Reading File! Aborting\n";   // process error 
            CloseHandle(hInFile);
            CloseHandle(hOutFile);
            DeleteFile(out_filename); // delete the file since it will NOT be complete
            return false;
        }
        else
        {
            // read okay encrypt (decrypt) the buffer
            mrc4_crypt(&arc4, (unsigned char *) &buffer, (unsigned char *) &buffer, bytesRead);
            // write the block to the output file
            if ( !WriteFile(hOutFile, buffer, bytesRead, &bytesWritten, NULL) || bytesWritten != bytesRead)
            {
                // if an error,  print error, close the files, delete useles out file, bail
                cout << "ERROR: Writing File! Aborting\n";   // process error 
                CloseHandle(hInFile);
                CloseHandle(hOutFile);
                DeleteFile(out_filename); // delete the file since it will NOT be complete
                return false;
            }
        }
    } while (bytesRead == 4096);
    
    // Close the files.
    CloseHandle(hInFile);
    CloseHandle(hOutFile);

    // return true (success)
    return true;
}

// Hex Encode in_filename to out_filename
bool DoHexEncode(char *in_filename, char *out_filename, char *hex_fmt_str)
{
    bool use_fmt_string = false;  // use supplied format string when function is hex encode file if true
    int i, j;
    int curr_fmt_pos = 0, fmt_len = 0;
    // try to open the input file for reading
    HANDLE hInFile = open_file(in_filename, _INPUT_);
    // print error if could not open and return
    if (hInFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open In_File Failed! Aborting\n";   // process error 
        return false;
    }

    // try to open the output file for writing
    HANDLE hOutFile = open_file(out_filename, _OUTPUT_);
    // print error &  close in file if could not open and return
    if (hOutFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open Out_File Failed! Aborting\n";   // process error 
        CloseHandle(hInFile); // close the input file
        return false;
    }

    // get Input File size
    unsigned int dwInFileSize = GetFileSize(hInFile, NULL);
    // if an error occurred print message, delete useless out file and bail
    if ( dwInFileSize == 0xFFFFFFFF)
    {
        cout << "ERROR: GetFileSize Failed! Aborting\n";   // process error 
        CloseHandle(hInFile);     // close the input file
        CloseHandle(hOutFile);    // close the output file
        DeleteFile(out_filename); // delete the file since it will NOT be complete
        return false;
    }

    // if length of hex format string not == 0, make sure is valid
    if (lstrlen(hex_fmt_str) > 0)
    {
        fmt_len = lstrlen(hex_fmt_str);
        if ( (unsigned int)fmt_len < (dwInFileSize * 2) )
        {
            cout << "ERROR: bad hex format string, too small!\n";   // process error 
            CloseHandle(hInFile);     // close the input file
            CloseHandle(hOutFile);    // close the output file
            DeleteFile(out_filename); // delete the file since it will NOT be complete
            return false;
        }

        // make sure format string doesnt contain hex chars itself, i.e. no '0'..'9' or 'A'..'F', or 'a'..'f'
        // and does contain 2 * dwInFileSize '#' chars
        for (i = 0, j = 0; i < fmt_len; i++)
        {
            // test for "hex" char
            if ( (hex_fmt_str[i] >= '0' && hex_fmt_str[i] <= '9') || 
                 (hex_fmt_str[i] >= 'A' && hex_fmt_str[i] <= 'F') || 
                 (hex_fmt_str[i] >= 'a' && hex_fmt_str[i] <= 'f')
               )
            {
                cout << "ERROR: bad hex format string, invalid chars!\n";   // process error 
                CloseHandle(hInFile);     // close the input file
                CloseHandle(hOutFile);    // close the output file
                DeleteFile(out_filename); // delete the file since it will NOT be complete
                return false;
            }

            if (hex_fmt_str[i] == '#')
                j++;
        }

        // if make it through loop have no invalid chars, make sure have correct number of '#' chars
        if ( (unsigned int) j != (dwInFileSize * 2) )
        {
            cout << "ERROR: bad hex format string, wrong number of '#' chars!\n";   // process error 
            CloseHandle(hInFile);     // close the input file
            CloseHandle(hOutFile);    // close the output file
            DeleteFile(out_filename); // delete the file since it will NOT be complete
            return false;
        }
        // if made it to here format string should be okay
        use_fmt_string = true;
        curr_fmt_pos = 0; // initialize 
    }

    // dont expect will want to hex encode very large files, so no sense wasting too much buffer,
    // process file in 128 byte chunks (need room for 256 hex encoded bytes)
    unsigned char buffer[256], buffer2[256];
    unsigned long  bytesRead, bytesWritten, hex_bytesWritten;

    do
    {
        if ( !ReadFile(hInFile, buffer, 128, &bytesRead, NULL) )
        {
            // if an error,  print error, close the files, delete useless out file and bail
            cout << "ERROR: Reading File! Aborting\n";   // process error 
            CloseHandle(hInFile);
            CloseHandle(hOutFile);
            DeleteFile(out_filename); // delete the file since it will NOT be complete
            return false;
        }
        else
        {
            // read okay hex encode the buffer
            if ( !HexEncode(buffer, bytesRead) )
            {
                CloseHandle(hInFile);
                CloseHandle(hOutFile);
                DeleteFile(out_filename); // delete the file since it will NOT be complete
                return false;
            }

            // if have hex fmt string format contents of buffer and write
            if (use_fmt_string)
            {
                hex_bytesWritten = 0;
                // copy up to 128 bytes to buffer2 coming either from format string or hex encoded buffer
                // do this and write until buffer contents have been written to disk
                j = 0; // use i for buffer2 pos, j for hex encoded buffer pos
                while (hex_bytesWritten < bytesRead * 2)
                {
                    i = 0;
                    while (i < 256  && j < (int) bytesRead * 2 ) // up to 256 chars or until hex encoded buffer exhausted
                    {
                        // format string len should be >= total bytes in file * 2, cur_fmt_pos >= fmt_len an error
                        if (curr_fmt_pos >= fmt_len)
                        {
                            // should never happen (here in case I overlooked some details!)
                            // if an error,  print error, close the files, delete useles out file, bail
                            cout << "ERROR: Format string Exhausted! Aborting\n";   // process error 
                            CloseHandle(hInFile);
                            CloseHandle(hOutFile);
                            DeleteFile(out_filename); // delete the file since it will NOT be complete
                            return false;
                        }

                        // check curr_fmt_pos of format string to see if a place holder or literal char
                        if (hex_fmt_str[curr_fmt_pos] == '#' )
                            buffer2[i++] = buffer[j++]; // placeholder, take char from hex encoded buffer
                        else
                            buffer2[i++] = hex_fmt_str[curr_fmt_pos]; // literal, take char from format string
                        curr_fmt_pos++;
                    }
                    // have 256 chars or hex encoded buffer exhausted, do write
                    if ( !WriteFile(hOutFile, buffer2, i, &bytesWritten, NULL) || (int) bytesWritten != i)
                    {
                        // if an error,  print error, close the files, delete useles out file, bail
                        cout << "ERROR: Writing File! Aborting\n";   // process error 
                        CloseHandle(hInFile);
                        CloseHandle(hOutFile);
                        DeleteFile(out_filename); // delete the file since it will NOT be complete
                        return false;
                    }
                    hex_bytesWritten += j;
                }
            }
            else
            {
                // no format string, just write the block to the output file
                if ( !WriteFile(hOutFile, buffer, (2 * bytesRead), &bytesWritten, NULL) ||
                      bytesWritten != (2 * bytesRead)
                   )
                {
                    // if an error,  print error, close the files, delete useles out file, bail
                    cout << "ERROR: Writing File! Aborting\n";   // process error 
                    CloseHandle(hInFile);
                    CloseHandle(hOutFile);
                    DeleteFile(out_filename); // delete the file since it will NOT be complete
                    return false;
                }
            }
        }
    } while (bytesRead == 128);

    // if using optional format string, write any trailing literal characters in the format string to file
    if (use_fmt_string && curr_fmt_pos < fmt_len)
    {
        while (curr_fmt_pos < fmt_len)
        {
            i = 0;
            j = curr_fmt_pos;

            while (i < 256 && j < fmt_len)
            {
                buffer2[i++] = hex_fmt_str[j++];
            }

            // have 256 chars or fmt string  exhausted, do write
            if ( !WriteFile(hOutFile, buffer2, i, &bytesWritten, NULL) || (int) bytesWritten != i)
            {
                // if an error,  print error, close the files, delete useles out file, bail
                cout << "ERROR: Writing File! Aborting\n";   // process error 
                CloseHandle(hInFile);
                CloseHandle(hOutFile);
                DeleteFile(out_filename); // delete the file since it will NOT be complete
                return false;
            }

            curr_fmt_pos += j;
        }
    }

    // Close the files.
    CloseHandle(hInFile);
    CloseHandle(hOutFile);

    return true;
}

// Calc MD5 haso of contents of  in_filename, write hex encoded to out_filename
bool DoMD5Hash(char *in_filename, char *out_filename, bool hex_encode_md5)
{
    // try to open the input file for reading
    HANDLE hInFile = open_file(in_filename, _INPUT_);
    // print error if could not open and return
    if (hInFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open In_File Failed! Aborting\n";   // process error 
        return false;
    }

    // get Input File size
    unsigned int dwInFileSize = GetFileSize(hInFile, NULL);
    // if an error occurred print message, delete useless out file and bail
    if ( dwInFileSize == 0xFFFFFFFF)
    {
        cout << "ERROR: GetFileSize Failed! Aborting\n";   // process error 
        CloseHandle(hInFile);     // close the input file
        return false;
    }

    // expect most often doing MD5 hashes of KEY files which will be small, no sense wasting too much buffer,
    // process file in 256 byte chunks
    unsigned char buffer[256];
    unsigned long bytesRead, bytesWritten;

    // create and initialize MD5 hash context
    MD5Context    md5c;
    MD5Init(&md5c);

    do
    {
        if ( !ReadFile(hInFile, buffer, 256, &bytesRead, NULL) )
        {
            // if an error,  print error, close the files, delete useless out file and bail
            cout << "ERROR: Reading File! Aborting\n";   // process error 
            CloseHandle(hInFile);
            return false;
        }
        else
        {
            // read okay update the MD5 hash context
            MD5Update(&md5c, buffer, (unsigned int) bytesRead);
        }
    } while (bytesRead == 256);
    
    // Close the input file
    CloseHandle(hInFile);

    // now reuse buffer to hold the 16 byte MD5 Hash
    FillMemory( (void *) buffer, 256, '\0');
    MD5Final(buffer, &md5c);

    // now Hex Encode the hash if chosen 
    if ( hex_encode_md5)
        if ( !HexEncode(buffer, 16) )
            return false;

    // try to open the output file for writing
    HANDLE hOutFile = open_file(out_filename, _OUTPUT_);
    // print error &  close in file if could not open and return
    if (hOutFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open Out_File Failed! Aborting\n";   // process error 
        return false;
    }

    // write the MD5 hash to the output file
    if ( hex_encode_md5)
    {
        // 32 btyes to write if hex encoded
        if ( !WriteFile(hOutFile, buffer, 32, &bytesWritten, NULL) || bytesWritten != 32 )
        {
            // if an error,  print error, close the files, delete useles out file, bail
            cout << "ERROR: Writing File! Aborting\n";   // process error 
            CloseHandle(hOutFile);
            DeleteFile(out_filename); // delete the file since it will NOT be complete
            return false;
        }
    }
    else
    {
        // 16 btyes to write if NOT hex encoded
        if ( !WriteFile(hOutFile, buffer, 16, &bytesWritten, NULL) || bytesWritten != 16 )
        {
            // if an error,  print error, close the files, delete useles out file, bail
            cout << "ERROR: Writing File! Aborting\n";   // process error 
            CloseHandle(hOutFile);
            DeleteFile(out_filename); // delete the file since it will NOT be complete
            return false;
        }
    }

    // all done, close file and return true
    CloseHandle(hOutFile);
    return true;
}

// Encrypt (Decrypt) in_string using key and write it to out_filename
bool DoSCipher(char *in_string, char *out_filename, unsigned char *key, int keylen)
{
    // try to open the output file for writing
    HANDLE hOutFile = open_file(out_filename, _OUTPUT_); ;
    // print error &  close in file if could not open and return
    if (hOutFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open Out_File Failed! Aborting\n";   // process error 
        return false;
    }

    // generate encryptor/decryptor
    mrc4_ctx arc4;
    mrc4_keysetup(&arc4, key, keylen);

    // encrypt (decrypt) the string
    int slen = lstrlen( (LPCSTR) in_string);
    mrc4_crypt(&arc4, (unsigned char *) in_string, (unsigned char *) in_string, slen);

    // write the block to the output file
    unsigned long  bytesWritten;
    if ( !WriteFile(hOutFile, in_string, slen, &bytesWritten, NULL) || (int) bytesWritten != slen)
    {
        // if an error,  print error, close the files, delete useles out file, bail
        cout << "ERROR: Writing File! Aborting\n";   // process error 
        CloseHandle(hOutFile);
        DeleteFile(out_filename); // delete the file since it will NOT be complete
        return false;
    }

    // Close the file.
    CloseHandle(hOutFile);

    // return true (success)
    return true;
}

bool KeyFromFile( char *key_filename, unsigned char *key_buffer)
{
    // try to open the input key file for reading
    HANDLE hKeyFile = open_file(key_filename, _INPUT_); 
    // print error if could not open and return
    if (hKeyFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open Key_File Failed! Aborting\n";   // process error 
        return false;
    }

    // get Input File size
    unsigned int dwKeyFileSize = GetFileSize(hKeyFile, NULL);
    // if an error occurred print message and bail
    if ( dwKeyFileSize == 0xFFFFFFFF)
    {
        cout << "ERROR: GetFileSize of KeyFile Failed! Aborting\n";   // process error 
        CloseHandle(hKeyFile);     // close the input file
        return false;
    }

    // max key size = 1024, but allow /r/n and '-' or other seperators in key, so allow
    // larger size with SKIP of values outside range of '0' <--> '9' and 'A' <--> 'F'
    // (or 'a' - 'f'), process file using buffer of 256
    unsigned char buffer[256];
    unsigned long  bytesRead;

    int j, k = 0;
    unsigned char c;
    // read key file
    do
    {
        if ( !ReadFile(hKeyFile, buffer, 256, &bytesRead, NULL) )
        {
            // if an error,  print error, close the file, and bail
            cout << "ERROR: Reading Key File! Aborting\n";   // process error 
            CloseHandle(hKeyFile);
            return false;
        }

        // process buffer skipping non Hex Encoded Characters (bail is key size exceeds 1024)
        for (j = 0; j < (int) bytesRead; j++)
        {
            c = buffer[j];
            if ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ) 
            {
                if (k > 1023)
                {
                    cout << "ERROR: Processing Key File, key too big! Aborting\n";   // process error 
                    CloseHandle(hKeyFile);
                    return false;
                }
                // add to key_buffer if a Hex Encoded value
                key_buffer[k++] = c;
            }
        }
    } while (bytesRead == 256);

    // close the keyfile
    CloseHandle(hKeyFile);

    // make sure length okay
    if ( k  < 16 || k % 2 != 0 )
    {
        cout << "Key Length MUST be EVEN, at least 16 hex chars, and not more than 1024 hex chars!\n";
        return(false);
    }

    // key okay, return true
    return true;
}

// decodes (to unsigned char) the hex encoded string in hex_string, in place
bool HexDecode(unsigned char *hex_string, int slen)
{
    if ( slen < 2 || slen % 2 != 0)
    {
        cout << "HexDecode: hex encoded string length NOT even! Aborting.";
        return false;
    }

    int           i, j;
    unsigned char c, newchar, hex_sub1, hex_sub2;
    hex_sub1 = unsigned char ('A' - 10);
    hex_sub2 = unsigned char ('a' - 10);

    for (i = 0, j = 0; i < slen; i++)
    {
        // get character and make sure a valid hex encoded character
        c = hex_string[i];
        if ( ! ( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ) ) 
        {
            cout << "HexDecode: invalid (non hex) character " << c << " in string! Aborting.";
            return false;
        }
        // valid hex char, convert to binary value
        if (c >= '0' && c <= '9')
            c -= (unsigned char) '0';
        else
        if (c >= 'A' && c <= 'F')
            c = (unsigned char) (c - hex_sub1);
        else
            c = (unsigned char) (c - hex_sub2);

        // now see if 1st or second byte of 2 byte group
        if (i % 2 == 0)
            newchar = (unsigned char) (c << 4); // 1st byte of group (0 based: 0 is 1st, 1 is 2nd, 2 is 1st, 3 is 2nd, etc)
        else
        {
            // 2nd byte push together into single unsigned char byte
            newchar = (unsigned char) (newchar | c);    // low nibble of newchar = 0000, high nibble of c is 0000 so bitwise OR
            hex_string[j++] = newchar;// can overwrite hex_string as j < i so already processed
        }
    }

    // done decoding, value of j is 1/2 slen, 0 out hex_string from j to end;
    while (j < slen)
    {
        hex_string[j++] = 0;
    }

    // all done
    return true;
}

// hex encodes in place the string in in_string, in_string MUST be big enough to hold 2 * slen chars
bool HexEncode(unsigned char *in_string, int slen)
{
    int           i, j;
    int           hchar1, hchar2;
    unsigned char c;

    if ( slen < 1)
    {
        cout << "HexEncode: hex encoded string length == 0! Aborting.";
        return false;
    }

    // since doing in place and encoded string will be TWICE size of input string must process from end
    for (i = slen - 1, j = (slen - 1) * 2; i >= 0; i--, j -= 2)
    {
        // get character and make sure a valid hex encoded character
        c = in_string[i];

        // process "high" nibble
        hchar1 = c >> 4;
        if (hchar1 < 10)
            hchar1 += (int) '0';
        else
            hchar1 = (hchar1 - 10) + (int) 'A';

        // process "low" nibble"
        hchar2 = c & 0x0f;
        if (hchar2 < 10)
            hchar2 += (int) '0';
        else
            hchar2 = (hchar2 - 10) + (int) 'A';

        // put hchar1 and hchar2 back into appropriate place in in_string
        in_string[j] = (unsigned char) hchar1;
        in_string[j+1] = (unsigned char) hchar2;
    }

    // all done
    return true;
}

HANDLE open_file(char *filename, int mode)
{
    HANDLE hFile;

    switch (mode)
    {
        case _INPUT_:
            hFile = CreateFile( filename,              // open input file 
                                GENERIC_READ,          // open for reading 
                                0,                     // dont share 
                                NULL,                  // no security 
                                OPEN_EXISTING,         // existing file only 
                                FILE_ATTRIBUTE_NORMAL, // normal file 
                                NULL);                 // no attr. template 
            break;
 
        case _OUTPUT_:
            hFile = CreateFile( filename,               // open output file
                                GENERIC_WRITE,          // open for writing 
                                0,                      // do not share 
                                NULL,                   // no security 
                                CREATE_ALWAYS,          // overwrite existing 
                                FILE_ATTRIBUTE_NORMAL | // normal file with
                                FILE_FLAG_WRITE_THROUGH,// no lazy flushing 
                                NULL);                  // no attr. template 

            break;

        default:
            return(INVALID_HANDLE_VALUE);
    }

    return hFile;
}

void usage()
{
    cout << "\n\n";
    cout << "Usage: Ncrypt [-FF  HexKeyFileName InputFileName   OutputFileName  |\n";
    cout << "               -FS  HexKeyFileName InputString     OutputFileName  |\n";
    cout << "               -HE  InputFileName  OutputFileName [OptionalFmtStr] |\n";
    cout << "               -MB  InputFileName  OutputFileName                  |\n";
    cout << "               -MH  InputFileName  OutputFileName                  |\n";
    cout << "               -SF  HexKeyString   InputFileName   OutputFileName  |\n";
    cout << "               -SS  HexKeyString   InputString     OutputFileName  |\n";
    cout << "               -h\n";
    cout << "              ]\n\n";
    cout << "-FF    Encrypt (Decrypt) InputFileName to OutputFileName using Hex Encoded\n";
    cout << "         Key String found in HexKeyFileName.\n\n";
    cout << "-FS    Encrypt (Decrypt) InputString to OutputFileName using Hex Encoded\n";
    cout << "         Key String found in HexKeyFileName.\n\n";
    cout << "-HE    Hex Encode InputFileName to OutputFileName\n";
    cout << "          optionally using a format string.\n\n";
    cout << "-MB    Calculate MD5 hash of InputFileName and write it OutputFileName.\n\n";
    cout << "-MH    Calculate MD5 hash of InputFileName and write it hex encoded to\n";
    cout << "         OutputFileName.\n\n";
    cout << "-SF    Encrypt (Decrypt) InputFilename to OutputFileName using Hex Encoded\n";
    cout << "         Key String HexKeyString.\n\n";
    cout << "-SS    Encrypt (Decrypt) InputString to OutputFileName using Hex Encoded\n";
    cout << "         Key String HexKeyString.\n\n";
    cout << "-h     Help: this screen.\n\n";
    cout << "Without parms runs interactively, i.e. prompts for necessary parameters.\n\n";
}
