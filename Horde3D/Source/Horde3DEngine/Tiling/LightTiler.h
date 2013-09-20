/*
 * LightTiler.h
 *
 *  Created on: 5. sep. 2013
 *      Author: ZCule
 */

#ifndef LIGHTTILER_H_
#define LIGHTTILER_H_

#define	M_PI		3.14159265358979323846	/* pi */

namespace Horde3D {
class LightTiler {
public:
	int32_t m_iWinWidth, m_iWinHeight;
	GLfloat m_fZNear = 5.0;
	GLfloat m_fFOVy;
	GLfloat fRightEdge;
	GLfloat fTopEdge;

	GLfloat xStep;
	GLfloat yStep;
	GLushort numZDivisions = 16;
	GLushort numYDivisions = 16;
	GLfloat quantizedZ[17] = {0.1, 5.0, 6.8, 9.2, 12.6, 17.1, 23.2, 31.5, 42.9, 58.3, 79.2, 108, 146, 199, 271, 368, 500};
	GLfloat quantizedY[17];// = {2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,128.0f,256.0f,512.0f,1024.0f};
	GLfloat quantizedX[17];// = {2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,128.0f,256.0f,512.0f,1024.0f};

	//represents an offset to the light information texture
	//together with number of point and spotlights effecting the tile
	//tiles are indexed in ZYX order -> 256*Z + 16*Y + X
	//structure is:
	//			- 16bit offset
	//			- 8bit sotlight count
	//			- 8bit pointlight count
	int tiles[16*16*16];



	//lists of lights aff. each tile
	//limited to 64 lights per tile
	int lists[16*16*16][64];

	int counts[16*16*16]; //counters


//	btTransform* camera;
	LightTiler();
	LightTiler(GLfloat m_fFOVy, int32_t m_iWinWidth, int32_t m_iWinHeight);
	virtual ~LightTiler();

	void tileIt(Spotlight* spot);
	void tileIt(PointLight* point);

	void flatten(Vec3f *points, int count, Vec3f l, Vec3f ln, ConvexHull &ch);

	void markZRowTiles(ConvexHull &ch, int Z, int lightIndex);
	void markYRowTiles(float* crosssections, int count, int Z, int Y, int lightIndex);

	void getQZRange(GLfloat ZMin, GLfloat ZMax, int* lower, int* upper);
	void getQYRange(GLfloat YMin, GLfloat YMax, int* lower, int* upper);

	void cutSphere(float Z, PointLight* p, Vec3f &RCut, float &rc);

	void project2Near(float &l, float Z);
	void project2Near(Vec3f* point);

	void markZRowTilesSphere(Vec3f* R, float r, int Z, int lightIndex);
	void markYRowTilesSphere(float from, float to, int count, int Z, int Y, int lightIndex);
	void flattenSphere(Vec3f *Bottom, Vec3f* Top, float* rbottom, float* rtop);

};
}
#endif /* LIGHTTILER_H_ */
