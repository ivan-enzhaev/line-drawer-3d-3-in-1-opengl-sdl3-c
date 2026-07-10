#include "math_helper.h"
#include <math.h>
#include <string.h> /* for memcpy */

/* Helper: normalize a vec3 in-place, returns length before normalization */
static float _vec3_normalize_inplace(vec3 v)
{
    float x = v[0], y = v[1], z = v[2];
    float len = sqrtf(x * x + y * y + z * z);
    if (len > 0.0f)
    {
        v[0] = x / len;
        v[1] = y / len;
        v[2] = z / len;
    }
    return len;
}

/* Helper: compute cross product a x b -> out */
static void _vec3_cross(const vec3 a, const vec3 b, vec3 out)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

/* Helper: dot product */
static float _vec3_dot(const vec3 a, const vec3 b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* Helper: normalize quaternion (x,y,z,w) in-place */
static void _quat_normalize_inplace(versor q)
{
    float x = q[0], y = q[1], z = q[2], w = q[3];
    float len = sqrtf(x * x + y * y + z * z + w * w);
    if (len > 0.0f)
    {
        q[0] = x / len;
        q[1] = y / len;
        q[2] = z / len;
        q[3] = w / len;
    }
}

void MathHelper_setAxisAngle(const vec3 axis, float rad, versor out)
{
    /* rad provided in radians. Python code halves angle and uses sin(half) */
    float half = rad * 0.5f;
    float s = sinf(half);

    out[0] = s * axis[0];
    out[1] = s * axis[1];
    out[2] = s * axis[2];
    out[3] = cosf(half);
    /* ensure normalized */
    _quat_normalize_inplace(out);
}

void MathHelper_rotationTo(const vec3 initialVector,
    const vec3 destinationVector, versor out)
{
    const vec3 xUnitVec3 = { 1.0f, 0.0f, 0.0f };
    const vec3 yUnitVec3 = { 0.0f, 1.0f, 0.0f };

    float dot = _vec3_dot(destinationVector, initialVector);

    if (dot < -0.999999f)
    {
        /* vectors nearly opposite */
        vec3 tmpvec3;
        _vec3_cross(initialVector, xUnitVec3, tmpvec3);
        if (_vec3_normalize_inplace(tmpvec3) < 0.000001f)
        {
            /* pick another axis (y) if cross with x is nearly zero */
            _vec3_cross(initialVector, yUnitVec3, tmpvec3);
            _vec3_normalize_inplace(tmpvec3);
        }
        /* set quaternion representing 180 degree rotation around tmpvec3 */
        MathHelper_setAxisAngle(tmpvec3, (float)M_PI, out);
        return;
    }
    else if (dot > 0.999999f)
    {
        /* vectors nearly identical: identity quaternion */
        out[0] = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
        out[3] = 1.0f;
        return;
    }
    else
    {
        /* general case: q = [cross, 1 + dot] then normalize */
        vec3 tmpvec3;
        _vec3_cross(initialVector, destinationVector, tmpvec3);
        out[0] = tmpvec3[0];
        out[1] = tmpvec3[1];
        out[2] = tmpvec3[2];
        out[3] = 1.0f + dot;
        _quat_normalize_inplace(out);
        return;
    }
}
