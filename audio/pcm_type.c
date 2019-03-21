/*================================================================
*   Copyright (C) 2019 Guangzhou Firefly Ltd. All rights reserved.
*   
*   文件名称：pcm_type.c
*   创 建 者：luhuadong
*   创建日期：2019年03月21日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <alsa/asoundlib.h>

int main(void)
{
	int val;

	// 显示版本信息
	printf("ALSA library version: %s\n", SND_LIB_VERSION_STR);

	// 显示流类型名
	printf("\nPCM stream types:\n");
	for(val=0; val<=SND_PCM_STREAM_LAST; val++) {
		printf("  %s\n", snd_pcm_stream_name((snd_pcm_stream_t)val));
	}

	// 显示access类型
	printf("\nPCM access types:\n");
	for(val=0; val<=SND_PCM_ACCESS_LAST; val++) {
		printf("  %s\n", snd_pcm_access_name((snd_pcm_access_t)val));
	}

	// 显示PCM格式
	printf("\nPCM formats:\n");
	for(val=0; val<=SND_PCM_FORMAT_LAST; val++) {
		if(snd_pcm_format_name((snd_pcm_format_t)val) != NULL) {
			printf("  %s (%s)\n", snd_pcm_format_name((snd_pcm_format_t)val), 
					snd_pcm_format_description((snd_pcm_format_t)val));
		}
	}
	
	// 子格式
	printf("\nPCM subformats:\n");
	for(val=0; val<=SND_PCM_SUBFORMAT_LAST; val++) {
		printf("  %s (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t)val), 
				snd_pcm_subformat_description((snd_pcm_subformat_t)val));
	}

	// PCM当前状态
	printf("\nPCM states:\n");
	for(val=0; val<=SND_PCM_STATE_LAST; val++) {
		printf("  %s\n", snd_pcm_state_name((snd_pcm_state_t)val));
	}

	return 0;
}
