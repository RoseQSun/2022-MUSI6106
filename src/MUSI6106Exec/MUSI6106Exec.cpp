
#include <iostream>
#include <ctime>
#include <fstream>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 3)
        return -1;
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead, &stFileSpec);
    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath);
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    long long iLength = 0;
    phAudioFile->getLength(iLength);
    auto iNumChannels = stFileSpec.iNumChannels;


    ppfAudioData = new float* [iNumChannels];
    for (int i=0; i<iNumChannels; ++i)
        ppfAudioData[i] = new float[kBlockSize];

    ////////////////////////////////////////iLength//////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    for (long long pos=0; pos<iLength; pos+=kBlockSize) {
        long long iReadLength = 0;
        if (iLength - pos <= kBlockSize)
            iReadLength = kBlockSize;
        else
            iReadLength = iLength - pos;
        phAudioFile->readData(ppfAudioData, iReadLength);
        for (long long i=0; i<iReadLength; ++i) {
            for (int c=0; c<iNumChannels; ++c) {
                hOutputFile << ppfAudioData[c][i];
                if (c < iNumChannels - 1)
                    hOutputFile << ", ";
                else
                    hOutputFile << "\n";
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory
    for (int i=0; i<iNumChannels; ++i)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = nullptr;
    phAudioFile->closeFile();
    hOutputFile.close();
    // all done
    return 0;
}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

