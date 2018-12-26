#ifndef _ESUTIL_H_
#define _ESUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	float m[4][4];
} ESMatrix;

typedef struct
{
	float m[3][3];
} ESMatrix3;

/* Matrix manipulation functions - useful for ES2, which doesn't have them built in */
void esTranslate(ESMatrix *result, float tx, float ty, float tz);
void esMatrix3Translate(ESMatrix3 *result, float tx, float ty);
void esScale(ESMatrix *result, float sx, float sy, float sz);
void esMatrix3Scale(ESMatrix3 *result, float sx, float sy, float anchorX ,float anchorY );
void esMatrix3ScaleOrigin(ESMatrix3 *result, float sx, float sy);
void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB);
void esMatrix3Multiply(ESMatrix3 *result, ESMatrix3 *srcA, ESMatrix3 *srcB);
int  esInverse(ESMatrix * in, ESMatrix * out);
void esRotate(ESMatrix *result, float angle, float x, float y, float z);
void esMatrix3Rotate(ESMatrix3 *result, float angle , float anchorX,float anchorY);
void esMatrix3RotateOrigin(ESMatrix3 *result, float angle);

void esMatrixLoadIdentity(ESMatrix *result);
void esMatrix3LoadIdentity(ESMatrix3 *result);
void esMatrix3LoadIdentityWith2DInit(ESMatrix3 *result);
void esFrustum(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ);
void esPerspective(ESMatrix *result, float fovy, float aspect, float zNear, float zFar);
void esOrtho(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ);

void set2DCanavaEnvironment(size_t width , size_t height);
void set2DCanavaEnvironmentTarget(size_t width , size_t height);
#ifdef __cplusplus
}
#endif

#endif /* _ESUTIL_H_ */
