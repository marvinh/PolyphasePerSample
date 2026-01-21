Driver Proper phase selection and interpolation posted as answer: 

Per sample variation.


https://dsp.stackexchange.com/questions/99566/polyphase-fir-for-audio-signal-how-do-you-upsample-pitch-down-without-have-it


<img width="429" height="183" alt="Screenshot 2026-01-17 at 11 56 46 PM" src="https://github.com/user-attachments/assets/064e6ea8-99c0-4f79-a422-f5a4f6878298" />


```
    //my mistake was also in how i would wrap
    //we wrap (sample_length - FIR_LEN -1 )
    //wrapping at sample length will cause exaggerated filtering as well.

    *samplePosition += increment;

    uint64_t wrap = (uint64_t)(nInputSamples-(SRC_FIR_LENGTH-1));

    if((*samplePosition>>32) >= wrap) {
        *samplePosition -= (wrap<<32);
    }
    
    return out;
```



The functions:

```
//Look into code for usage original is here: https://github.com/marvinh/StackExchangeDSPPolyphase
//original by robert bristow johnson 
void convertSampleRate(float* x, float* y, float srcRatio, uint32_t nInputSamples)
    {
    float fixedScaler = 1.0/(float)(0x4000000);
    uint64_t increment = (uint64_t)floor((double)(0x0000000100000000UL)/srcRatio);

    uint32_t nOutputSamples = (uint32_t)floor((double)(nInputSamples-(SRC_FIR_LENGTH-1))*srcRatio);
    uint64_t precisionIndex = 0x0000000100000000UL * (SRC_FIR_LENGTH-1);
    uint32_t prev = 0;
    
    for(uint32_t n=0; n<nOutputSamples; n++)
        {
        uint32_t intIndex = (uint32_t)(precisionIndex>>32);
        int phaseIndex = (int)(precisionIndex>>(32-6)) & (SRC_DECIMATION-1);
        float linearInterpCoef = (float)(precisionIndex & 0x0000000003FFFFFFUL) * fixedScaler;
        
        int iFIR = phaseIndex * SRC_FIR_LENGTH;

        float y0 = 0.0;                 // near linear interpolation value
        uint32_t i = intIndex;
        for (int m=SRC_FIR_LENGTH; m>0; m--)
            {
            y0 += SRC_FIR_coefs[iFIR++] * x[(i--+nInputSamples)%nInputSamples];
            }
        
        float y1 = 0.0;                 // far linear interpolation value
        i = intIndex;                   // reset i but note iFIR just keeps incrementing
        for (int m=SRC_FIR_LENGTH; m>0; m--)
            {
            y1 += SRC_FIR_coefs[iFIR++] * x[(i--+nInputSamples)%nInputSamples];
            }
            
            y[n] = y0 + linearInterpCoef*(y1-y0);
            
    
        precisionIndex += increment;
    }
}


// Look into main.c code for usage: modulating ratio.
// Purpose: using in a for loop modulating srcRatio unrolled of course
float srcSampleBySample(double srcRatio, float * x, uint32_t nInputSamples, uint64_t * samplePosition ) {

    float fixedScaler = 1.0/(float)(0x4000000);
    uint64_t increment = (uint64_t)floor((double)(0x0000000100000000UL)/srcRatio);
    uint64_t precisionIndex = *samplePosition + (0x0000000100000000UL * (SRC_FIR_LENGTH-1));
    uint32_t prev = 0;
    float out = 0.0f;
    uint32_t intIndex = (uint32_t)(precisionIndex>>32);
    int phaseIndex = (int)(precisionIndex>>(32-6)) & (SRC_DECIMATION-1);
    float linearInterpCoef = (float)(precisionIndex & 0x0000000003FFFFFFUL) * fixedScaler;
    int iFIR = phaseIndex * SRC_FIR_LENGTH;
    float y0 = 0.0;                 // near linear interpolation value
    uint32_t i = intIndex;
    for (int m=SRC_FIR_LENGTH; m>0; m--)
    {
        y0 += SRC_FIR_coefs[iFIR++] * x[(i--+nInputSamples)%nInputSamples];
    }
    
    float y1 = 0.0;                 // far linear interpolation value
    i = intIndex;                   // reset i but note iFIR just keeps incrementing
    for (int m=SRC_FIR_LENGTH; m>0; m--)
    {
        y1 += SRC_FIR_coefs[iFIR++] * x[(i--+nInputSamples)%nInputSamples];
    }
        
    out = y0 + linearInterpCoef*(y1-y0);

    *samplePosition += increment;

    uint64_t wrap = (uint64_t)(nInputSamples-(SRC_FIR_LENGTH-1));

    if((*samplePosition>>32) >= wrap) {
        *samplePosition -= (wrap<<32);
    }
    
    return out;
}
```
