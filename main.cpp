#include "gus.h"
#include <cmath>
#include <conio.h>
#include <cstdint>
#include <cstdio>

#define WAV_FILE "rock22.wav"

void printVolFreq(int16_t vol, int32_t freq, bool playing)
{
    printf("\rvol=%d, freq=%d, %s        ", int(vol), freq, playing ? "playing" : "paused");
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    uint32_t port = GUSFind();
    if (!port)
    {
        printf("GUS not found.\n");
        return 1;
    }
    uint32_t gusMemSize = GUSFindMem();
    printf("%u kB detected\n", gusMemSize >> 10);
    GUSReset();

    printf("Slowly loading " WAV_FILE "...\n");
    const uint8_t voice = 0;
    FILE* fin = fopen(WAV_FILE, "rb");
    if (!fin)
    {
        printf("FAILED\n");
        return 1;
    }
    fseek(fin, 44, SEEK_SET);
    uint8_t b;
    uint32_t sampleSize = 0;
    while (fread(&b, 1, 1, fin))
    {
        b -= 128;
        GUSPoke(sampleSize++, b);
    }
    fclose(fin);
    printf("Done: %u bytes\n", sampleSize);
    printf("Esc: quit, P: pause, I/K: change volume, O/L: change frequency\n");

    int16_t vol = 511;
    int32_t freq = 22050;
    bool playing = true;

    printVolFreq(vol, freq, playing);
    GUSSetVolume(voice, vol);
    GUSSetBalance(voice, 7);
    GUSSetFreq(voice, freq);
    GUSPlayVoice(voice, GUS_LOOP_ENABLE, 0, 0, sampleSize);

    uint32_t pos = 0;

    for (;;)
    {
        if (kbhit())
        {
            int key = getch();
            if (key == 27)
            {
                printf("\n");
                break;
            }
            else if (key == 'i')
            {
                vol += 8;
                if (vol > 511)
                    vol = 511;
                printVolFreq(vol, freq, playing);
                GUSSetVolume(voice, vol);
            }
            else if (key == 'k')
            {
                vol -= 8;
                if (vol < 0)
                    vol = 0;
                printVolFreq(vol, freq, playing);
                GUSSetVolume(voice, vol);
            }
            else if (key == 'o')
            {
                freq += 10;
                printVolFreq(vol, freq, playing);
                GUSSetFreq(voice, freq);
            }
            else if (key == 'l')
            {
                freq -= 10;
                if (freq < 0)
                    freq = 0;
                printVolFreq(vol, freq, playing);
                GUSSetFreq(voice, freq);
            }
            else if (key == 'p')
            {
                if (playing)
                {
                    pos = GUSVoicePos(voice);
                    GUSVoiceControl(voice, GUS_VOICE_STOP);
                }
                else
                {
                    GUSPlayVoice(voice, GUS_LOOP_ENABLE, pos, 0, sampleSize);
                }
                playing = !playing;
                printVolFreq(vol, freq, playing);
            }
        }
    }
    GUSReset();
    return 0;
}
