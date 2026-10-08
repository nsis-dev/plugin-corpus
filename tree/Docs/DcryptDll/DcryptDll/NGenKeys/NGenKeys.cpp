#include <Windows.h>
#include <math.h>
#include <iostream.h>

#include "cryptopp\hex.h"
#include "cryptopp\osrng.h"
using namespace CryptoPP;

// forward declares of functions used
void DoHexEncode(int len, unsigned char *in_string);
void GenAKey(int keylen, unsigned char *key);
void GenSKey(int keylen, char *passphrase, unsigned char *key);

HANDLE open_file(char *filename, int mode);

void usage();

// open file enums
enum
{
    _INPUT_,   // mode for open files (read mode)
    _OUTPUT_   //     "     "         (write mode)
};

// Program Description:
// Program provides 4 "functions":
//   1.: Create a random keystring with no passphrase and write to an output file (binary)
//   2.: Create a random keystring with no passphrase and write hex encoded to an output file
//   3.: Create a keystring using input passphrase as seed and write to an output file (binary)
//   4.: Create a keystring using input passphrase as seed and write hex encoded to an output file

// Designed to be given ALL of the needed info as command line parms OR will prompt for info if NO parms 
// are provided.

// SINGLE parm of -h causes program to print a "usage" screen and exit.

// Partial or incorrect parms will be treated as if invoked with a SINGLE parm of  -h  
// To invoke with parms, parms MUST be in specific order, incorrect/wrong # of parms: see previous line.

// note using convention where 1st parm is argv[1], 2nd is argv[2], etc. (argv[0], the real 1st parm is program name)

// if invoke with parms ( other than -h ):
// 1st parm: "function" to perform (see usage screen), one of:   -BA | -BS | -HA | -HS

// 2nd parm: size of key, 8 minimum, 512 max

// 3rd parm: one of:    BinKeyFileName | HexKeyFileName | InPassPhrase depending on 1st parm (see usage screen)
//     BinKeyFileName  is name (full path) of file that will hold the binary generated key if 1st parm -BA
//     HexKeyFileName  is name (full path) of file that will hold the hex encoded generated key if 1st parm -HA
//     InputPassPhrase is string used as passphrase seed for key generation if 1st parm is -BS or -HS

// 4rd parm:  one of:   InputFileName | InputString | OutputFileName depending on 1st parm (see usage screen)
//     BinKeyFileName  is name (full path) of file that will hold the binary generated key if 1st parm -BS
//     HexKeyFileName  is name (full path) of file that will hold the hex encoded generated key if 1st parm -HS
//     UNUSED if 1st parm is -BA or -HA (see 2nd parm)

// NOTE when using function 3 or 4 the input string cannot contain whitespace

