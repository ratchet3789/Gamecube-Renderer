//
// Created by addic on 2026/10/18.
//

#include "Camera.h"
#include <ogc/gx.h>

void Camera::InitializeCamera(f32 fov, f32 aspect, f32 n, f32 f)
{
	FOV = fov;
	Aspect = aspect;
	NearClip = n;
	FarClip = f;
	guPerspective(Projection, FOV, Aspect, NearClip, FarClip);
	GX_LoadProjectionMtx(Projection, GX_PERSPECTIVE);
}

void Camera::InitializeCamera()
{
	guPerspective(Projection, FOV, Aspect, NearClip, FarClip);
	GX_LoadProjectionMtx(Projection, GX_PERSPECTIVE);
}