//
// Created by Coby Jennings on 2026/10/18.
//

#ifndef GCCUBEENGINE_CAMERA_H
#define GCCUBEENGINE_CAMERA_H
#include <ogc/gu.h>

class Camera
{
public:
	Camera() = default;
	Camera(const guVector& NewCameraVector, const guVector& NewUp, const guVector& NewLook, bool bInitialize = true)
	{
		CameraVector = NewCameraVector;
		Up = NewUp;
		Look = NewLook;
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
	const guVector& 	GetCameraVector()	const { return CameraVector; }
	const guVector& 	GetUpVector()		const { return Up; }
	const guVector& 	GetLookVector()		const { return Look; }

	Mtx&		GetViewNonConst()			{ return View; }
	Mtx44&		GetProjectionNonConst()		{ return Projection; }
	guVector& 	GetCameraVectorNonConst()	{ return CameraVector; }
	guVector& 	GetUpVectorNonConst()		{ return Up; }
	guVector& 	GetLookVectorNonConst()		{ return Look; }

private:
	Mtx View;
	Mtx44 Projection;
	guVector CameraVector, Up, Look;
	float FOV{60};
	float Aspect{1.33f};
	float NearClip{10.0f};
	float FarClip{300.0f};
};

#endif //GCCUBEENGINE_CAMERA_H