//
//  main.cpp
//  midiParser
//
//  Created by William Henning on 5/30/15.
//  Copyright (c) 2015 William Henning. All rights reserved.
//

#include "midi/include/MidiFile.h"
#include "midi/include/Options.h"
#include "testPerlin.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits.h>
#include <sys/time.h>
#include <unistd.h>

#define CASE_RETURN(err)                                                       \
  case (err):                                                                  \
    return "##err"
const char *al_err_str(ALenum err) {
  switch (err) {
    CASE_RETURN(AL_NO_ERROR);
    CASE_RETURN(AL_INVALID_NAME);
    CASE_RETURN(AL_INVALID_ENUM);
    CASE_RETURN(AL_INVALID_VALUE);
    CASE_RETURN(AL_INVALID_OPERATION);
    CASE_RETURN(AL_OUT_OF_MEMORY);
  }
  return "unknown";
}
#undef CASE_RETURN

#define __al_check_error(file, line)                                           \
  do {                                                                         \
    ALenum err = alGetError();                                                 \
    for (; err != AL_NO_ERROR; err = alGetError()) {                           \
      std::cerr << "AL Error " << al_err_str(err) << " at " << file << ":"     \
                << line << std::endl;                                          \
    }                                                                          \
  } while (0)

#define al_check_error() __al_check_error(__FILE__, __LINE__)

const int sampleRate = 22050;

void init_al() {
  ALCdevice *dev = NULL;
  ALCcontext *ctx = NULL;

  const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
  std::cout << "Default device: " << defname << std::endl;

  dev = alcOpenDevice(defname);
  ctx = alcCreateContext(dev, NULL);
  alcMakeContextCurrent(ctx);
}

void exit_al() {
  ALCdevice *dev = NULL;
  ALCcontext *ctx = NULL;
  ctx = alcGetCurrentContext();
  dev = alcGetContextsDevice(ctx);

  alcMakeContextCurrent(NULL);
  alcDestroyContext(ctx);
  alcCloseDevice(dev);
}

void addMidiNoteToBuffer(int note, double time, double length, int *buf,
                         int vel) {

  int startSamp = (int)(time * sampleRate);
  float freq = 440.0 * powf(2.0, 1 / 12.0 * (float)(note - 69));
  // freq=freq*.125;
  float velEff = vel / 128.0;
  velEff = powf(velEff, .1);
  // velEff=velEff*velEff*velEff;
  float waveFormEff;
  for (int i = 0; i < length * sampleRate; i++) {
    if (i < 30) {
      waveFormEff = i / 30.0;
    } else {
      waveFormEff = (length * sampleRate - i) / length / sampleRate;
      // waveFormEff=waveFormEff*waveFormEff*waveFormEff*waveFormEff;
    }
    float vol = waveFormEff * velEff;
    float sample =
        16000 * vol * sin((2.f * float(M_PI) * freq) / sampleRate * i);
    if (sample > 0) {
      if (buf[startSamp + i] + sample < 32750) {
        buf[startSamp + i] += sample;
      } else {
        buf[startSamp + i] = 32750;
      }
    }
    if (sample < 0) {
      if (buf[startSamp + i] + sample > -32750) {
        buf[startSamp + i] += sample;
      } else {
        buf[startSamp + i] = -32750;
      }
    }
  }
}

void addSynthNoteToBuffer(double freq, double time, double length, int *buf,
                          double vol) {
  int startSamp = (int)(time * sampleRate);
  double noteSound;
  double envolope;
  for (int i = 0; i < length * sampleRate; i++) {
    if (i < 30) {
      envolope = i / 30.0;
    } else {
      envolope = (length * sampleRate - i) / length / sampleRate;
    }
    noteSound = sin(2 * M_PI * freq * i / sampleRate);
    float val = envolope * noteSound * vol * SHRT_MAX;
    // printf("%f\n", val);
    if (buf[startSamp + i] + val < 32750) {
      buf[startSamp + i] += val;
    } else {
      buf[startSamp + i] = 32750;
    }
  }
}

void addReverb(int length, float duration, float mix1, float mix2, int *buf) {
  int sampleDiff = duration * sampleRate;
  for (int i = sampleDiff; i < length; i++) {

    buf[i] = mix1 * buf[i] + mix2 * buf[i - sampleDiff];
  }
}

