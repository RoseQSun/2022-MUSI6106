
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Fft.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,sOutputFilePath;

    static const int        kBlockSize = 1024;
    int iBlockSize = 4096;
    int iHopSize = 2048;

    clock_t                 time = 0;

    float **ppfAudioData = 0;
    CFft *pFft = 0;
    float **ppfBlock = 0;
    CFft::complex_t **ppFftOut = 0;
    float **ppfMagnitude = 0;

    float **ppfBuffer = 0;

    CAudioFileIf* phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;


    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        if (argc == 3) {
            iBlockSize = atoi(argv[2]);
        }
        if (argc == 4) {
            iBlockSize = atoi(argv[2]);
            iHopSize = atoi(argv[3]);
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);
    assert(stFileSpec.iNumChannels == 1);

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfAudioData[i] = new float[iBlockSize];
        for (int j = 0; j < iBlockSize; j++){
            ppfAudioData[i][j] = 0;
        }
    }

    if (ppfAudioData == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hOutputFile.close();
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hOutputFile.close();
        return -1;
    }

    ppfBuffer = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        ppfBuffer[i] = new float[iBlockSize];
        for (int j = 0; j < iBlockSize; j++){
            ppfBuffer[i][j] = 0;
        }
    }

    ppfBlock = new float*[1];
    ppfBlock[0] = new float[iBlockSize];

    ppFftOut = new float*[1];
    ppFftOut[0] = new CFft::complex_t[iBlockSize];

    ppfMagnitude = new float*[1];
    ppfMagnitude[0] = new float[iBlockSize];


    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    // initializing
    CFft::createInstance(pFft);
    pFft->initInstance(iBlockSize,1,CFft::kWindowHann,CFft::kPreWindow);

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = iHopSize;
        int iReadIdx = 0;
        int iWriteIdx = iBlockSize - iHopSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);

        for (int i = 0; i < iNumFrames; i++){
            ppfBuffer[0][iWriteIdx] = ppfAudioData[0][i];
            iWriteIdx = (iWriteIdx + 1) % iBlockSize;
        }
        for (int i = 0; i < iBlockSize - iHopSize + iNumFrames; i++) {
            ppfBlock[0][i] = ppfBuffer[0][iReadIdx];
            iReadIdx = (iReadIdx + 1) % iBlockSize;
        }
        pFft->doFft(ppFftOut[0],ppfBlock[0]);
        pFft->getMagnitude(ppfMagnitude[0], ppFftOut[0]);

        cout << "\r" << "reading and writing";

        // write
        int iMagnitudeLen = pFft->getLength(CFft::kLengthMagnitude);
        for (int i = 0; i < iMagnitudeLen; i++)
        {
            hOutputFile << ppfMagnitude[0][i] << "\t";
        }
        hOutputFile << endl;

    }

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfBuffer[i];
    delete[] ppfBuffer;
    ppfBuffer = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfBlock[i];
    delete[] ppfBlock;
    ppfBlock = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppFftOut[i];
    delete[] ppFftOut;
    ppFftOut = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfMagnitude[i];
    delete[] ppfMagnitude;
    ppfMagnitude = 0;

    // all done
    return 0;

}

void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;

    return;
}

