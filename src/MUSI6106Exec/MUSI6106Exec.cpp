#include <iostream>
#include <ctime>
#include <cmath>


#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

//void example1 ();
//void example2 ();
void sineWave();
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

/////////////////////////////////////////////////////////////////////////////////

// main function
int main(int argc, char* argv[])
{
    if (argc == 0) {
//        example1();
//        example2();
        test1 ();
        test2 ();
        test3_FIR ();
        test3_IIR ();
        test4_FIR ();
        test4_IIR ();
        test5 ();
        showClInfo();
    }
    else {
        arg argall;
        argall.inputAudioPath = argv[1];
        argall.outputAudioPath = argall.inputAudioPath + "filtered.wav";
        argall.blockSize = 1024;
        if (argv[2]=="fir") {
            argall.filterType = CCombFilterIf::kCombFIR;
        }
        else if (argv[2]=="iir") {
            argall.filterType = CCombFilterIf::kCombIIR;
        }
        argall.gain = atof(argv[4]);
        argall.delay = atof(argv[3]);
        return filtering(argall);
    }
}

void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

/////////////////////////////////////////////////////////////////////////////////


void example1 () {
    cout << "Example 1" << endl;
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

const double PI  =3.141592653589793238463;

void sinWave(float*& pfSineBuffer, float fSampleRateInHz, float fLenInSec, float fFreq){
    int iLenInSample = fSampleRateInHz * fLenInSec;
    pfSineBuffer = new float[iLenInSample];
    for (int i = 0; i < iLenInSample; i++){
        pfSineBuffer[i] = 1 * sin((2 * PI * fFreq / fSampleRateInHz) * i);
    }
}

// FIR: Output is zero if input freq matches feedforward
void test1 () {
    cout << "Test 1" << endl;
    static const int kBlockSize = 1024;
    float **ppfOutputData = 0;

    float **ppfSineBuffer = 0;
    float fLenInSec = 2;
    float fFreq = 440;
    float fSampleRateInHz = 44100;

    CCombFilterIf *phCombFilter = 0;
    float fDelayLength = 1 / fFreq / 2;
    float fGain = 0.5;
    float fMaxDelayLengthInS = 1;

    int iNumChannels = 1;
    int iNumBlocks = round(fSampleRateInHz * fLenInSec / kBlockSize);

    ppfOutputData = new float*[1];
    ppfOutputData[0] = new float[kBlockSize];

    ppfSineBuffer = new float*[1];
    sinWave(ppfSineBuffer[0], fSampleRateInHz, fLenInSec, fFreq);
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombFIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phCombFilter->process(ppfSineBuffer, ppfOutputData, kBlockSize);
    for (int i = 0; i < iNumChannels; i++) {
        for (int j = fSampleRateInHz * fLenInSec; j < kBlockSize*iNumBlocks; j++) {
            if (abs(ppfOutputData[i][j]) > 1e-3) {
                cout << "Test 1 failed with sample" << ppfOutputData[i][j] << endl;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    cout << "Test 1 passed" << endl;
    delete [] ppfOutputData[0];
    delete [] ppfOutputData;
    delete [] ppfSineBuffer[0];
    delete [] ppfSineBuffer;
}

// IIR: amount of magnitude increase/decrease if input freq matches feedback
void test2 () {
    cout << "Test 2" << endl;
    static const int kBlockSize = 1024;
    float **ppfOutputData = 0;

    float **ppfSineBuffer = 0;
    float fLenInSec = 2;
    float fFreq = 440;
    float fSampleRateInHz = 44100;

    CCombFilterIf *phCombFilter = 0;
    float fDelayLength = 1 / fFreq;
    float fGain = 0.5;
    float fMaxDelayLengthInS = 1;

    int iNumChannels = 1;
    int iNumBlocks = round(fSampleRateInHz * fLenInSec / kBlockSize);

    ppfOutputData = new float*[1];
    ppfOutputData[0] = new float[kBlockSize];

    ppfSineBuffer = new float*[1];
    sinWave(ppfSineBuffer[0], fSampleRateInHz, fLenInSec, fFreq);
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombIIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phCombFilter->process(ppfSineBuffer, ppfOutputData, kBlockSize);
    for (int i = 0; i < iNumChannels; i++) {
        for (int j = fSampleRateInHz * fLenInSec; j < kBlockSize*iNumBlocks; j++) {
            if (abs(ppfOutputData[i][j]) < abs(ppfSineBuffer[i][j])) {
                cout << "Test 2 failed with sample" << ppfOutputData[i][j] << endl;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    cout << "Test 2 passed" << endl;
    delete [] ppfOutputData[0];
    delete [] ppfOutputData;
    delete [] ppfSineBuffer[0];
    delete [] ppfSineBuffer;

}

// FIR/IIR: correct result for VARYING input block size
void test3_FIR () {
    cout << "Test 3 FIR" << endl;
    int kBlockSize = 1024, kNewBlockSize = 2048;
    float **ppfOutputData = 0, **ppfNewOutputData = 0;

    float **ppfSineBuffer = 0;
    float fLenInSec = 2;
    float fFreq = 440;
    float fSampleRateInHz = 44100;

    CCombFilterIf *phCombFilter = 0;
    CCombFilterIf *phNewCombFiler = 0;
    float fDelayLength = 0.2;
    float fGain = 0.5;
    float fMaxDelayLengthInS = 1;

    int iNumChannels = 1;
    int iNumBlocks = round(fSampleRateInHz * fLenInSec / kBlockSize), iNewNumBlocks = round(fSampleRateInHz * fLenInSec / kNewBlockSize);

    ppfOutputData = new float*[1];
    ppfOutputData[0] = new float[kBlockSize];
    ppfNewOutputData = new float*[1];
    ppfNewOutputData[0] = new float[kNewBlockSize];

    ppfSineBuffer = new float*[1];
    sinWave(ppfSineBuffer[0], fSampleRateInHz, fLenInSec, fFreq);

    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombFIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phCombFilter->process(ppfSineBuffer, ppfOutputData, kBlockSize);

    CCombFilterIf::create(phNewCombFiler);
    phNewCombFiler->init(CCombFilterIf::kCombFIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phNewCombFiler->setParam(CCombFilterIf::kParamGain, fGain);
    phNewCombFiler->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phNewCombFiler->process(ppfSineBuffer, ppfNewOutputData, kNewBlockSize);

    for (int i = 0; i < iNumChannels; i++) {
        for (int j = 0; j < kBlockSize && j<kNewBlockSize; j++) {
            if (ppfOutputData[i][j] != ppfNewOutputData[i][j]) {
                cout << "Test 3 FIR failed with sample" << ppfOutputData[i][j] <<" "<< ppfNewOutputData[i][j] << endl;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    CCombFilterIf::destroy(phNewCombFiler);
    cout << "Test 3 FIR passed" << endl;
    delete [] ppfOutputData[0];
    delete [] ppfOutputData;
    delete [] ppfNewOutputData[0];
    delete [] ppfNewOutputData;
    delete [] ppfSineBuffer[0];
    delete [] ppfSineBuffer;
}

void test3_IIR () {
    cout << "Test 3 IIR" << endl;
    int kBlockSize = 1024, kNewBlockSize = 2048;
    float **ppfOutputData = 0, **ppfNewOutputData = 0;

    float **ppfSineBuffer = 0;
    float fLenInSec = 2;
    float fFreq = 440;
    float fSampleRateInHz = 44100;

    CCombFilterIf *phCombFilter = 0;
    CCombFilterIf *phNewCombFiler = 0;
    float fDelayLength = 0.2;
    float fGain = 0.5;
    float fMaxDelayLengthInS = 1;

    int iNumChannels = 1;
    int iNumBlocks = round(fSampleRateInHz * fLenInSec / kBlockSize), iNewNumBlocks = round(fSampleRateInHz * fLenInSec / kNewBlockSize);

    ppfOutputData = new float*[1];
    ppfOutputData[0] = new float[kBlockSize];
    ppfNewOutputData = new float*[1];
    ppfNewOutputData[0] = new float[kNewBlockSize];

    ppfSineBuffer = new float*[1];
    sinWave(ppfSineBuffer[0], fSampleRateInHz, fLenInSec, fFreq);

    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombIIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phCombFilter->process(ppfSineBuffer, ppfOutputData, kBlockSize);

    CCombFilterIf::create(phNewCombFiler);
    phNewCombFiler->init(CCombFilterIf::kCombIIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phNewCombFiler->setParam(CCombFilterIf::kParamGain, fGain);
    phNewCombFiler->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phNewCombFiler->process(ppfSineBuffer, ppfNewOutputData, kNewBlockSize);

    for (int i = 0; i < iNumChannels; i++) {
        for (int j = 0; j < kBlockSize && j<kNewBlockSize; j++) {
            if (ppfOutputData[i][j] != ppfNewOutputData[i][j]) {
                cout << "Test 3 IIR failed with sample" << ppfOutputData[i][j] <<" "<< ppfNewOutputData[i][j] << endl;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    CCombFilterIf::destroy(phNewCombFiler);
    cout << "Test 3 IIR passed" << endl;
    delete [] ppfOutputData[0];
    delete [] ppfOutputData;
    delete [] ppfNewOutputData[0];
    delete [] ppfNewOutputData;
    delete [] ppfSineBuffer[0];
    delete [] ppfSineBuffer;
}

// FIR/IIR: correct processing for zero input signal
void test4_FIR () {
    cout << "Test 4 FIR" << endl;
    static const int kBlockSize = 1024;
    float **ppfOutputData = 0;

    float **ppfSineBuffer = 0;
    float fLenInSec = 2;
    float fFreq = 440;
    float fSampleRateInHz = 44100;

    CCombFilterIf *phCombFilter = 0;
    float fDelayLength = 1 / fFreq;
    float fGain = 0.5;
    float fMaxDelayLengthInS = 1;

    int iNumChannels = 1;
    int iNumBlocks = round(fSampleRateInHz * fLenInSec / kBlockSize);

    ppfOutputData = new float*[1];
    ppfOutputData[0] = new float[kBlockSize];

    ppfSineBuffer = new float*[1];
    ppfSineBuffer[0] = new float[kBlockSize];
    for (int i=0; i<kBlockSize; ++i){
        ppfSineBuffer[0][i] = 0;
    }
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombFIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phCombFilter->process(ppfSineBuffer, ppfOutputData, kBlockSize);
    for (int i = 0; i < iNumChannels; i++) {
        for (int j = 0; j < kBlockSize*iNumBlocks; j++) {
            if (abs(ppfOutputData[i][j]) > 1e-3) {
                cout << "Test 4 FIR failed with sample" << ppfOutputData[i][j] << endl;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    cout << "Test 4 FIR passed" << endl;
    delete [] ppfOutputData[0];
    delete [] ppfOutputData;
    delete [] ppfSineBuffer[0];
    delete [] ppfSineBuffer;
}

void test4_IIR () {
    cout << "Test 4 IIR" << endl;
    static const int kBlockSize = 1024;
    float **ppfOutputData = 0;

    float **ppfSineBuffer = 0;
    float fLenInSec = 2;
    float fFreq = 440;
    float fSampleRateInHz = 44100;

    CCombFilterIf *phCombFilter = 0;
    float fDelayLength = 1 / fFreq;
    float fGain = 0.5;
    float fMaxDelayLengthInS = 1;

    int iNumChannels = 1;
    int iNumBlocks = round(fSampleRateInHz * fLenInSec / kBlockSize);

    ppfOutputData = new float*[1];
    ppfOutputData[0] = new float[kBlockSize];

    ppfSineBuffer = new float*[1];
    ppfSineBuffer[0] = new float[kBlockSize];
    for (int i=0; i<kBlockSize; ++i){
        ppfSineBuffer[0][i] = 0;
    }

    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(CCombFilterIf::kCombIIR, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayLength);
    phCombFilter->process(ppfSineBuffer, ppfOutputData, kBlockSize);
    for (int i = 0; i < iNumChannels; i++) {
        for (int j = 0; j < kBlockSize*iNumBlocks; j++) {
            if (abs(ppfOutputData[i][j]) > 1e-3) {
                cout << "Test 4 IIR failed with sample" << ppfOutputData[i][j] << endl;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    cout << "Test 4 IIR passed" << endl;
    delete [] ppfOutputData[0];
    delete [] ppfOutputData;
    delete [] ppfSineBuffer[0];
    delete [] ppfSineBuffer;
}

// One more additional MEANINGFUL test to verify your filter implementation
// FIR/IIR: correct processing for zero delay
void test5 () {
}
