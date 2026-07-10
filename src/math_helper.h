#ifndef MATH_HELPER_H
#define MATH_HELPER_H

#include <cglm/cglm.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* Sets a quaternion from axis (vec3) and angle in radians (rad). Out is written to `out`. */
    void MathHelper_setAxisAngle(const vec3 axis, float rad, versor out);

    /* Sets a quaternion representing the shortest rotation from initialVector to destinationVector.
       Both vectors are assumed to be unit length. Result in `out`. */
    void MathHelper_rotationTo(const vec3 initialVector, const vec3 destinationVector, versor out);

#ifdef __cplusplus
}
#endif

#endif /* MATH_HELPER_H */
