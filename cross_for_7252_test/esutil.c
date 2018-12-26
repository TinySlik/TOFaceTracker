#include <string.h>
#include <math.h>

#include "esutil.h"

//#define M_PI 3.1415926535898 
size_t g_2DCanavaEnvironmentWidth = 640;
size_t g_2DCanavaEnvironmentHeight = 480;

size_t g_2DCanavaEnvironmentTargetWidth = 150;
size_t g_2DCanavaEnvironmentTargetHeight = 150;


void esTranslate(ESMatrix *result, float tx, float ty, float tz)
{
	result->m[3][0] += (result->m[0][0] * tx + result->m[1][0] * ty + result->m[2][0] * tz);
	result->m[3][1] += (result->m[0][1] * tx + result->m[1][1] * ty + result->m[2][1] * tz);
	result->m[3][2] += (result->m[0][2] * tx + result->m[1][2] * ty + result->m[2][2] * tz);
	result->m[3][3] += (result->m[0][3] * tx + result->m[1][3] * ty + result->m[2][3] * tz);
}

void esMatrix3Translate(ESMatrix3 *result, float tx, float ty)
{
	result->m[2][0] += (result->m[0][0] * tx + result->m[1][0] * ty );
	result->m[2][1] += (result->m[0][1] * tx + result->m[1][1] * ty );
	result->m[2][2] += (result->m[0][2] * tx + result->m[1][2] * ty );
}

void esScale(ESMatrix *result, float sx, float sy, float sz)
{
	result->m[0][0] *= sx;
	result->m[0][1] *= sx;
	result->m[0][2] *= sx;
	result->m[0][3] *= sx;

	result->m[1][0] *= sy;
	result->m[1][1] *= sy;
	result->m[1][2] *= sy;
	result->m[1][3] *= sy;

	result->m[2][0] *= sz;
	result->m[2][1] *= sz;
	result->m[2][2] *= sz;
	result->m[2][3] *= sz;
}

void esMatrix3Scale(ESMatrix3 *result, float sx, float sy, float anchorX,float anchorY)
{
	float dX = anchorX * g_2DCanavaEnvironmentTargetWidth  / g_2DCanavaEnvironmentWidth;
	float dY = anchorY * g_2DCanavaEnvironmentTargetHeight / g_2DCanavaEnvironmentHeight;
	esMatrix3Translate(result,dX,dY);

	esMatrix3ScaleOrigin(result,sx,sy);

	esMatrix3Translate(result,0-dX,0-dY);
	//esMatrix3Translate(result,anchorX / sx,anchorY / sy);
}

void esMatrix3ScaleOrigin(ESMatrix3 *result, float sx, float sy)
{
	result->m[0][0] /= sx;
	result->m[0][1] /= sx;
	result->m[0][2] /= sx;

	result->m[1][0] /= sy;
	result->m[1][1] /= sy;
	result->m[1][2] /= sy;
}

void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB)
{
	ESMatrix tmp;
	int i;

	for (i=0; i<4; i++)
	{
		tmp.m[i][0] =	(srcA->m[i][0] * srcB->m[0][0]) +
						(srcA->m[i][1] * srcB->m[1][0]) +
						(srcA->m[i][2] * srcB->m[2][0]) +
						(srcA->m[i][3] * srcB->m[3][0]) ;

		tmp.m[i][1] =	(srcA->m[i][0] * srcB->m[0][1]) +
						(srcA->m[i][1] * srcB->m[1][1]) +
						(srcA->m[i][2] * srcB->m[2][1]) +
						(srcA->m[i][3] * srcB->m[3][1]) ;

		tmp.m[i][2] =	(srcA->m[i][0] * srcB->m[0][2]) +
						(srcA->m[i][1] * srcB->m[1][2]) +
						(srcA->m[i][2] * srcB->m[2][2]) +
						(srcA->m[i][3] * srcB->m[3][2]) ;

		tmp.m[i][3] =	(srcA->m[i][0] * srcB->m[0][3]) +
						(srcA->m[i][1] * srcB->m[1][3]) +
						(srcA->m[i][2] * srcB->m[2][3]) +
						(srcA->m[i][3] * srcB->m[3][3]) ;
	}
	memcpy(result, &tmp, sizeof(ESMatrix));
}

