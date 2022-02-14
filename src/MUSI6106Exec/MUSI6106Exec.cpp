#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

void example1 ();
void example2 ();
void test1 ();
void test2 ();
void test3_FIR ();
void test3_IIR ();
void test4_FIR ();
void test4_IIR ();
void test5();

typedef struct arg{
    std::string inputAudioPath;
    std::string outputAudioPath;
    int blockSize;
    CCombFilterIf::CombFilterType_t filterType;
    float gain;
    float delay;};

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    example1();
    cout << "Example 1" << endl;
    example2();
    cout << "Example 2" << endl;
    test1 ();
    cout << "Test 1" << endl;
    test2 ();
    cout << "Test 2" << endl;
    test3_FIR ();
    cout << "Test 3 FIR" << endl;
    test3_IIR ();
    cout << "Test 3 IIR" << endl;
    test4_FIR ();
    cout << "Test 4 FIR" << endl;
    test4_IIR ();
    cout << "Test 4 IIR" << endl;
    test5 ();
    cout << "Test 5" << endl;
    showClInfo();
    return 0;
}

int filtering(arg argall){
    std::string sInputFilePath = argall.inputAudioPath;                 //!< file paths
    std::string sOutputFilePath = argall.outputAudioPath;
    std::string sOutputFilePathText = sOutputFilePath + ".txt";

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfOutputData = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phOutputAudioFile = 0;
    std::fstream hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    CCombFilterIf *phCombFilter = 0;
    float fDelayLength = argall.delay;
    float fGain = argall.gain;
    float fMaxDelayLengthInS = 1;


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

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePathText.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }

    // open the output wave file
    CAudioFileIf::create(phOutputAudioFile);
    phOutputAudioFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phOutputAudioFile->isOpen())
    {
        cout << "Output Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];

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

    ppfOutputData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfOutputData[i] = new float[kBlockSize];

    if (ppfOutputData == 0)
    {
        CAudioFileIf::destroy(phOutputAudioFile);
        hOutputFile.close();
        return -1;
    }
    if (ppfOutputData[0] == 0)
    {
        CAudioFileIf::destroy(phOutputAudioFile);
        hOutputFile.close();
        return -1;
    }

    Error_t error = CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombFIR, fMaxDelayLengthInS, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);

    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);

        cout << "\r" << "reading and writing";

        phCombFilter->process(ppfAudioData, ppfOutputData, iNumFrames);
        phOutputAudioFile->writeData(ppfOutputData, iNumFrames);

        // write
        for (int i = 0; i < iNumFrames; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile << ppfOutputData[c][i] << "\t";
            }
            hOutputFile << endl;
        }
    }

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();
    phOutputAudioFile->closeFile();
    CAudioFileIf::destroy(phOutputAudioFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfOutputData[i];
    delete[] ppfOutputData;
    ppfOutputData = 0;

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

void example1 () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/MajorTom16.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/MajorTom16FIR.wav";
    argall.blockSize = 1024;
    argall.filterType = CCombFilterIf::kCombFIR;
    argall.gain = 0.5;
    argall.delay = 0.2;
    filtering(argall);
}

void example2 () {
    cout << "Example 2" << endl;
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/sweep.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/sweepIIR.wav";
    argall.blockSize = 1024;
    argall.filterType = CCombFilterIf::kCombIIR;
    argall.gain = 0.5;
    argall.delay = 0.5;
    filtering(argall);
}

// FIR: Output is zero if input freq matches feedforward
void test1 () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/Asine.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/AsineFIR.wav";
    argall.blockSize = 1024;
    argall.filterType = CCombFilterIf::kCombFIR;
    argall.gain = -1;
    argall.delay = 1;
    filtering(argall);
}

// IIR: amount of magnitude increase/decrease if input freq matches feedback
void test2 () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/Asine.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/AsineIIR.wav";
    argall.blockSize = 1024;
    argall.filterType = CCombFilterIf::kCombIIR;
    argall.gain = -1;
    argall.delay = 0;
    filtering(argall);
}

//// FIR/IIR: correct result for VARYING input block size
void test3_FIR () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/Asine.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/AsineFIRBlock.wav";
    argall.blockSize = 512; //1024
    argall.filterType = CCombFilterIf::kCombFIR;
    argall.gain = 0.5;
    argall.delay = 0.5;
    filtering(argall);
}

void test3_IIR () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/Asine.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/AsineIIRBlock.wav";
    argall.blockSize = 512; //1024
    argall.filterType = CCombFilterIf::kCombIIR;
    argall.gain = 0.5;
    argall.delay = 0.5;
    filtering(argall);
}

// FIR/IIR: correct processing for zero input signal
void test4_FIR () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/AsineFIR.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/ZeroOutputFIR.wav";
    argall.blockSize = 1024;
    argall.filterType = CCombFilterIf::kCombFIR;
    argall.gain = 0.5;
    argall.delay = 0.5;
    filtering(argall);
}

void test4_IIR () {
    arg argall;
    argall.inputAudioPath = "/Users/roseqsun/Desktop/AsineFIR.wav";
    argall.outputAudioPath = "/Users/roseqsun/Desktop/ZeroOutputIIR.wav";
    argall.blockSize = 1024;
    argall.filterType = CCombFilterIf::kCombIIR;
    argall.gain = 0.5;
    argall.delay = 0.5;
    filtering(argall);
}

// One more additional MEANINGFUL test to verify your filter implementation
// FIR/IIR: correct processing for zero gain and zero delay
void test5 () {
    arg argalll;
    argalll.inputAudioPath = "/Users/roseqsun/Desktop/Asine.wav";
    argalll.outputAudioPath = "/Users/roseqsun/Desktop/ZeroProcessing.wav";
    argalll.blockSize = 1024;
    argalll.filterType = CCombFilterIf::kCombIIR;
    argalll.gain = 0;
    argalll.delay = 0;
    filtering(argalll);
}