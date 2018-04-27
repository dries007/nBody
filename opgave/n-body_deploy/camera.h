#ifndef _CAMERA_H_
#define _CAMERA_H_


#ifndef DECLARE_GLOBALS
#define EXTERNAL extern
#else
#define EXTERNAL
#endif

#ifdef __cplusplus
extern "C" {
#endif

const float* getMVPMat();
void moveCamera(float dx, float dy, float dz);
void init_camera();

void rotateCamera(float pan, float tilt);

#ifdef __cplusplus
}
#endif

#endif // _CAMERA_H_