void esMatrix3Multiply(ESMatrix3 *result, ESMatrix3 *srcA, ESMatrix3 *srcB)
{
	ESMatrix3 tmp;
	int i;

	for (i=0; i<3; i++)
	{
		tmp.m[i][0] =	(srcA->m[i][0] * srcB->m[0][0]) +
						(srcA->m[i][1] * srcB->m[1][0]) +
						(srcA->m[i][2] * srcB->m[2][0]) ;

		tmp.m[i][1] =	(srcA->m[i][0] * srcB->m[0][1]) +
						(srcA->m[i][1] * srcB->m[1][1]) +
						(srcA->m[i][2] * srcB->m[2][1]) ;

		tmp.m[i][2] =	(srcA->m[i][0] * srcB->m[0][2]) +
						(srcA->m[i][1] * srcB->m[1][2]) +
						(srcA->m[i][2] * srcB->m[2][2]) ;
	}
	memcpy(result, &tmp, sizeof(ESMatrix3));
}

void esRotate(ESMatrix *result, float angle, float x, float y, float z)
{
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf(angle * (float)M_PI / 180.0f);
	cosAngle = cosf(angle * (float)M_PI / 180.0f);
	if (mag > 0.0f)
	{
		float xx, yy, zz, xy, yz, zx, xs, ys, zs;
		float oneMinusCos;
		ESMatrix rotMat;

		x /= mag;
		y /= mag;
		z /= mag;

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * sinAngle;
		ys = y * sinAngle;
		zs = z * sinAngle;
		oneMinusCos = 1.0f - cosAngle;

		rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
		rotMat.m[1][0] = (oneMinusCos * xy) - zs;
		rotMat.m[2][0] = (oneMinusCos * zx) + ys;
		rotMat.m[3][0] = 0.0F;

		rotMat.m[0][1] = (oneMinusCos * xy) + zs;
		rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
		rotMat.m[2][1] = (oneMinusCos * yz) - xs;
		rotMat.m[3][1] = 0.0F;

		rotMat.m[0][2] = (oneMinusCos * zx) - ys;
		rotMat.m[1][2] = (oneMinusCos * yz) + xs;
		rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
		rotMat.m[3][2] = 0.0F;

		rotMat.m[0][3] = 0.0F;
		rotMat.m[1][3] = 0.0F;
		rotMat.m[2][3] = 0.0F;
		rotMat.m[3][3] = 1.0F;

		esMatrixMultiply(result, &rotMat, result);
	}
}

void esMatrix3Rotate(ESMatrix3 *result, float angle, float anchorX,float anchorY)
{
	float dX = anchorX * g_2DCanavaEnvironmentTargetWidth  / g_2DCanavaEnvironmentWidth;
	float dY = anchorY * g_2DCanavaEnvironmentTargetHeight / g_2DCanavaEnvironmentHeight;
	esMatrix3Translate(result,dX,dY);

	esMatrix3RotateOrigin(result,angle);

	esMatrix3Translate(result,0-dX,0-dY);
}

void esMatrix3RotateOrigin(ESMatrix3 *result, float angle)
{
	float sinAngle, cosAngle;
	sinAngle = sinf(angle * (float)M_PI / 180.0f);
	cosAngle = cosf(angle * (float)M_PI / 180.0f);

	float oneMinusCos;
	ESMatrix3 rotMat;

	oneMinusCos = 1.0f - cosAngle;

	rotMat.m[0][0] = cosAngle;
	rotMat.m[1][0] = 0.0F - sinAngle;
	rotMat.m[2][0] = 0.0F;

	rotMat.m[0][1] = sinAngle;
	rotMat.m[1][1] = cosAngle;
	rotMat.m[2][1] = 0.0F;

	rotMat.m[0][2] = 0.0F;
	rotMat.m[1][2] = 0.0F;
	rotMat.m[2][2] = 1.0F;

	esMatrix3Multiply(result, &rotMat, result);
}