void convertIntBufToShort(int len, int *intBuf, short *shortBuf) {
  for (int i = 0; i < len; i++) {
    if (intBuf[i] < SHRT_MAX - 10) {
      shortBuf[i] = intBuf[i];
    } else {
      shortBuf[i] = SHRT_MAX - 10;
    }
  }
}

void perlinSynthSong(float length, float minFreq, float maxFreq, int intervals,
                     int notes, int *buf);
float freqForMidiNote(int note);
int getNoteForPentScale(int root, int note);

int main(int argc, char **argv) {

  int track = 0;
  double songLength = 50;

  /* initialize OpenAL */
  init_al();

  /* Create buffer to store samples */
  ALuint buf;
  alGenBuffers(1, &buf);
  al_check_error();

  int buf_size = songLength * sampleRate;

  int *samples;
  samples = new int[buf_size];
  short *samplesShort = new short[buf_size];
  float vol = .8;
  float freq = 440;
  for (int i = 0; i < buf_size; i++) {
    short sample =
        SHRT_MAX / 2 * vol * sin((2.f * float(M_PI) * freq) / sampleRate * i);
    samples[i] = sample;
  }

  addReverb(buf_size, .3, .6, .4, samples);

  convertIntBufToShort(buf_size, samples, samplesShort);
  float relSpeed = 1.0;
  /* Download buffer to OpenAL */
  alBufferData(buf, AL_FORMAT_MONO16, samplesShort, buf_size,
               relSpeed * sampleRate);
  al_check_error();

  /* Set-up sound source and play buffer */
  ALuint src = 0;
  alGenSources(1, &src);
  alSourcei(src, AL_BUFFER, buf);
  alSourcePlay(src);

  /* While sound is playing, sleep */
  al_check_error();
  usleep(songLength * 1000000);

  /* Dealloc OpenAL */
  exit_al();
  al_check_error();
}

/*
void perlinSynthSong(float length, float minFreq, float maxFreq, int intervals,
                     int notes, int *buf) {
  int width = 16;
  int height = 2048;
  int bitsPerSample = 16;
  int samplesPerPixels = 4;
  int pixelByteSize = 8;
  int pixelBitSize = 64 * 4;

  int initHorDiv = 1;
  int initVertDiv = 128;
  int octaves = 5;

  unsigned short *data =
      (unsigned short *)malloc(width * height * pixelBitSize);
  float *heightData = (float *)malloc(width * height * sizeof(float));
  float *octaveNoise = (float *)malloc(width * height * sizeof(float));
  genNoiseFloat(width, height, initHorDiv, initVertDiv, octaves, .5, heightData,
                octaveNoise, 2);
  for (int i = 0; i < width * height; i++) {
    float val = heightData[i] * 4 + .5;
    if (val > 1) {
      val = 1;
    }
    if (val < 0) {
      val = 0;
    }
    unsigned short adj = val * USHRT_MAX;
    data[4 * i] = adj;
    data[4 * i + 1] = adj;
    data[4 * i + 2] = adj;
    data[4 * i + 3] = USHRT_MAX;
  }

  srand(time(NULL));

  float bpm = 200;
  int buf_size = 100 * sampleRate;

  for (int y = 0; y < 300; y++) {
    for (int x = 0; x < width; x++) {
      float time = y * 60 / bpm;
      float length = 60 / bpm;
      // float frequency=32.70319*powf(2, x/12.0);
      float frequency = freqForMidiNote(getNoteForPentScale(24, x));
      // float frequency=freqForMidiNote(56);
      float height = heightData[width * y + x];
      if (height + .5 > .75) {
        float volume = 2.0 / width * 2 * heightData[width * y + x];
        addSynthNoteToBuffer(frequency, time, length, buf, volume);
      }
    }
    printf("Row down %d\n", y);
  }

  printf("Running\n");
}
*/

float freqForMidiNote(int note) { return 8.175799 * powf(2, note / 12.0); }

int getNoteForPentScale(int root, int note) {
  int scale[5] = {0, 2, 4, 7, 9};
  note = root + note / 5 * 12 + scale[note % 5];
  return note;
}
