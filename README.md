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
