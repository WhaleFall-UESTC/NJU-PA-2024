/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

// void audio_callback(void *userdata, uint8_t *stream, int len) {
//   SDL_LockAudio();
//   stream = sbuf;
//   SDL_UnlockAudio();
// }

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (!is_write || offset != 16 || audio_base[reg_init] != 1) return;
  printf("start audio\n");
  printf("frep:\t%u\n", audio_base[reg_freq]);
  printf("channels:\t%u\n", audio_base[reg_channels]);
  printf("samples:\t%u\n", audio_base[reg_samples]);
  printf("count:\t%u\n", audio_base[reg_count]);
  printf("And read from sbuf: %#x\n\n", sbuf[0]);
  sbuf[0] = 0;
  audio_base[reg_count] = 0;
  // printf("Hardware starts playing\n");
  // audio_base[reg_count] = 0; 
  // // assert(!is_write);
  // // assert(offset == 0);
  
  // // initialize
  // if (SDL_Init(SDL_INIT_AUDIO) < 0) return;
  
  // // Context
  // SDL_AudioSpec want, have;
  // want.freq     = audio_base[reg_freq];
  // want.format   = AUDIO_S16;
  // want.channels = audio_base[reg_channels];
  // want.samples  = audio_base[reg_samples];
  // want.size     = audio_base[reg_count];
  // want.callback = audio_callback;
  // want.userdata = &want;
  
  // // Open audio device
  // SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  // if (device_id == 0) {
  //   SDL_Quit();
  //   return;
  // }

  // // Start audio
  // SDL_PauseAudio(0); 

  // // Close
  // audio_base[reg_count] = 0;
  // SDL_CloseAudio();
  // SDL_Quit();
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
  
  audio_base[reg_init]      = 0;
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  audio_base[reg_count]     = 0;
}
