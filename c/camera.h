#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int camera_init();

void camera_shutdown();

unsigned char* camera_read(const int header_size, int* size);

#ifdef __cplusplus
} /* extern "C" */
#endif
