//
// Created by Rose Sun on 2/12/22.
//

#ifndef MUSI6106_COMBFILTER_H
#define MUSI6106_COMBFILTER_H

#include "CombFilterIf.h"
#include "RingBuffer.h"

class CCombFilterBase {
public:
    CCombFilterBase (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels);

    virtual ~CCombFilterBase ();

    Error_t setParam (CCombFilterIf::FilterParam_t eParam, float fParamValue);
    float   getParam (CCombFilterIf::FilterParam_t eParam) const;

    virtual Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) = 0;

    void reset();
protected:
    float fMaxDelayLengthInS;
    float fSampleRateInHz;
    float fDelayLengthInS;

    float fGain;
    int iNumChannels;

    long delayLength;
    long bufferLength;  // in samples
    CRingBuffer<float>** ppRingBuffer;
};

class CCombFilterFIR : public CCombFilterBase {
public:
    CCombFilterFIR (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
            : CCombFilterBase(fMaxDelayLengthInS, fSampleRateInHz, iNumChannels)
    {
    };
    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) override;

};

class CCombFilterIIR : public CCombFilterBase {
public:
    CCombFilterIIR (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
    : CCombFilterBase(fMaxDelayLengthInS, fSampleRateInHz, iNumChannels)
    {
    };
    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) override;

};


#endif //MUSI6106_COMBFILTER_H
