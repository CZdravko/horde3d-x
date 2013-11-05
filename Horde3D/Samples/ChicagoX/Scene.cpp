// *************************************************************************************************
//
// Horde3D
//   Next-Generation Graphics Engine
//
// Sample Application
// --------------------------------------
// Copyright (C) 2006-2011 Nicolas Schulz
//
//
// This sample source file is not covered by the EPL as the rest of the SDK
// and may be used without any restrictions. However, the EPL's disclaimer of
// warranty and liability shall be in effect for this file.
//
// *************************************************************************************************

#include "Scene.h"
#include "Horde3D.h"
#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include "cocos2d.h"
#include "PointLight.h"
#include <vector>

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  \
											 "NvSLESPlayer", \
											 __VA_ARGS__))

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG,  \
											 "NvSLESPlayer", \
											 __VA_ARGS__))

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR,  \
											 "NvSLESPlayer", \
											 __VA_ARGS__))

using namespace std;
using namespace cocos2d;
using namespace Horde3D;

// Convert from degrees to radians
//inline float degToRad(float f) {
//	return f * (3.1415926f / 180.0f);
//}

float Scene::quantizedZ[17] = { 0.1, 5.0, 6.8, 9.2, 12.6, 17.1, 23.2, 31.5,
		42.9, 58.3, 79.2, 108, 146, 199, 271, 368, 500 };

