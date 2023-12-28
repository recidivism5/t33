#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#define LERP(a,b,t) ((a) + (t)*((b)-(a)))

typedef float vec3[3];
typedef float vec4[4];
typedef int ivec3[3];
typedef float mat4[16];

inline float vec3_dot(vec3 a, vec3 b){
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
inline float vec3_length(vec3 a){
	return sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}
inline float vec3_angle_between(vec3 a, vec3 b){
	return acosf((a[0]*b[0] + a[1]*b[1] + a[2]*b[2])/(sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2])*sqrtf(b[0]*b[0] + b[1]*b[1] + b[2]*b[2])));
}
inline void vec3_cross(vec3 a, vec3 b, vec3 out){
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}
inline void vec3_add(vec3 a, vec3 b){
	for (int i = 0; i < 3; i++) a[i] += b[i];
}
inline void vec3_subtract(vec3 a, vec3 b){
	for (int i = 0; i < 3; i++) a[i] -= b[i];
}
inline void vec3_negate(vec3 a){
	for (int i = 0; i < 3; i++) a[i] = -a[i];
}
inline void vec3_scale(vec3 a, float scale){
	for (int i = 0; i < 3; i++) a[i] = a[i]*scale;
}
inline void vec3_normalize(vec3 a){
	float d = 1.0f / sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
	for (int i = 0; i < 3; i++) a[i] *= d;
}
inline void vec3_set_length(vec3 a, float length){
	float d = length / sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
	for (int i = 0; i < 3; i++) a[i] *= d;
}
inline void vec3_midpoint(vec3 a, vec3 b, vec3 out){
	for (int i = 0; i < 3; i++) out[i] = a[i] + 0.5f*(b[i]-a[i]);
}
inline void vec3_lerp(vec3 a, vec3 b, float t){
	for (int i = 0; i < 3; i++) a[i] = LERP(a[i],b[i],t);
}
inline void vec3_clamp_euler(vec3 a){
	for (int i = 0; i < 3; i++){
		if (a[i] > 4.0f*M_PI) a[i] -= 4.0f*M_PI;
		else if (a[i] < -4.0f*M_PI) a[i] += 4.0f*M_PI;
	}
}
inline void quat_slerp(vec4 a, vec4 b, float t){//from https://github.com/microsoft/referencesource/blob/51cf7850defa8a17d815b4700b67116e3fa283c2/System.Numerics/System/Numerics/Quaternion.cs#L289C8-L289C8
	float cosOmega = a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
	int flip = 0;
	if (cosOmega < 0.0f){
		flip = 1;
		cosOmega = -cosOmega;
	}
	float s1,s2;
	if (cosOmega > (1.0f - 1e-6f)){
		s1 = 1.0f - t;
		s2 = flip ? -t : t;
	} else {
		float omega = acosf(cosOmega);
		float invSinOmega = 1.0f / sinf(omega);
		s1 = sinf((1.0f-t)*omega)*invSinOmega;
		float sm = sinf(t*omega)*invSinOmega;
		s2 = flip ? -sm : sm;
	}
	for (int i = 0; i < 4; i++) a[i] = s1*a[i] + s2*b[i];
}
inline void mat4_identity(mat4 a){
	a[0] = 1.0f;
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = 1.0f;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = 1.0f;
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_from_basis_vectors(mat4 a, vec3 x, vec3 y, vec3 z){
	a[0] = x[0];
	a[1] = x[1];
	a[2] = x[2];
	a[3] = 0.0f;

	a[4] = y[0];
	a[5] = y[1];
	a[6] = y[2];
	a[7] = 0.0f;

	a[8] = z[0];
	a[9] = z[1];
	a[10] = z[2];
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_transpose(mat4 a){
	/*
	0 4 8  12
	1 5 9  13
	2 6 10 14
	3 7 11 15
	*/
	float t;
	SWAP(t,a[1],a[4]);
	SWAP(t,a[2],a[8]);
	SWAP(t,a[3],a[12]);
	SWAP(t,a[6],a[9]);
	SWAP(t,a[7],a[13]);
	SWAP(t,a[11],a[14]);
}
inline void mat4_transpose_mat3(mat4 a){
	/*
	0 4 8  12
	1 5 9  13
	2 6 10 14
	3 7 11 15
	*/
	float t;
	SWAP(t,a[1],a[4]);
	SWAP(t,a[2],a[8]);
	SWAP(t,a[6],a[9]);
}
inline void mat4_orthogonal(mat4 a, float left, float right, float bottom, float top, float near, float far){
	a[0] = 2.0f/(right-left);
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = 2.0f/(top-bottom);
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = 2.0f/(near-far);
	a[11] = 0.0f;

	a[12] = (right+left)/(left-right);
	a[13] = (top+bottom)/(bottom-top);
	a[14] = (far+near)/(near-far);
	a[15] = 1.0f;
}
inline void mat4_perspective(mat4 a, float fovRadians, float aspectRatio, float near, float far){
	float s = 1.0f / tanf(fovRadians * 0.5f);
	float d = near - far;

	a[0] = s/aspectRatio;
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = s;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = (far+near)/d;
	a[11] = -1.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = (2.0f*far*near)/d;
	a[15] = 0.0f;
}
inline void mat4_scale(mat4 a, float x, float y, float z){
	a[0] = x;
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = y;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = z;
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_scale_v(mat4 a, vec3 scale){
	a[0] = scale[0];
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = scale[1];
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = scale[2];
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_translation(mat4 a, float x, float y, float z){
	a[0] = 1.0f;
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = 1.0f;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = 1.0f;
	a[11] = 0.0f;

	a[12] = x;
	a[13] = y;
	a[14] = z;
	a[15] = 1.0f;
}
inline void mat4_translation_v(mat4 a, vec3 translation){
	a[0] = 1.0f;
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = 1.0f;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = 1.0f;
	a[11] = 0.0f;

	a[12] = translation[0];
	a[13] = translation[1];
	a[14] = translation[2];
	a[15] = 1.0f;
}
inline void mat4_rotation_x(mat4 a, float angle){
	float c = cosf(angle);
	float s = sinf(angle);

	a[0] = 1.0f;
	a[1] = 0.0f;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = c;
	a[6] = s;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = -s;
	a[10] = c;
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_rotation_y(mat4 a, float angle){
	float c = cosf(angle);
	float s = sinf(angle);

	a[0] = c;
	a[1] = 0.0f;
	a[2] = -s;
	a[3] = 0.0f;

	a[4] = 0.0f;
	a[5] = 1.0f;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = s;
	a[9] = 0.0f;
	a[10] = c;
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_rotation_z(mat4 a, float angle){
	float c = cosf(angle);
	float s = sinf(angle);

	a[0] = c;
	a[1] = s;
	a[2] = 0.0f;
	a[3] = 0.0f;

	a[4] = -s;
	a[5] = c;
	a[6] = 0.0f;
	a[7] = 0.0f;

	a[8] = 0.0f;
	a[9] = 0.0f;
	a[10] = 1.0f;
	a[11] = 0.0f;

	a[12] = 0.0f;
	a[13] = 0.0f;
	a[14] = 0.0f;
	a[15] = 1.0f;
}
inline void mat4_multiply(mat4 a, mat4 b, mat4 out){
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			out[i*4+j] = a[j]*b[i*4] + a[j+4]*b[i*4+1] + a[j+8]*b[i*4+2] + a[j+12]*b[i*4+3];
}
inline void mat4_multiply_vec4(mat4 m, vec4 v, vec4 out){
	for (int i = 0; i < 4; i++) out[i] = v[0]*m[i] + v[1]*m[4+i] + v[2]*m[8+i] + v[3]*m[12+i];
}
inline void mat4_look_at_y(mat4 m, vec3 eye, vec3 target){//rotates eye about Y to look at target
	float kx = target[0] - eye[0];
	float kz = target[2] - eye[2];
	float d = 1.0f / sqrtf(kx*kx + kz*kz);
	kx *= d;
	kz *= d;

	m[0] = -kz;
	m[1] = 0.0f;
	m[2] = kx;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 1.0f;
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = -kx;
	m[9] = 0.0f;
	m[10] = -kz;
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}
inline void mat4_look_at_xy(mat4 m, vec3 eye, vec3 target){//rotates eye about X and Y to look at target
	float kx = target[0] - eye[0],
		ky = target[1] - eye[1],
		kz = target[2] - eye[2];
	float d = 1.0f / sqrtf(kx*kx + ky*ky + kz*kz);
	kx *= d;
	ky *= d;
	kz *= d;

	float ix = -kz,
		iz = kx;
	d = 1.0f / sqrtf(ix*ix + iz*iz);
	ix *= d;
	iz *= d;

	float jx = -iz*ky,
		jy = iz*kx - ix*kz,
		jz = ix*ky;

	m[0] = ix;
	m[1] = 0.0f;
	m[2] = iz;
	m[3] = 0.0f;

	m[4] = jx;
	m[5] = jy;
	m[6] = jz;
	m[7] = 0.0f;

	m[8] = -kx;
	m[9] = -ky;
	m[10] = -kz;
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}
inline void mat4_look_at_xy_view(mat4 m, vec3 eye, vec3 target){//produces a view matrix
	float kx = target[0] - eye[0],
		ky = target[1] - eye[1],
		kz = target[2] - eye[2];
	float d = 1.0f / sqrtf(kx*kx + ky*ky + kz*kz);
	kx *= d;
	ky *= d;
	kz *= d;

	float ix = -kz,
		iz = kx;
	d = 1.0f / sqrtf(ix*ix + iz*iz);
	ix *= d;
	iz *= d;

	float jx = -iz*ky,
		jy = iz*kx - ix*kz,
		jz = ix*ky;

	m[0] = ix;
	m[1] = jx;
	m[2] = -kx;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = jy;
	m[6] = -ky;
	m[7] = 0.0f;

	m[8] = iz;
	m[9] = jz;
	m[10] = -kz;
	m[11] = 0.0f;

	m[12] = -(ix*eye[0] + iz*eye[2]);
	m[13] = -(jx*eye[0] + jy*eye[1] + jz*eye[2]);
	m[14] = kx*eye[0] + ky*eye[1] + kz*eye[2];
	m[15] = 1.0f;
}