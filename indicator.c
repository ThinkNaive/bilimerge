#include "bilimerge.h"

void ProgressIndicator()
{
    if (g_current_duration - g_last_duration < 32768) return;
    float progress = g_progress_base + (float)g_current_duration/(float)g_video_duration/(float)g_number_of_videos;
    printf("\b\rProcessing : %5.1f%%", progress * 100.0);
    fflush(stdout);
    g_last_duration = g_current_duration;
}