void esMatrixLoadIdentity(ESMatrix *result)
{
	memset(result, 0x0, sizeof(ESMatrix));
	result->m[0][0] = 1.0f;
	result->m[1][1] = 1.0f;
	result->m[2][2] = 1.0f;
	result->m[3][3] = 1.0f;
}

void esMatrix3LoadIdentity(ESMatrix3 *result)
{
	memset(result, 0x0, sizeof(ESMatrix3));
	result->m[0][0] = 1.0f;
	result->m[1][1] = 1.0f;
	result->m[2][2] = 1.0f;
}

void esMatrix3LoadIdentityWith2DInit(ESMatrix3 *result)
{
	memset(result, 0x0, sizeof(ESMatrix3));

	result->m[0][0] = 1.0f;
	result->m[1][1] = 1.0f;
	result->m[2][2] = 1.0f;

	esMatrix3Translate(result,0.5,0.5);

	float xS = (float)g_2DCanavaEnvironmentWidth / g_2DCanavaEnvironmentTargetWidth;
	float yS = (float)g_2DCanavaEnvironmentHeight / g_2DCanavaEnvironmentTargetHeight;

	result->m[0][0] *= xS;
	result->m[0][1] *= xS;
	result->m[0][2] *= xS;

	result->m[1][0] *= yS;
	result->m[1][1] *= yS;
	result->m[1][2] *= yS;
}

void esFrustum(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
	float deltaX = right - left;
	float deltaY = top - bottom;
	float deltaZ = farZ - nearZ;
	ESMatrix frust;

	if ((nearZ <= 0.0f) || (farZ <= 0.0f) ||
		(deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f))
		return;

	frust.m[0][0] = 2.0f * nearZ / deltaX;
	frust.m[0][1] = frust.m[0][2] = frust.m[0][3] = 0.0f;

	frust.m[1][1] = 2.0f * nearZ / deltaY;
	frust.m[1][0] = frust.m[1][2] = frust.m[1][3] = 0.0f;

	frust.m[2][0] = (right + left) / deltaX;
	frust.m[2][1] = (top + bottom) / deltaY;
	frust.m[2][2] = -(nearZ + farZ) / deltaZ;
	frust.m[2][3] = -1.0f;

	frust.m[3][2] = -2.0f * nearZ * farZ / deltaZ;
	frust.m[3][0] = frust.m[3][1] = frust.m[3][3] = 0.0f;

	esMatrixMultiply(result, &frust, result);
}

void esPerspective(ESMatrix *result, float fovy, float aspect, float zNear, float zFar)
{
	ESMatrix m;
	float sine, cotangent, deltaZ;
	float radians = fovy / 2.0f * (float)M_PI / 180.0f;

	deltaZ = zFar - zNear;
	sine = sinf(radians);
	if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
	{
		return;
	}
	cotangent = cosf(radians) / sine;

	m.m[0][0] = cotangent / aspect; m.m[0][1] =                          0; m.m[0][2] =                          0; m.m[0][3] =  0;
	m.m[1][0] =                  0; m.m[1][1] =                  cotangent; m.m[1][2] =                          0; m.m[1][3] =  0;
	m.m[2][0] =                  0; m.m[2][1] =                          0; m.m[2][2] =   -(zFar + zNear) / deltaZ; m.m[2][3] = -1;
	m.m[3][0] =                  0; m.m[3][1] =                          0; m.m[3][2] = -2 * zNear * zFar / deltaZ; m.m[3][3] =  0;

	esMatrixMultiply(result, &m, result);
}

void esOrtho(ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ)
{
	float deltaX = right - left;
	float deltaY = top - bottom;
	float deltaZ = farZ - nearZ;
	ESMatrix ortho;

	if ((deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f))
		return;

	esMatrixLoadIdentity(&ortho);
	ortho.m[0][0] = 2.0f / deltaX;
	ortho.m[3][0] = -(right + left) / deltaX;
	ortho.m[1][1] = 2.0f / deltaY;
	ortho.m[3][1] = -(top + bottom) / deltaY;
	ortho.m[2][2] = -2.0f / deltaZ;
	ortho.m[3][2] = -(nearZ + farZ) / deltaZ;

	esMatrixMultiply(result, &ortho, result);
}

