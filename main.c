#include <stdio.h>
#include "miniaudio.h"
#include "miniaudio.c"

// setting the variable things so that it's identicle for both gen and device
#define FORMAT ma_format_f32 
#define CHANNELS 2
#define SAMPLE_RATE 48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{

    ma_noise* pNoise = (ma_noise*)pDevice->pUserData;

    ma_noise_read_pcm_frames(pNoise, pOutput, frameCount, NULL);


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
	ma_noise_type_white,
	0,
	0.2);

    // represents our gen config
    ma_noise noise;

    // initializes the gen using config and checks if it worked
    if (ma_noise_init(&noiseConfig, NULL, &noise) != MA_SUCCESS) {
	printf("Failed to initialize the noise generator :(\n");
	return 1;
    }

    // create audio device config

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = FORMAT;   // Set to ma_format_unknown to use the device's native format.
    config.playback.channels = CHANNELS;               // Set to 0 to use the device's native channel count.
    config.sampleRate        = SAMPLE_RATE;           // Set to 0 to use the device's native sample rate.
    config.dataCallback      = data_callback;   // This function will be called when miniaudio needs more data.
    config.pUserData         = &noise;   // Can be accessed from the device object (device.pUserData).

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

    printf("White noise is playing (hopefully). Enter to stop\n");
    getchar();

    // cleanup and undo started things
    ma_device_uninit(&device);
    ma_noise_uninit(&noise, NULL);
    return 0;
}

