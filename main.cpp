#include <SDL.h>
#include <stdio.h>

#include "utils.h"
#include <opal.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512

#define FREQ_BASE_REG 0xA0
#define OCTAVE_BASE_REG 0xB0

int8_t *audio_buffer;

static Opal opl_(SAMPLE_RATE);

int algorithm_ = 0;

void oscillator_callback(void *userdata, uint8_t *byteStream,
                         int byteStreamLen) {
  // clear buffer
  memset(byteStream, 0, byteStreamLen);

  int16_t *s_byte_stream;
  /* cast buffer as 16bit signed int */
  s_byte_stream = (int16_t *)byteStream;

  size_t len = byteStreamLen / (sizeof(int16_t) * 2);
  // FILL buffer from OPAL
  int i = 0;
  while (len--) {
    int16_t l, r;
    opl_.Sample(&l, &r);

    s_byte_stream[i++] = l;
    s_byte_stream[i++] = r;
  }
}

const int startMidiNote = 60;

static const unsigned int noteFNumbers[] = {342, 363, 385, 408, 432, 458,
                                            485, 514, 544, 577, 611, 647};

static void handle_note_keys(SDL_Keysym *keysym) {
  /* change note or octave depending on which key is pressed */
  int new_note = startMidiNote;

  switch (keysym->sym) {
  case SDLK_a:
    new_note = startMidiNote;
    break;
  case SDLK_s:
    new_note = startMidiNote + 1;
    break;
  case SDLK_d:
    new_note = startMidiNote + 2;
    break;
  case SDLK_f:
    new_note = startMidiNote + 3;
    break;
  case SDLK_g:
    new_note = startMidiNote + 4;
    break;
  case SDLK_h:
    new_note = startMidiNote + 5;
    break;
  case SDLK_j:
    new_note = startMidiNote + 6;
    break;
  case SDLK_k:
    new_note = startMidiNote + 7;
    break;
  case SDLK_l:
    new_note = startMidiNote + 8;
    break;
  case SDLK_0:
    algorithm_ = 0;
    break;
  case SDLK_1:
    algorithm_ = 1;
  default:
    new_note = 0;
    break;
  }

  // channel wide settings
  // enable left/right output (D4, D5) & set algorithm D0
  // for now only 2 op so just Additive or FM
  opl_.Port(0xC0, 0x30 + algorithm_);

  // set note in OPAL
  u_char block = new_note / 12;
  u_int16_t fnum = noteFNumbers[new_note % 12];
  printf("ALGORITHM:%d\n", algorithm_);
  printf("PLAY %d block[%d] fnum:%d\n", new_note, block, fnum);

  u_char areg = fnum & 0xFF; // truncate to lowest 8 bytes
  //        note on  block in d2,3,4  high 2 bits of fnum in D0,D1
  u_char breg = 0x20 | (block << 2) | (fnum >> 8);

  // Note on with the fnumber that matches the give midi note
  opl_.Port(FREQ_BASE_REG, areg);
  opl_.Port(OCTAVE_BASE_REG, breg);

  // printBits(sizeof(fnum), &fnum);
  // printBits(sizeof(areg), &areg);
  // printBits(sizeof(breg), &breg);
}

static void handle_key_down(SDL_Keysym *keysym) { handle_note_keys(keysym); }

int main(int argc, char const *argv[]) {

  // opl_.Port(0x01, 0x20);

  uint8_t op1TremVibSusKSR_ = 0x0;
  uint8_t op2TremVibSusKSR_ = 0x0;

  // multiplier is only 4bits
  uint8_t freqMultOp1 = 0x3 & 0xF;
  uint8_t freqMultOp2 = 0x0 & 0xF;

  uint8_t tremVibSusKSR1 = op1TremVibSusKSR_;
  uint8_t tremVibSusKSR2 = op2TremVibSusKSR_;

  uint8_t tvskmOp1 = (tremVibSusKSR1 << 4) + freqMultOp1;
  uint8_t tvskmOp2 = (tremVibSusKSR2 << 4) + freqMultOp2;

  // Tremolo/Vibrato/Sustain/KSR/Multiplication
  opl_.Port(0x20, tvskmOp1);
  opl_.Port(0x21, tvskmOp2);
  // Waveform
  opl_.Port(0xE0, 0x04); // 0 = pure sine
  opl_.Port(0xE1, 0x00);
  // Key Scale Level/Output Level
  opl_.Port(0x40, 0x0F);
  opl_.Port(0x41, 0x00);
  // Attack Rate/Decay Rate
  opl_.Port(0x60, 0x50);
  opl_.Port(0x61, 0x50);
  // Sustain Level/Release Rate
  opl_.Port(0x80, 0x03);
  opl_.Port(0x81, 0x03);

  printf("OPAL SDL TEST \n");

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
    printf("Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  // SDL WINDOW setup
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_GLContext context;

  window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            640, 480, SDL_WINDOW_OPENGL);

  if (window != NULL) {
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
      printf("\nFailed to create context: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer != NULL) {
      SDL_GL_SetSwapInterval(1);
      SDL_SetWindowTitle(window, "SDL2 synth sample 2");
    } else {
      printf("Failed to create renderer: %s", SDL_GetError());
    }
  } else {
    printf("Failed to create window:%s", SDL_GetError());
  }

  // SDL window setup done ==========
  SDL_AudioSpec spec = {
      .freq = SAMPLE_RATE,
      .format = AUDIO_S16LSB,
      .channels = 2,
      .samples = BUFFER_SIZE,
      .callback = oscillator_callback,
  };

  if (SDL_OpenAudio(&spec, NULL) < 0) {
    printf("Failed to open Audio Device: %s\n", SDL_GetError());
    return 1;
  } else {
    printf("Opened Audio Device: %d\n", spec.format);
  }

  SDL_PauseAudio(0);

  while (true) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_KEYDOWN:
        // dont want key repeats
        if (e.key.repeat == 0) {
          handle_key_down(&e.key.keysym);
        }
        break;
      case SDL_KEYUP:
        printf("STOP NOTE\n");
        // note off in OPAL
        opl_.Port(0xB0, 0x0);
        break;
      case SDL_QUIT:
        printf("exiting...\n");
        return 0;
      }
    }
  }

  return 0;
}
