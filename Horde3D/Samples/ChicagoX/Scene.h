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

#ifndef _app_H_
#define _app_H_

#include "crowd.h"
#include <string>
#include <vector>
#include "CCEvent.h"
#include "nodes/CCNode.h"
#include "PointLight.h"
#include "utMath.h"

using namespace Horde3D;


class Scene: public cocos2d::CCNode
{
public:
	Scene( );
	
	void setKeyState( int key, bool state ) { _prevKeys[key] = _keys[key]; _keys[key] = state; }

	const char *getTitle() { return "Chicago - Horde3D Sample"; }
	
	void update( float fDelta );
	bool onEvent( const cocos2d::CCEvent& e  );

	void release();
	void resize( int width, int height );

	void keyStateHandler();
	void mouseMoveEvent( float dX, float dY );

	void updateLightTexture();
	void tileIt(Vec3f R, float r, uint8_t lightIndex, uint8_t* texPointer);
	void tileIt2(Vec3f R, float r, uint8_t lightIndex, uint8_t* texPointer);
	void getQZRange(float ZMin, float ZMax, int* lower, int* upper);
	void getQYRange(float YMin, float YMax, int* lower, int* upper);
	void project2Near(float& l, float Z);
	void project2Near(Vec3f& point);
	void flattenSphere(Vec3f& Bottom, Vec3f& Top, float& rbottom, float& rtop);
	void cutSphere(float Z, Vec3f R, float r, Vec3f& RCut, float& rc);
	void markYRowTilesSphere(float from, float to, int count, uint8_t Z, uint8_t Y, uint8_t lightIndex, uint8_t* texPointer);
	void markZRowTilesSphere(Vec3f* R, float r, uint8_t Z, uint8_t lightIndex, uint8_t* texPointer);

	uint8_t* pack_float(float val, uint8_t* chbuffer);

	static float quantizedZ[17];

private:
	void keyHandler();

private:
	bool         _keys[cocos2d::Key::Count], _prevKeys[cocos2d::Key::Count];
	bool			   _pointerDown;
	float				_pointerX, _pointerY;

	float        _x, _y, _z, _rx, _ry;  // Viewer position and orientation
	float        _velocity;  // Velocity for movement
	float        _curFPS;

	int          _statMode;
	int          _freezeMode;
	bool         _debugViewMode, _wireframeMode;
	
	CrowdSim     *_crowdSim;
	
	// Engine objects
	H3DRes       _fontMatRes, _panelMatRes;
	H3DRes       _logoMatRes, _forwardPipeRes, _deferredPipeRes;
	H3DNode      _cam;
	H3DRes 		texRes;
	H3DRes 		lightColorTex; //RGB8 Texture with light color
	H3DRes 		lightRrTex; //RGBA16F Texture with x,z,y,r values
	H3DRes 		tileTex; //RGBA16F Texture with x,z,y,r values

	unsigned int color_index, tTexW, tTexH;

	int m_iWinWidth, m_iWinHeight;
	float m_fZNear;

	float m_fFOVy;

	float fRightEdge;
	float fTopEdge;

	float xStep;
	float yStep;
	unsigned short numZDivisions;
	unsigned short numYDivisions;

	float quantizedY[18]; // = {2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,128.0f,256.0f,512.0f,1024.0f};
	float quantizedX[18]; // = {2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,128.0f,256.0f,512.0f,1024.0f};

	//lists of lights aff. each tile
	//limited to 64 lights per tile
	int lists[16 * 16 * 16][64];

	//int counts[16 * 16 * 16] = { }; //counters

	std::vector<PointLight> m_vPointLights;


	std::string  _contentDir;
};

#endif // _app_H_