int esInverse(ESMatrix * in, ESMatrix * out)
{
	float det_1;
	float abssum, temp;

#  define PRECISION_LIMIT (1.0e-15)
	
	/*
	Calculate the determinant of submatrix A and determine if the
	the matrix is singular.
	*/
	temp =  in->m[0][0] * in->m[1][1] * in->m[2][2];
	det_1 = temp; abssum = fabsf(temp);
	temp =  in->m[1][0] * in->m[2][1] * in->m[0][2];
	det_1 += temp; abssum += fabsf(temp);
	temp =  in->m[2][0] * in->m[0][1] * in->m[1][2];
	det_1 += temp; abssum += fabsf(temp);
	temp = -in->m[2][0] * in->m[1][1] * in->m[0][2];
	det_1 += temp; abssum += fabsf(temp);
	temp = -in->m[1][0] * in->m[0][1] * in->m[2][2];
	det_1 += temp; abssum += fabsf(temp);
	temp = -in->m[0][0] * in->m[2][1] * in->m[1][2];
	det_1 += temp; abssum += fabsf(temp);

	/* Is the submatrix A singular? */
	if ((det_1 == 0.0f) || (fabsf(det_1 / abssum) < PRECISION_LIMIT))
	{
		/* Matrix M has no inverse */
		return 0;
	}
	else
	{
		/* Calculate inverse(A) = adj(A) / det(A) */
		det_1 = 1.0f / det_1;
		out->m[0][0] =   (in->m[1][1] * in->m[2][2] - in->m[2][1] * in->m[1][2]) * det_1;
		out->m[1][0] = - (in->m[0][1] * in->m[2][2] - in->m[2][1] * in->m[0][2]) * det_1;
		out->m[2][0] =   (in->m[0][1] * in->m[1][2] - in->m[1][1] * in->m[0][2]) * det_1;
		out->m[0][1] = - (in->m[1][0] * in->m[2][2] - in->m[2][0] * in->m[1][2]) * det_1;
		out->m[1][1] =   (in->m[0][0] * in->m[2][2] - in->m[2][0] * in->m[0][2]) * det_1;
		out->m[2][1] = - (in->m[0][0] * in->m[1][2] - in->m[1][0] * in->m[0][2]) * det_1;
		out->m[0][2] =   (in->m[1][0] * in->m[2][1] - in->m[2][0] * in->m[1][1]) * det_1;
		out->m[1][2] = - (in->m[0][0] * in->m[2][1] - in->m[2][0] * in->m[0][1]) * det_1;
		out->m[2][2] =   (in->m[0][0] * in->m[1][1] - in->m[1][0] * in->m[0][1]) * det_1;

		/* Calculate -C * inverse(A) */
		out->m[3][0] = - (in->m[0][3] * out->m[0][0] +
						  in->m[1][3] * out->m[1][0] +
						  in->m[2][3] * out->m[2][0]);
		out->m[3][1] = - (in->m[0][3] * out->m[0][1] +
						  in->m[1][3] * out->m[1][1] +
						  in->m[2][3] * out->m[2][1]);
		out->m[3][2] = - (in->m[0][3] * out->m[0][2] +
						  in->m[1][3] * out->m[1][2] +
						  in->m[2][3] * out->m[2][2]);

		/* Fill in last column */
		out->m[0][3] = out->m[1][3] = out->m[2][3] = 0.0;
		out->m[3][3] = 1.0;

		return 1;
	}
}

void set2DCanavaEnvironment(size_t width , size_t height)
{
	g_2DCanavaEnvironmentWidth = width;
	g_2DCanavaEnvironmentHeight = height;
}

void set2DCanavaEnvironmentTarget(size_t width , size_t height)
{
	g_2DCanavaEnvironmentTargetWidth = width;
	g_2DCanavaEnvironmentTargetHeight = height;
}
