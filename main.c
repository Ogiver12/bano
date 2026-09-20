#include <stdio.h>
#include "miniaudio.h"
#include "miniaudio.c"

// setting the variable things so that it's identicle for both gen and device
#define FORMAT ma_format_f32 
#define CHANNELS 2
#define SAMPLE_RATE 48000

// oh boy a struct
typedef struct {
    ma_noise noise;
    ma_lpf lpf;
} AudioData;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{

    AudioData* pAudio = (AudioData*)pDevice->pUserData;

    // temp buffer to hold generated pink noise
    float tempBuffer[frameCount * CHANNELS];

    ma_noise_read_pcm_frames(&pAudio->noise, tempBuffer, frameCount, NULL);

    ma_lpf_process_pcm_frames(&pAudio->lpf, pOutput, tempBuffer, frameCount);

    // In playback mode copy data to pOutput. In capture mode read data from pInput. In full-duplex mode, both
    // pOutput and pInput will be valid and you can move data from pInput into pOutput. Never process more than
    // frameCount frames.
}

int main()
{
    // create noise gen
    
    ma_noise_config noiseConfig = ma_noise_config_init(
	FORMAT,
	CHANNELS,
	ma_noise_type_pink,
	0,
	0.3);

    // represents our gen config
    ma_noise noise;

    // initializes the gen using config and checks if it worked
    if (ma_noise_init(&noiseConfig, NULL, &noise) != MA_SUCCESS) {
	printf("Failed to initialize the noise generator :(\n");
	return 1;
    }

    // create low pass as default pink is still too high

    ma_lpf_config lpfConfig = ma_lpf_config_init(
	FORMAT,
	CHANNELS,
	SAMPLE_RATE,
	300, //cutoff frequency - change this until ok 
	4);

    //initialize the low pass filter
    ma_lpf lpf;
    
    if (ma_lpf_init(&lpfConfig, NULL, &lpf) != MA_SUCCESS) {
	printf("Failed to initialize the low-pass filter :(\n");
	return 1;
    }

    // combined noise+filter for callback
    AudioData audio;

    // put noise gen into AudioData struct
    audio.noise = noise;
    // put low-pass filter into AudioData struct
    audio.lpf = lpf;

//    config.pUserData = &audio;

    // create audio device config

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = FORMAT;   // Set to ma_format_unknown to use the device's native format.
    config.playback.channels = CHANNELS;               // Set to 0 to use the device's native channel count.
    config.sampleRate        = SAMPLE_RATE;           // Set to 0 to use the device's native sample rate.
    config.dataCallback      = data_callback;   // This function will be called when miniaudio needs more data.
    config.pUserData         = &audio;   // Can be accessed from the device object (device.pUserData).

    // initializes the device
    ma_device device;
    
    // null means miniaudio chooses the default audio backend (im pretty sure thats like pulseaudio)
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
	printf("Failed to initialize audio device :(\n");
        return -1;  // Failed to initialize the device.
    }

     // The device is sleeping by default so you'll need to start it manually.

    if (ma_device_start(&device) != MA_SUCCESS){
	printf("Failed to start audio device :(\n");
	// other things were already initialized so undo that to clean it up
	ma_device_uninit(&device);
	ma_noise_uninit(&noise, NULL);
	return 1;
    }    

    printf("pink noise is playing (hopefully). Enter to stop\n");
    getchar();

    // cleanup and undo started things
    ma_device_uninit(&device);
    ma_lpf_uninit(&lpf, NULL);
    ma_noise_uninit(&noise, NULL);
    return 0;
}

