//
// Created by Coby Jennings on 2026/10/18.
//

#ifndef GCCUBEENGINE_CAMERA_H
#define GCCUBEENGINE_CAMERA_H
#include <ogc/gu.h>

#include "RenderObjects/StaticMesh.h"

class alignas(32) Camera
{
public:
	Camera() = default;
	Camera(const guVector& NewCameraVector, const guVector& NewUp, const guVector& NewLook, bool bInitialize = true)
	{
		CameraPosition = NewCameraVector;
		Up = NewUp;
		CameraLook = NewLook;
		if (bInitialize)
			InitializeCamera();
	}

	~Camera() = default;

	// Initialize Camera without modifying any value
	void InitializeCamera();
	// Initialize Camera with custom values
	void InitializeCamera(f32 fov, f32 aspect, f32 n, f32 f);

	const Mtx&			GetView()			const { return View; }
	const Mtx44&		GetProjection()		const { return Projection; }
	const guVector& 	GetCameraVector()	const { return CameraPosition; }
	const guVector& 	GetUpVector()		const { return Up; }
	const guVector& 	GetLookVector()		const { return CameraLook; }

	Mtx&		GetViewNonConst()			{ return View; }
	Mtx44&		GetProjectionNonConst()		{ return Projection; }
	guVector& 	GetCameraVectorNonConst()	{ return CameraPosition; }
	guVector& 	GetUpVectorNonConst()		{ return Up; }
	guVector& 	GetLookVectorNonConst()		{ return CameraLook; }

	void SetPosition(const Vec3f& NewPosition)
	{
		CameraPosition.x = NewPosition.x;
		CameraPosition.y = NewPosition.y;
		CameraPosition.z = NewPosition.z;

		CameraLook.x = NewPosition.x;
		CameraLook.y = NewPosition.y;
		CameraLook.z = NewPosition.z;

		guLookAt(View, &CameraPosition, &Up, &CameraLook);
		GX_LoadProjectionMtx(Projection, GX_PERSPECTIVE);
		GX_LoadPosMtxImm(View, GX_PNMTX0);
		//printf("[Gamecube] Moving Camera %f %f %f", CameraPosition.x, CameraPosition.y, CameraPosition.z);
	}

	void AddPosition(const Vec3f& AddedPosition)
	{
		CameraPosition.x += AddedPosition.x;
		CameraPosition.y += AddedPosition.y;
		CameraPosition.z += AddedPosition.z;

		CameraLook.x += AddedPosition.x;
		CameraLook.y += AddedPosition.y;
		CameraLook.z += AddedPosition.z;
		guMtxTransApply(View, View, 50.0f,0.0f, -50.0f);

		guLookAt(View, &CameraPosition, &Up, &CameraLook);
		GX_LoadProjectionMtx(Projection, GX_PERSPECTIVE);
		GX_LoadPosMtxImm(View, GX_PNMTX0);
	}

private:
	Mtx View;
	Mtx44 Projection;
	guVector CameraPosition, Up, CameraLook;
	float FOV{60};
	float Aspect{1.33f};
	float NearClip{0.1f};
	float FarClip{1000.0f};
};

#endif //GCCUBEENGINE_CAMERA_H