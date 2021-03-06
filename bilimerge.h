#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

int flvmerge(int argc, char *argv[]);
int kmp(uint8_t s[], uint8_t t[], int len_s, int len_t);

// Progressing Indicator
int g_number_of_videos;
int g_video_duration;
int g_current_duration;
int g_last_duration;
float g_progress_base;
void ProgressIndicator();