int main(int argc, char *argv[])
{
    bool interactive = false;
    bool seeded_gen = false;      // using passphrase to see random number generator
    bool hex_encode_file = false; // hex encode file before writing to output file

    // invoked with no parms (i.e. only program name): interactive:
    if (argc == 1)
        interactive = true;
    else
    {
        // invoked with parms, get/test parms
        if (  lstrcmp(argv[1],"-h") == 0 ||
             (lstrcmp(argv[1], "-BA") != 0 && lstrcmp(argv[1], "-BS") != 0 &&
              lstrcmp(argv[1], "-HA") != 0 && lstrcmp(argv[1], "-HS") != 0
             )
           )
        {
            // 1st parm asking for help or incorrect parms print usage
            usage();
            return(1);
        }

        // 1st parm ok, validate correct # parms
        if ( (lstrcmp(argv[1], "-BA") == 0 && argc != 4) || (lstrcmp(argv[1], "-HA") == 0 && argc != 4) ||
             (lstrcmp(argv[1], "-BS") == 0 && argc != 5) || (lstrcmp(argv[1], "-HS") == 0 && argc != 5)
           )
        {
            // 1st parm asking for help or incorrect parms print usage
            usage();
            return(1);
        }

        // correct # of parms, validate supported parms (in correct order)
        if ( lstrcmp(argv[1], "-BS") == 0 || lstrcmp(argv[1], "-HS") == 0)
        {
            // using pass phrase
            seeded_gen = true;
        }

        if (lstrcmp(argv[1], "-HA") == 0 || lstrcmp(argv[1], "-HS") == 0)
        {
            // Hex Encode the key
            hex_encode_file = true;
        }
    }

    // to hold the Input keysize
    int  klen = 0;
	char key_len[5];
    FillMemory( (void *) key_len, 5, '\0');

    // to hold the Input pass phrase for seeded key generation
	char in_str[1025];
    FillMemory( (void *) in_str, 1025, '\0');

    // to hold the Output filename
    char out_filename[1024];
    FillMemory( (void *) out_filename, 1024, '\0');

    // to hold "function" answers in interactive (prompt for) mode
    char ans1[20];
    FillMemory ( (void *) ans1, 20, '\0');

    // "functionality" output
    cout << "Generates an AutoRandom or Seeded Key and writes the key to an output file,\n";
    cout << "key in output file is either binary or Hex Encoded.\n\n";

    // if doing interactive, determine whether Encrypting (Decrypting) or Hex Encoding
    if (interactive)
    {
        // determine if getting key from (F)ile or input (S)tring
        cout << "B writes key to outfile as binary data,\n";
        cout << "H writes key to outfile as hex encoded data.\n";
        cout << "B or H (CTRL-C to Quit): ";
        cin >> ans1;

        if (lstrcmp(ans1, "H") == 0)
            hex_encode_file = true;
        else
        if (lstrcmp(ans1, "B") != 0)
        {
            usage();
            return(1);
        }

        // determine if AutoRandom or Seeded with passphrase
        FillMemory ( (void *) ans1, 20, '\0');
        cout << "Now Enter A for AutoRandom Key Generation, or S for Seeded with a Passphrase Key Generation.\n";
        cout << "A or S (CTRL-C to Quit): ";
        cin >> ans1;

        if (lstrcmp(ans1, "S") == 0)
            seeded_gen = true;
        else
        {
            if (lstrcmp(ans1, "A") != 0)
            {
                usage();
                return(1);
            }
        }

        // now get key size
        cout << "Key Size in bytes: 8 Min, 512 Max, 511 max for NSIS (CTRL-C to Quit): ";
        cin.getline(key_len, 4); // attempt to get key_len
        if (lstrlen(key_len) == 0)
            cin.getline(key_len, 4); // get key_len again if "phantom newline" bug appears

        klen = atoi(key_len);
        if (klen < 8 || klen > 512)
        {
            usage();
            return(1);
        }

        // if Seeded Key Generation, Get passphrase
        if (seeded_gen)
        {
            cout << "Now enter a passphrase to use as the seed: 1024 chars max (CTRL-C to Quit).\n";
            cout << "PassPhrase: ";
            cin.getline(in_str, 1024); // attempt to get in_str
            if (lstrlen(in_str) == 0)
                cin.getline(in_str, 1024); // get in_str again if "phantom newline" bug appears
        }

        // prompt for output filename
        cout << "Output File (CTRL-C to Quit): ";
        cin >> out_filename;
    }
    else
    {
        // NOT interactive command line should contain parms

        // get key size
        lstrcpy(key_len, argv[2]);
        klen = atoi(key_len);
        if (klen < 8 || klen > 512)
        {
            usage();
            return(1);
        }

        if (seeded_gen)
        {
            // if seeded 3rd parm is passphrase, 4th is outfile
            lstrcpy(in_str, argv[3]);
            lstrcpy(out_filename, argv[4]);
        }
        else
        {
            // if not seeded 3rd parm is outfile
            lstrcpy(out_filename, argv[3]);
        }
    }

    unsigned char *key;
    if (hex_encode_file)
        key = new unsigned char[klen * 2];
    else
        key = new unsigned char[klen];

    // Generate key
    if (seeded_gen)
        GenSKey(klen, in_str, key);
    else
        GenAKey(klen, key);

    // hex encode key if
    if (hex_encode_file)
        DoHexEncode(klen, key);

    // write key to outfile
    // try to open the output file for writing
    HANDLE hOutFile = open_file(out_filename, _OUTPUT_);
    // print error &  close in file if could not open and return
    if (hOutFile == INVALID_HANDLE_VALUE) 
    { 
        cout << "ERROR: Open Out_File Failed! Aborting\n";   // process error 
        return(1);
    }

    // write the key to the output file
    BOOL write_ok = false;
    unsigned long  bytesWritten;
    if (hex_encode_file)
        write_ok = WriteFile(hOutFile, key, klen * 2, &bytesWritten, NULL);
    else
        write_ok = WriteFile(hOutFile, key, klen, &bytesWritten, NULL);

    if (  !write_ok  || ( hex_encode_file && (int) bytesWritten != klen * 2) ||
                        (!hex_encode_file && (int) bytesWritten != klen)
       )
    {
        // if an error,  print error, close the files, delete useles out file, bail
        cout << "ERROR: Writing File! Aborting\n";   // process error 
        CloseHandle(hOutFile);
        DeleteFile(out_filename); // delete the file since it will NOT be complete
        return(1);
    }

    // Close the file.
    CloseHandle(hOutFile);

    // print message
    cout << "OK: Output File " << out_filename << " Written!\n";   // process error 
    // return (success)
    return 0;
}

// Hex Encode in_filename to out_filename
void DoHexEncode(int len, unsigned char *in_string)
{
    HexEncoder hexEncoder;
    hexEncoder.Put(in_string, len);
    hexEncoder.MessageEnd();
    hexEncoder.Get(in_string, len * 2);
}

void GenAKey(int keylen, unsigned char *key)
{
    AutoSeededRandomPool rng;
    rng.GenerateBlock(key, keylen);
}

void GenSKey(int keylen, char *passphrase, unsigned char *key)
{
    int slen = (int) lstrlen( (LPCTSTR) passphrase);
    RandomPool rng;
    rng.Put( (unsigned char *) passphrase, slen);
    rng.GenerateBlock(key, keylen);
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
    cout << "Usage: NGenKeys [-BA  KeySize OutputFileName                  |\n";
    cout << "                 -BS  KeySize PassPhraseString OutputFileName |\n";
    cout << "                 -HA  KeySize OutputFileName                  |\n";
    cout << "                 -HS  KeySize PassPhraseString OutputFileName |\n";
    cout << "                 -h\n";
    cout << "                ]\n\n";
    cout << "-BA    Generate Auto Random Key of size KeySize,\n";
    cout << "        write as binary data to OutputFilename\n\n";
    cout << "-BS    Generate PassPhrase Seeded Key of size KeySize,\n";
    cout << "        write as binary data to OutputFilename\n\n";
    cout << "-HA    Generate Auto Random Key of size KeySize,\n";
    cout << "        write as hex encoded string to OutputFilename\n\n";
    cout << "-HS    Generate PassPhrase Seeded Key of size KeySize,\n";
    cout << "        write as hex encoded string to OutputFilename\n\n";
    cout << "-h     Help: this screen.\n\n";
    cout << "Without parms runs interactively, i.e. prompts for necessary parameters.\n\n";
}
