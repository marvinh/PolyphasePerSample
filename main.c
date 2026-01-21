//
//  main.c
//  StackExchangePolyPhase
//
//  Created by Marvin Harootoonyan on 1/17/26.
//

#include <stdio.h>
#include <math.h>
#include "coef.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"


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


// good for using in a for loop modulating srcRatio unrolled of course
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


int main(int argc, char** argv)
{



    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalPCMFrameCount;
    
    
   float* pSampleData = drwav_open_file_and_read_pcm_frames_f32("./saw-256.wav", &channels, &sampleRate, &totalPCMFrameCount, NULL);
    
   if (pSampleData == NULL) {
       // Error opening and reading WAV file.
       printf("error check if you have file");
       return 0;
   }    


    drwav_data_format format;
    drwav wav;
    float tempFrames[4096];
    drwav_uint64 totalFramesToWrite;
    drwav_uint64 totalFramesWritten = 0;
    

    if (argc < 2) {
        printf("No output file specified.\n");
        return -1;
    }

    format.container     = drwav_container_riff;
    format.format        = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels      = 1;
    format.sampleRate    = 44100;
    format.bitsPerSample = 32;
    if (!drwav_init_file_write(&wav, argv[1], &format, NULL)) {
        printf("Failed to open file.\n");
        return -1;
    }

    totalFramesToWrite = format.sampleRate * 1;
    totalFramesWritten = 0;

    uint64_t samplePosition = 0;

    float t = 0;

    while (totalFramesToWrite > totalFramesWritten) {
        drwav_uint64 framesRemaining = totalFramesToWrite - totalFramesWritten;
        drwav_uint64 framesToWriteNow = drwav_countof(tempFrames) / format.channels;
        if (framesToWriteNow > framesRemaining) {
            framesToWriteNow = framesRemaining;
        }
        double srcRatio = 5.0;

        for(int n = 0; n < framesToWriteNow; n++) {

            float frequency = 2.0;
            float s = (float)(sin(3.1415965*2 * t * frequency));
            t += 1.0/(float)(sampleRate);

            while(t > 1.0) {
                t -= 1.0;
            }

            float modAmount = 0.1;// modulate with a sine

            tempFrames[n] = srcSampleBySample(srcRatio*exp(s*modAmount), pSampleData, totalPCMFrameCount, &samplePosition);
        
        }
        
        drwav_write_pcm_frames(&wav, framesToWriteNow, tempFrames);

        totalFramesWritten += framesToWriteNow;
    }

    drwav_uninit(&wav);
    drwav_free(pSampleData, NULL);

    return 0;
}