Scene::Scene() {
	for (unsigned int i = 0; i < 320; ++i) {
		_keys[i] = false;
		_prevKeys[i] = false;
	}

	_x = 15;
	_y = 3;
	_z = 20;
	_rx = -10;
	_ry = 60;
	_velocity = 10.0f;
	_curFPS = 30;

	_statMode = 0;
	_freezeMode = 0;
	_debugViewMode = false;
	_wireframeMode = false;
	_cam = 0;

	color_index = 0;
	tTexH = 256;
	tTexW = 256;

	m_fFOVy = 45;
	m_fZNear = 5;

	numZDivisions = 16;
	numYDivisions = 17;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	_contentDir = "Content";
#else
	_contentDir = "../Content";
#endif

	// Initialize engine
	if (!h3dInit(CCDirector::sharedDirector()->getOpenGLView()->getDevice())) {
		LOGE("COULDN't INITIALIZE HORDE");
		h3dutDumpMessages();
		exit(-1);
	}

	// Set options
	h3dSetOption(H3DOptions::LoadTextures, 1);
	h3dSetOption(H3DOptions::MaxAnisotropy, 4);
	h3dSetOption(H3DOptions::ShadowMapSize, 2048);
	h3dSetOption(H3DOptions::FastAnimation, 1);

	// Add resources
	// Pipelines
	_forwardPipeRes = h3dAddResource(H3DResTypes::Pipeline,
			"pipelines/forward.pipeline.xml", 0);
	_deferredPipeRes = h3dAddResource(H3DResTypes::Pipeline,
			"pipelines/deferred.pipeline.xml", 0);
	// Overlays
	_fontMatRes = h3dAddResource(H3DResTypes::Material,
			"overlays/font.material.xml", 0);
	_panelMatRes = h3dAddResource(H3DResTypes::Material,
			"overlays/panel.material.xml", 0);
	_logoMatRes = h3dAddResource(H3DResTypes::Material,
			"overlays/logo.material.xml", 0);
	// Shader for deferred shading
	H3DRes lightMatRes = h3dAddResource(H3DResTypes::Material,
			"materials/light.material.xml", 0);
	// Environment
	H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph,
			"models/platform/platform.scene.xml", 0);
	// Skybox
	H3DRes skyBoxRes = h3dAddResource(H3DResTypes::SceneGraph,
			"models/skybox/skybox.scene.xml", 0);

	//Adding 2 lights:
	PointLight p1 = PointLight(Vec3f(1, 1, 1), 6, 1);
	PointLight p2 = PointLight(Vec3f(-1, 2, -3), 2, 2);

	m_vPointLights.push_back(p1);
	m_vPointLights.push_back(p2);

	//light Color texture
	lightColorTex = h3dCreateTexture("lightColorTex", 256, 4,
			H3DFormats::TEX_RGBA8, H3DResFlags::NoTexMipmaps);
	uint8_t* pLightColor = static_cast<uint8_t*>(h3dMapResStream(lightColorTex,
			H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	//L1

	pLightColor = pack_float(0.9f, pLightColor);
	pLightColor = pack_float(0.7f, pLightColor);
	pLightColor = pack_float(0.7f, pLightColor);
	pLightColor = pack_float(0.7f, pLightColor);

	h3dUnmapResStream(lightColorTex);
//	//L2
//	pLightColor = pack_float(0.9f, pLightColor);
//	pLightColor = pack_float(0.7f, pLightColor);
//	pLightColor = pack_float(0.7f, pLightColor);
//	pLightColor = pack_float(0.7f, pLightColor);

	//light Rr texture
	lightRrTex = h3dCreateTexture("lightRrTex", 256, 4, H3DFormats::TEX_RGBA8,
			H3DResFlags::NoTexMipmaps);
	uint8_t* pLightRr = static_cast<uint8_t*>(h3dMapResStream(lightRrTex,
			H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	//L1
	pLightRr = pack_float(p1.position.x, pLightRr);
	pLightRr = pack_float(p1.position.y, pLightRr);
	pLightRr = pack_float(p1.position.z, pLightRr);
	pLightRr = pack_float(p1.r, pLightRr);
	//L2
	pLightRr = pack_float(p2.position.x, pLightRr);
	pLightRr = pack_float(p2.position.y, pLightRr);
	pLightRr = pack_float(p2.position.z, pLightRr);
	pLightRr = pack_float(p2.r, pLightRr);

	h3dUnmapResStream(lightRrTex);

	//Tile texture

	tileTex = h3dCreateTexture("tileTex", tTexW, tTexH, H3DFormats::TEX_RGBA8,
			H3DResFlags::NoTexMipmaps);
	uint8_t* pTileTex = static_cast<uint8_t*>(h3dMapResStream(tileTex,
			H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	h3dUnmapResStream(tileTex);

	// Load resources
#ifdef HORDE3D_D3D11
	const char* platformDirectory = "d3d11";
#elif HORDE3D_GL
	const char* platformDirectory = "gl";
#elif defined(HORDE3D_GLES2)
	const char* platformDirectory = "gles2";
#endif
	CCDirector::sharedDirector()->loadResourcesFromDisk(_contentDir.c_str(),
			platformDirectory);

	// Add scene nodes
	// Add camera
	_cam = h3dAddCameraNode(H3DRootNode, "Camera", _forwardPipeRes);
	//h3dSetNodeParamI( _cam, H3DCamera::OccCullingI, 1 );
	// Add environment
	H3DNode env = h3dAddNodes(H3DRootNode, envRes);
	h3dSetNodeTransform(env, 0, 0, 0, 0, 0, 0, 0.23f, 0.23f, 0.23f);
	// Add skybox
	H3DNode sky = h3dAddNodes(H3DRootNode, skyBoxRes);
	h3dSetNodeTransform(sky, 0, 0, 0, 0, 0, 0, 210, 50, 210);
	h3dSetNodeFlags(sky, H3DNodeFlags::NoCastShadow, true);
	// Add light source
	H3DNode light = h3dAddLightNode(H3DRootNode, "Light1", lightMatRes,
			"LIGHTING", "SHADOWMAP");
	h3dSetNodeTransform(light, 0, 20, 50, -30, 0, 0, 1, 1, 1);
	h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 200);
	h3dSetNodeParamF(light, H3DLight::FovF, 0, 90);
	h3dSetNodeParamI(light, H3DLight::ShadowMapCountI, 3);
	h3dSetNodeParamF(light, H3DLight::ShadowSplitLambdaF, 0, 0.9f);
	h3dSetNodeParamF(light, H3DLight::ShadowMapBiasF, 0, 0.001f);
	h3dSetNodeParamF(light, H3DLight::ColorF3, 0, 0.9f);
	h3dSetNodeParamF(light, H3DLight::ColorF3, 1, 0.7f);
	h3dSetNodeParamF(light, H3DLight::ColorF3, 2, 0.75f);

	_crowdSim = new CrowdSim(_contentDir);
	_crowdSim->init();

	resize(CCDirector::sharedDirector()->getWinSize().width,
			CCDirector::sharedDirector()->getWinSize().height);
}

void Scene::update(float fDelta) {
	keyStateHandler();

	color_index++;
	if (color_index % 256 == 0) {
		updateLightTexture();
	}
	//update texture
//	uint8_t *pDest = static_cast<uint8_t*>(h3dMapResStream(texRes,
//			H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));
//
//	for (int row = 0; row < height; ++row) {
//		for (int col = 0; col < width; ++col) {
//
//			*pDest++ = (color_index >> 16) & 0xff; // R
//			*pDest++ = (color_index >> 8) & 0xff; // G
//			*pDest++ = color_index & 0xff; // B
//			*pDest++ = 255; // A
//		}
//	}
//
//	h3dUnmapResStream(texRes);

	for (unsigned int i = 0; i < Key::Count; ++i)
		_prevKeys[i] = _keys[i];

	_curFPS = 1.0f / fDelta;

	h3dSetOption(H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f);
	h3dSetOption(H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f);

	if (!_freezeMode) {
		_crowdSim->update(_curFPS);
	}

	// Set camera parameters
	h3dSetNodeTransform(_cam, _x, _y, _z, _rx, _ry, 0, 1, 1, 1);

	// Show stats
	h3dutShowFrameStats(_fontMatRes, _panelMatRes, _statMode);
	if (_statMode > 0) {
		if (h3dGetNodeParamI(_cam, H3DCamera::PipeResI) == _forwardPipeRes)
			h3dutShowText("Pipeline: forward", 0.03f, 0.24f, 0.026f, 1, 1, 1,
					_fontMatRes);
		else
			h3dutShowText("Pipeline: deferred", 0.03f, 0.24f, 0.026f, 1, 1, 1,
					_fontMatRes);
	}

	// Show logo
	const float ww = (float) h3dGetNodeParamI(_cam, H3DCamera::ViewportWidthI)
			/ (float) h3dGetNodeParamI(_cam, H3DCamera::ViewportHeightI);
	const float ovLogo[] = { ww - 0.4f, 0.8f, 0, 1, ww - 0.4f, 1, 0, 0, ww, 1,
			1, 0, ww, 0.8f, 1, 1 };
	h3dShowOverlays(ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _logoMatRes, 0);

	// Render scene
	h3dRender(_cam);

	// Finish rendering of frame
	h3dFinalizeFrame();

	// Remove all overlays
	h3dClearOverlays();

	// Write all messages to log file
	h3dutDumpMessages();
}

bool Scene::onEvent(const CCEvent& e) {
	switch (e.Type) {
	case CCEvent::Quit:
		release();
		return true;
	case CCEvent::TouchBegan:
		_pointerDown = true;
		_pointerX = e.Touch.X;
		_pointerY = e.Touch.Y;
		break;
	case CCEvent::TouchEnded:
	case CCEvent::TouchCancelled:
		_pointerDown = false;
		break;
	case CCEvent::TouchMoved:
		if (_pointerDown) {
			mouseMoveEvent(e.Touch.X - _pointerX, e.Touch.Y - _pointerY);
			_pointerX = e.Touch.X;
			_pointerY = e.Touch.Y;
		}
		break;
	case CCEvent::KeyPressed:
		_keys[e.Key.Code] = true;
		break;
	case CCEvent::KeyReleased:
		_keys[e.Key.Code] = false;
		break;
	}

	return false;
}

void Scene::release() {
	delete _crowdSim;
	_crowdSim = 0x0;

	// Release engine
	h3dRelease();
}

void Scene::resize(int width, int height) {
	//init private variables
	m_iWinHeight = height;
	m_iWinWidth = width;

	// Resize viewport
	h3dSetNodeParamI(_cam, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportWidthI, width);
	h3dSetNodeParamI(_cam, H3DCamera::ViewportHeightI, height);

	// Set virtual camera parameters
	h3dSetupCameraView(_cam, 45.0f, (float) width / height, 0.1f, 1000.0f);
	h3dResizePipelineBuffers(_deferredPipeRes, width, height);
	h3dResizePipelineBuffers(_forwardPipeRes, width, height);

	fRightEdge = (float) (m_fZNear * tan(m_fFOVy * M_PI / 360.0));
	float fAspectRatio = ((float) m_iWinHeight) / ((float) m_iWinWidth);
	fTopEdge = fRightEdge * fAspectRatio;

	xStep = 2 * fRightEdge / 16;
	yStep = 2 * fTopEdge / 16;
	quantizedY[0] = -100;
	quantizedX[0] = -100;
	for (int i = 1; i < 17; i++) {
		quantizedY[i] = -fTopEdge + i * yStep;
		quantizedX[i] = -fRightEdge + i * xStep;
	}
	quantizedY[17] = 100;
	quantizedX[17] = 100;
}

void Scene::keyStateHandler() {
	// ----------------
	// Key-press events
	// ----------------
	if (_keys[Key::Space] && !_prevKeys[Key::Space])  // Space
			{
		if (++_freezeMode == 3)
			_freezeMode = 0;
	}

	if (_keys[Key::F3] && !_prevKeys[Key::F3])  // F3
			{
		if (h3dGetNodeParamI(_cam, H3DCamera::PipeResI) == _forwardPipeRes)
			h3dSetNodeParamI(_cam, H3DCamera::PipeResI, _deferredPipeRes);
		else
			h3dSetNodeParamI(_cam, H3DCamera::PipeResI, _forwardPipeRes);
	}

	if (_keys[Key::F7] && !_prevKeys[Key::F7])  // F7
		_debugViewMode = !_debugViewMode;

	if (_keys[Key::F8] && !_prevKeys[Key::F8])  // F8
		_wireframeMode = !_wireframeMode;

	if (_keys[Key::F6] && !_prevKeys[Key::F6])  // F6
			{
		_statMode += 1;
		if (_statMode > H3DUTMaxStatMode)
			_statMode = 0;
	}

	// --------------
	// Key-down state
	// --------------
	if (_freezeMode != 2) {
		float curVel = _velocity / _curFPS;

		if (_keys[Key::LShift])
			curVel *= 5;	// LShift

		if (_keys[Key::W]) {
			// Move forward
			_x -= sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			_y -= sinf(-degToRad(_rx)) * curVel;
			_z -= cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
		}
		if (_keys[Key::S]) {
			// Move backward
			_x += sinf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
			_y += sinf(-degToRad(_rx)) * curVel;
			_z += cosf(degToRad(_ry)) * cosf(-degToRad(_rx)) * curVel;
		}
		if (_keys[Key::A]) {
			// Strafe left
			_x += sinf(degToRad(_ry - 90)) * curVel;
			_z += cosf(degToRad(_ry - 90)) * curVel;
		}
		if (_keys[Key::D]) {
			// Strafe right
			_x += sinf(degToRad(_ry + 90)) * curVel;
			_z += cosf(degToRad(_ry + 90)) * curVel;
		}
	}
}

void Scene::mouseMoveEvent(float dX, float dY) {
	if (_freezeMode == 2)
		return;

	// Look left/right
	_ry -= dX / 100.0f * 30.0f;

	// Loop up/down but only in a limited range
	_rx += dY / 100.0f * 30.0f;
	if (_rx > 90)
		_rx = 90;
	if (_rx < -90)
		_rx = -90;
}

void Scene::updateLightTexture() {
	uint8_t *pTileTex = static_cast<uint8_t*>(h3dMapResStream(tileTex,
			H3DTexRes::ImageElem, 0, H3DTexRes::ImgPixelStream, false, true));

	//drop all counts to 0
	for (int i = 0; i < (16 * 16 * 16); i++)
		pTileTex[i * 64] = 0;

	//get camera view matrix
	const float *camTrans;
	h3dGetNodeTransMats(_cam, 0x0, &camTrans);
	Matrix4f viewMat(Matrix4f(camTrans).inverted());

	//for each light
	for (int i = 0; i < m_vPointLights.size(); i++) {
		//transform position to view space
		LOGI("Point Light %d position: %f, %f, %f\n", i,
				m_vPointLights[i].position.x, m_vPointLights[i].position.y,
				m_vPointLights[i].position.z);
		Vec3f R = viewMat * m_vPointLights[i].position;
		//write to tile texture
		tileIt(R, m_vPointLights[i].r, m_vPointLights[i].lightIndex, pTileTex);
	}

	h3dUnmapResStream(tileTex);

//	for (int row = 0; row < tTexH; ++row) {
//		for (int col = 0; col < tTexW; ++col) {
//			*pDest++ = 0; // R
//			*pDest++ = 255; // G
//			*pDest++ = 0; // B
//			*pDest++ = 255; // A
//		}
//	}
}

void Scene::tileIt(Vec3f R, float r, uint8_t lightIndex, uint8_t* texPointer) {
	//Thinking about it :
	//First we will cut the sphere by Z planes from min to max and center
	//becouse projection from the same Z preserves angles we wil project all x,y points
	//to m_fNear, scale radius and do the same as with spot light

	//Now we calculate Z extents of spotlight points

	R.z = -R.z;

	float minZ = R.z - r;
	float maxZ = R.z + r;

	//get Quantized Z range
	int qZMax, qZMin;
	getQZRange(minZ, maxZ, &qZMin, &qZMax);

	//TODO: This could cause problems
	int centerZ;
	for (centerZ = qZMin; centerZ < qZMax; centerZ++) {
		if (R.z < quantizedZ[centerZ])
			break;
	}
	LOGI("centerZ = %d\n", centerZ);

	//check if whole thing is in one z subdivision
	if (qZMax < qZMin) {
		//TODO: flatten the whole thing
		//projected center cut - if it doesn't work we'll multiply rcut with 1.1
		Vec3f Rcut = R;
		float rcut = r;
		//cutSphere(R.z, R, r, Rcut, rcut);
		project2Near(rcut, R.z);
		project2Near(Rcut);
		markZRowTilesSphere(&Rcut, rcut, qZMin, lightIndex, texPointer);
		return;
	}

	//Projected center cut
	Vec3f Rcenter = Vec3f(R);
	float rcenter = r;

	project2Near(Rcenter);
	project2Near(rcenter, R.z);

	LOGI("To near projected %f, %f, %f center: %f, %f, %f \t %f", R.x, R.y, R.z,
			Rcenter.x, Rcenter.y, Rcenter.z, r);

	if (qZMax == qZMin) {
		//only one Z cut
		Vec3f Rcut;
		float rcut;
		cutSphere(quantizedZ[qZMax], R, r, Rcut, rcut);
//		project2Near(Rcenter);
//		r = r * m_fZNear / R.z;
		if (R.z < quantizedZ[qZMax]) {
			//sphere center below the cut
			if (qZMin > 0)
				markZRowTilesSphere(&Rcenter, rcenter, qZMin - 1, lightIndex,
						texPointer);
			markZRowTilesSphere(&Rcut, rcut, qZMin, lightIndex, texPointer);
		} else {
			markZRowTilesSphere(&Rcenter, rcenter, qZMin, lightIndex,
					texPointer);
			if (qZMin > 0)
				markZRowTilesSphere(&Rcut, rcut, qZMin - 1, lightIndex,
						texPointer);
		}

		return;
	}

	//Need top and bottom cuts
	float rtop, rbottom;
	Vec3f Rtop, Rbottom;
	for (int z = qZMin; z <= qZMax; z++) {
		LOGI("Cutting sphere: z = %f\n", quantizedZ[z]);
		cutSphere(quantizedZ[z], R, r, Rtop, rtop);
		LOGI("cutSphere: %f, %f, %f \t %f", Rtop.x, Rtop.y, Rtop.z, rtop);
		rtop = rtop * m_fZNear / Rtop.z;
		project2Near(Rtop);
		LOGI("project2Near: %f, %f, %f \t %f", Rtop.x, Rtop.y, Rtop.z, rtop);
		if (z > 0 ||) {
			if (z == qZMin) {
				//there's no bottom
				markZRowTilesSphere(&Rtop, rtop, qZMin - 1, lightIndex,
						texPointer);
			} else {
				if (z == centerZ)
					flattenSphere(Rbottom, Rcenter, rbottom, rcenter);
				flattenSphere(Rbottom, Rtop, rbottom, rtop);
				markZRowTilesSphere(&Rbottom, rbottom, z - 1, lightIndex,
						texPointer);
			}
		}
		Rbottom = Rtop;	//TODO : does this work?
		rbottom = rtop;
	}
	//Need to mark the top row
	markZRowTilesSphere(&Rtop, rtop, qZMax, lightIndex, texPointer);

}

void Scene::getQZRange(float ZMin, float ZMax, int* lower, int* upper) {
	int i = 0;
	for (; i < numZDivisions; i++)
		if (ZMin <= quantizedZ[i])
			break;
	(*lower) = i; // - 1;
	for (; i < numZDivisions; i++)
		if (ZMax <= quantizedZ[i])
			break;
	(*upper) = i - 1;
}

void Scene::getQYRange(float YMin, float YMax, int* lower, int* upper) {
	int i = 0;
	for (; i < numYDivisions; i++)
		if (YMin <= quantizedY[i])
			break;
	(*lower) = i; // - 1;
	for (; i < numYDivisions; i++)
		if (YMax <= quantizedY[i])
			break;
	(*upper) = i - 1;
}

//projects a float value at 'z' to a fixed Z plane
//l ... (in/out) scalar to be projected
//z ... (in) z of l
void Scene::project2Near(float& l, float Z) {
	l = l * m_fZNear / Z;
}

//projects a Vec3 to a fixed Z plane
//point ... (in/out) Vec3 to be projected
void Scene::project2Near(Vec3f& point) {
	point = point * (m_fZNear / point.z);
}

//returns a bounding circle
//Bottom ... (in/out) circle 1 center position
//rbottom ... (in/out) circle 1 radius
//Top ... (in) circle 2 center position
//rtop ... (in) circle 2 radius
void Scene::flattenSphere(Vec3f& Bottom, Vec3f& Top, float& rbottom,
		float& rtop) {
	Vec3f ndelta = (Top - Bottom).normalized();
	Vec3f T1 = Bottom + ndelta * rbottom;
	Vec3f T2 = Top - ndelta * rtop;
	Bottom = (T1 + T2) / 2;
	rbottom = (T2 - T1).length() / 2;
}

//returns the position and radius of a circle
//rc^2  = r^2 - dZ^2
//RC = (R.x, R.y, Z)
void Scene::cutSphere(float Z, Vec3f R, float r, Vec3f& RCut, float& rc) {
	RCut.x = R.x;
	RCut.y = R.y;
	RCut.z = Z;

	float dZ = Z - R.z;
	rc = sqrt(r * r - dZ * dZ);
}

void Scene::markYRowTilesSphere(float from, float to, int count, uint8_t Z,
		uint8_t Y, uint8_t lightIndex, uint8_t* texPointer) {
	uint8_t ixStart = (int) ((from + fRightEdge) / xStep);
	uint8_t ixStop = (int) ((to + fRightEdge) / xStep);

	ixStart = ixStart > 0 ? ixStart < 15 ? ixStart : 15 : 0;
	ixStop = ixStop > 0 ? ixStop < 15 ? ixStop : 15 : 0;
	LOGI("Marking tiles from %d to %d for Z=%d and Y=%d\n", ixStart, ixStop, Z,
			Y);
	int ZYindex = 256 * Z + 16 * Y;
	for (int j = ixStart; j <= ixStop; j++) {
		int index = ZYindex + j;
		uint8_t count = texPointer[index];
		LOGI("Calculated index = 256 * Z + 16 * Y + j = %d\n", index);
		if (texPointer[index + count + 4] != lightIndex) {
			texPointer[index + count + 4] = lightIndex; //TODO : check if this works
			texPointer[index]++; //increment count
		}
	}

}

void Scene::markZRowTilesSphere(Vec3f* R, float r, uint8_t Z,
		uint8_t lightIndex, uint8_t* texPointer) {
	float rx = R->x;
	float ry = R->y;

	//TODO : fix it
	int iyMin, iyMax;
	if (ry - r + fTopEdge > 0)
		iyMin = (int) ((ry - r + fTopEdge) / yStep) + 1;
	else
		iyMin = 0;

	if (ry + r + fTopEdge > 0)
		iyMax = (int) ((ry + r + fTopEdge) / yStep);
	else
		iyMax = 0;

	LOGI("calculating iYmin: ((ry - r + fTopEdge) / yStep : %f / %f = %d",
			ry - r + fTopEdge, yStep, iyMin);

	iyMin = iyMin > 0 ? iyMin < 15 ? iyMin : 15 : 0;
	iyMax = iyMax > 0 ? iyMax < 15 ? iyMax : 15 : 0;

	LOGI("iyMin, iyMax, fTopEdge,  yStep, ry: %d, %d, %f, %f, %f\n", iyMin,
			iyMax, fTopEdge, yStep, ry);

	int center = 0;
	if (iyMin > iyMax) {
		markYRowTilesSphere(rx - r, rx + r, 0, Z, iyMax, lightIndex,
				texPointer);
	} else if (iyMax == iyMin) {
		float y = -fTopEdge + iyMin * yStep;
		float dy = ry - y;
		float rc;
		rc = sqrt(r * r - dy * dy);
		if (y > ry) {
			markYRowTilesSphere(rx - r, rx + r, 0, Z, iyMax - 1, lightIndex,
					texPointer);
			markYRowTilesSphere(rx - rc, rx + rc, 0, Z, iyMax, lightIndex,
					texPointer);
		} else {
			markYRowTilesSphere(rx - r, rx + r, 0, Z, iyMax, lightIndex,
					texPointer);
			markYRowTilesSphere(rx - rc, rx + rc, 0, Z, iyMax - 1, lightIndex,
					texPointer);
		}
	} else {
		for (int i = iyMin; i <= iyMax; i++) {
			float y = -fTopEdge + i * yStep;
			float dy = ry - y;
			float rc;
			if (y > ry && y < ry + yStep) {
				//center between ys
				//mark twice
				center = 1;
				rc = r;
				markYRowTilesSphere(rx - rc, rx + rc, 0, Z, i - 1, lightIndex,
						texPointer);

			}
			rc = sqrt(r * r - dy * dy);

			LOGI("markZRowTilesSphere: %f %f %f\n", rx, rc, Z);
			markYRowTilesSphere(rx - rc, rx + rc, 0, Z, i + center - 1,
					lightIndex, texPointer);
		}
	}
}

//packs a float value to a char array
//float range between -500 and 500 0-256
uint8_t* Scene::pack_float(float val, uint8_t* chbuffer) {
	val = val / 4 + 125;
	uint8_t r, g, b, a;
	r = val;
	g = ((int) (val * 256)) & 0xff;
	b = ((int) (val * 65536)) & 0xff;
	a = ((int) (val * 16777216)) & 0xff;
	*chbuffer++ = r; // R
	*chbuffer++ = g; // G
	*chbuffer++ = b; // B
	*chbuffer++ = a; // A
	return chbuffer;
}

void Scene::tileIt2(Vec3f R, float r, uint8_t lightIndex, uint8_t* texPointer) {
	R.z = -R.z;

	float minZ = R.z - r;
	float maxZ = R.z + r;

	//get Quantized Z range
	int qZMax, qZMin;
	getQZRange(minZ, maxZ, &qZMin, &qZMax);

	//TODO: This could cause problems
	int centerZ;
	for (centerZ = qZMin; centerZ < qZMax; centerZ++) {
		if (R.z < quantizedZ[centerZ])
			break;
	}

	centerZ = centerZ == 0 ? 0 : centerZ-1;
	LOGI("centerZ = %d\n", centerZ);
}
