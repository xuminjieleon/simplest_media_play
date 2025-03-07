/**
 * 最简单的SDL2播放音频的例子（SDL2播放PCM）
 * Simplest Audio Play SDL2 (SDL2 play PCM) 
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序使用SDL2播放PCM音频采样数据。SDL实际上是对底层绘图
 * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
 * API。
 *
 * 函数调用步骤如下: 
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_OpenAudio(): 根据参数（存储于SDL_AudioSpec）打开音频设备。
 * SDL_PauseAudio(): 播放音频数据。
 *
 * [循环播放数据]
 * SDL_Delay(): 延时等待播放完成。
 *
 * This software plays PCM raw audio data using SDL2.
 * SDL is a wrapper of low-level API (DirectSound).
 * Use SDL is much easier than directly call these low-level API.
 *
 * The process is shown as follows:
 *
 * [Init]
 * SDL_Init(): Init SDL.
 * SDL_OpenAudio(): Opens the audio device with the desired 
 *					parameters (In SDL_AudioSpec).
 * SDL_PauseAudio(): Play Audio.
 *
 * [Loop to play data]
 * SDL_Delay(): Wait for completetion of playback.
 */

#include <stdio.h>
#include <tchar.h>

#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk; 
static  Uint32  audio_len; 
static  Uint8  *audio_pos; 

static SDL_AudioDeviceID audioDeviceId;
static SDL_AudioStream* stream;

static void quit(int rc)
{
    SDL_Quit();
    /* Let 'main()' return normally */
    if (rc != 0) {
        exit(rc);
    }
}

static void close_audio(void)
{
    if (audioDeviceId != 0) {
        SDL_DestroyAudioStream(stream);
        stream = NULL;
        SDL_CloseAudioDevice(audioDeviceId);
		audioDeviceId = 0;
    }
}

static void open_audio(void)
{
    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = 44100;
    wanted_spec.format = SDL_AUDIO_S16SYS;
    wanted_spec.channels = 2;

    audioDeviceId = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, &wanted_spec);
    if (!audioDeviceId) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open audio: %s\n", SDL_GetError());
        quit(2);
    }

    stream = SDL_CreateAndBindAudioStream(audioDeviceId, &wanted_spec);
    if (!stream)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create audio stream: %s\n", SDL_GetError());
        SDL_CloseAudioDevice(audioDeviceId);
        quit(2);
    }
}

int main(int argc, char* argv[])
{
	//Init
	if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	open_audio();

	FILE *fp=fopen("../NocturneNo2inEflat_44.1k_s16le.pcm","rb+");
    if (fp == NULL) {
        printf("cannot open this file\n");
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "cannot open this file\n");
		return -1;
	}

	int pcm_buffer_size=4096;
	char *pcm_buffer=(char *)SDL_malloc(pcm_buffer_size);
	int data_count=0;

	bool done = 0;
	while(!done){
		if (fread(pcm_buffer, 1, pcm_buffer_size, fp) != pcm_buffer_size){
			// Loop
			fseek(fp, 0, SEEK_SET);
			fread(pcm_buffer, 1, pcm_buffer_size, fp);
			data_count=0;
		}

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Now Playing %10d Bytes data.\n", data_count);
		data_count+=pcm_buffer_size;
		//Set audio buffer (PCM data)
		audio_chunk = (Uint8 *) pcm_buffer; 
		//Audio buffer length
		audio_len =pcm_buffer_size;
		audio_pos = audio_chunk;

		int ret = SDL_PutAudioStreamData(stream, audio_chunk, audio_len);
		if(ret == 0)
			audio_pos += audio_len;
		else
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't SDL_PutAudioStreamData: %s\n", SDL_GetError());

		SDL_Event event;
        while (SDL_PollEvent(&event) > 0) {
            if (event.type == SDL_EVENT_QUIT) {
                done = 1;
            }
        }
	}

	free(pcm_buffer);
	close_audio();
	SDL_Quit();

	return 0;
}

