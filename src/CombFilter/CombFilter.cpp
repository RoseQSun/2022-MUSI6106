//
// Created by Rose Sun on 2/12/22.
//

#include "CombFilter.h"
#include "RingBuffer.h"
#include "ErrorDef.h"

CCombFilterBase::CCombFilterBase (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels) :
fMaxDelayLengthInS(fMaxDelayLengthInS),
fSampleRateInHz(fSampleRateInHz),
fDelayLengthInS(0.f),
iNumChannels(iNumChannels)
{
    this->reset ();
    bufferLength = static_cast<long>(round(fMaxDelayLengthInS * fSampleRateInHz));

    ppRingBuffer = new CRingBuffer<float>*[iNumChannels];
    for (int i=0; i<iNumChannels; ++i)
        ppRingBuffer[i] = new CRingBuffer<float>(bufferLength);
}

Error_t CCombFilterBase::setParam (CCombFilterIf::FilterParam_t eParam, float fParamValue)
{
    switch (eParam) {
        case CCombFilterIf::FilterParam_t::kParamDelay:
            if (fParamValue > fMaxDelayLengthInS)
                return Error_t::kFunctionInvalidArgsError;
            fDelayLengthInS = fParamValue;
            delayLength = static_cast<long>(round(fSampleRateInHz * fParamValue));
            for (int i=0; i < iNumChannels; ++i)
                ppRingBuffer[i]->setWriteIdx(1.F*delayLength + ppRingBuffer[i]->getReadIdx());
            break;
        case CCombFilterIf::FilterParam_t::kParamGain:
            if (fGain < 0)
                return Error_t::kFunctionInvalidArgsError;
            fGain = fParamValue;
            break;
    }
    return Error_t::kNoError;
}

float CCombFilterBase::getParam (CCombFilterIf::FilterParam_t eParam) const
{
    switch (eParam) {
        case CCombFilterIf::FilterParam_t::kParamDelay:
            return fDelayLengthInS;

        case CCombFilterIf::FilterParam_t::kParamGain:
            return fGain;
    }
    return -1.f;
}

CCombFilterBase::~CCombFilterBase ()
{
    this->reset ();
}

void CCombFilterBase::reset() {
    if (ppRingBuffer) {
        for (int i=0; i<iNumChannels; ++i)
            delete ppRingBuffer[i];
        delete [] ppRingBuffer;
        ppRingBuffer = nullptr;
    }
    fGain = 0.f;
    bufferLength = 0;
}


Error_t CCombFilterFIR::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    for (int i=0; i<iNumberOfFrames; ++i){
        for (int j=0; j<iNumChannels; ++j){
            ppRingBuffer[j]->putPostInc(ppfInputBuffer[j][i]);
            ppfOutputBuffer[j][i] = ppfInputBuffer[j][i] + fGain * ppRingBuffer[j]->getPostInc();
        }
    }
    return Error_t::kNoError;
}

Error_t CCombFilterIIR::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    for (int i=0; i<iNumChannels; ++i){
        for (int j=0; j<iNumberOfFrames; ++j){
            ppfOutputBuffer[j][i] = ppfInputBuffer[j][i] + fGain * ppRingBuffer[j]->getPostInc();
            ppRingBuffer[j]->putPostInc(ppfOutputBuffer[j][i]);
        }
    }
    return Error_t::kNoError;
}

