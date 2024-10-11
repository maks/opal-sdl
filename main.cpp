#include <SDL.h>
#include <stdio.h>

#include "utils.h"
#include <opal.h>

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 512;

int8_t *audio_buffer;

static Opal opl_(SAMPLE_RATE);

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

    // *2 to bump up output volume for now
    s_byte_stream[i++] = l * 2;
    s_byte_stream[i++] = r * 2;
  }
}

const int startMidiNote = 69;

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
  }

  // set note in OPAL
  u_char block = new_note / 12;
  u_int16_t fnum = noteFNumbers[new_note % 12];
  printf("PLAY block[%d] fnum:%d\n", block, fnum);

  u_char areg = fnum & 0xFF; // truncate to lowest 8 bytes
  //        note on  block in d2,3,4  high 2 bits of fnum in D0,D1
  u_char breg = 0x20 | (block << 2) | (fnum >> 8);

  // Note on with the fnumber that matches the give midi note
  opl_.Port(0xA0, areg);
  opl_.Port(0xB0, breg);

  // printBits(sizeof(fnum), &fnum);
  // printBits(sizeof(areg), &areg);
  // printBits(sizeof(breg), &breg);
}

static void handle_key_down(SDL_Keysym *keysym) { handle_note_keys(keysym); }

int main(int argc, char const *argv[]) {

  // Frequency
  opl_.Port(0xA0, 0xe7);
  // Note on, block, hi freq
  opl_.Port(0xB0, 0x31);
  // Tremolo/Vibrato/Sustain/KSR/Multiplication
  opl_.Port(0x20, 0x00);
  opl_.Port(0x23, 0x01);
  opl_.Port(0x28, 0x06);
  opl_.Port(0x2B, 0x01);
  // Waveform
  opl_.Port(0xE0, 0x01);
  opl_.Port(0xE3, 0x01);
  opl_.Port(0xE8, 0x02);
  opl_.Port(0xEB, 0x02);
  // Key Scale Level/Output Level
  opl_.Port(0x40, 0x40);
  opl_.Port(0x43, 0x1c);
  opl_.Port(0x48, 0x5c);
  opl_.Port(0x4B, 0x02);
  // Attack Rate/Decay Rate
  opl_.Port(0x60, 0x72);
  opl_.Port(0x63, 0x71);
  opl_.Port(0x68, 0x82);
  opl_.Port(0x6B, 0xf0);
  // Sustain Level/Release Rate
  opl_.Port(0x80, 0x86);
  opl_.Port(0x83, 0x94);
  opl_.Port(0x88, 0xbc);
  opl_.Port(0x8B, 0xbc);

  // enable left/right only for channel0
  opl_.Port(0xc0, 0x37);
  opl_.Port(0xc1, 0x00);
  opl_.Port(0xc2, 0x00);
  opl_.Port(0xc3, 0x00);
  opl_.Port(0xc4, 0x00);
  opl_.Port(0xc5, 0x00);
  opl_.Port(0xc6, 0x00);
  opl_.Port(0xc7, 0x00);
  opl_.Port(0xc8, 0x00);
  opl_.Port(0x1c0, 0x00);
  opl_.Port(0x1c1, 0x00);
  opl_.Port(0x1c2, 0x00);
  opl_.Port(0x1c3, 0x00);
  opl_.Port(0x1c4, 0x00);
  opl_.Port(0x1c5, 0x00);
  opl_.Port(0x1c6, 0x00);
  opl_.Port(0x1c7, 0x00);
  opl_.Port(0x1c8, 0x00);

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
