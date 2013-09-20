/*
 * LightTiler.cpp
 *
 *  Created on: 5. sep. 2013
 *      Author: ZCule
 */

//#include "LightTiler.h"
#include "util.h"
#include <stdlib.h>


namespace Horde3D {
LightTiler::LightTiler() {
	// TODO Auto-generated constructor stub

}

LightTiler::~LightTiler() {
	// TODO Auto-generated destructor stub
}

//Accepts a spotlight whose V point and direction have already been transformed to view space
//
void LightTiler::tileIt(Spotlight* spot) {
	//given a direction vector, calculate new "up" and "right" vectors
	//we are presuming that light starting direction is 1z
	//so we need to find a transformation to rotate 1z to dir
	//
	//1. axis of rotation is 1z cross dir
	//2. angle is 1z dot dir
	//3. quaternion rotate 1y -> up; 1x -> right

	Vec3f rotaxis = Vec3f(0, 0, 1).cross(spot->dir);
	GLfloat angle = Vec3f(0, 0, 1).dot(spot->dir);

	Vec3f up = Vec3f(0, 1, 0).rotate(rotaxis, angle);
	Vec3f right = Vec3f(1, 0, 0).rotate(rotaxis, angle);

	btVector3 T1, T2, T3, T4, T, V;

	V = spot->V;
	T = spot->V + spot->dir * spot->height;
	T1 = T + up * spot->r + right * spot->r;
	T2 = T + up * spot->r - right * spot->r;
	T3 = T - up * spot->r + right * spot->r;
	T4 = T - up * spot->r - right * spot->r;

	//That's possibly not a good idea
	//What it does is it limits rotation axis to a plane Z = const.
	//this has an effect of keeping points on the same Y plane.
	//That way we still have a situation where a only one point intersects
	//a Z plane.
	//That can be avoided if we rotate a pyramid so that at least two T points are
	//on the same z plane.
	//All we have to do is find a point on a circular endcap
	//that is farthest in the direction of 1z. (RTCD - page 165)
	//this direction is U = (1z x dir) x dir
	//U is our new right or up vector we will generalize it as up
	//new right vector is a cross of dir and U (R = dir x U)

	Vec3f U, R;

	U = Vec3f(0, 0, 1).cross(spot->dir).cross(spot->dir);
	R = spot->dir.cross(U);

	V = spot->V;
	T = spot->V + spot->dir * spot->height;
	T1 = T + U * spot->r + R * spot->r;
	T2 = T + U * spot->r - R * spot->r;
	T3 = T - U * spot->r + R * spot->r;
	T4 = T - U * spot->r - R * spot->r;

	//Problem of a previous method was that we had 5 different (Z) critical
	//points. This way we reduce the number to 3 (We only have 3 different Z values
	//namely T1 and T2 have the same "z" and T3 and T4 the same)

	//Now we calculate Z extents of spotlight points
	btVector3* minZ;
	btVector3* maxZ;

	minZ = minZ(&V, &T1);
	maxZ = maxZ(&V, &T1);
	minZ = minZ(minZ, &T3);
	maxZ = maxZ(maxZ, &T3);

	//get Quantized Z range
	int qZMax, qZMin;
	getQZRange(minZ->getZ(), maxZ->getZ(), &qZMin, &qZMax);

	//check if whole thing is in one z subdivision
	if (qZMax <= qZMin) {
		// flatten the whole thing
	}

	//Every cross section of such pyramid with a Z-plane is a trapeze with one side equal to
	//T1-T2 moved in Up direction by dZ/cos(angle(1z,Up)) = dZ/dot(1z,dir) other two points are

	//We have to separate the case when dir vector is colinear with 1z vector
	if (spot->dir.dot(btVector3(0.0f, 0.0f, 1.0f))) { //special case
		//all cross sections are squares
	} else {
		//Every cross section of such pyramid with a Z-plane is a trapeze with one side equal to
		//T1-T2
		//instead of looking at it as trapezes when flattening its better to think of them as
		//number of lines parallel to T1-T2 defined by a sequence of pairs of points
		//                               /
		//                              //
		//                           / //
		//                            //
		//                           /
		//all lines have the same simetral direction of which is normalized projection of dir vector on Z-plane

		// normalized projection of dir to Z
		btVector3 ln =
				spot->dir.cross(btVector3(0.0f, 0.0f, 1.0f)).normalized();

		//is V in the middle? do I need this...hm

		//first row between min and first qZ plane
		//
		btVector3 *middle;
		if (minZ == *V) {
			middle = (maxZ == *T1) ? *T3 : *T1;
		} else if (maxZ == *V) {
			middle = (minZ == *T1) ? *T3 : *T1;
		} else {
			middle = *V;
		}

		btVector3 points[1024];
		btVector3 planeZVector = new btVector3(0.0f, 0.0f, 1.0f);
		float t;
		int start = 0, stop = 0;
		//Making points invariant
		btVector3 U1, U2, D1, D2, M1, M2;
		if (middle == *V) {
			if (maxZ == T1) {
				U1 = T1;
				U2 = T2;
				D1 = T3;
				D2 = T4;
			} else {
				U1 = T3;
				U2 = T4;
				D1 = T1;
				D2 = T2;
			}
			M1 = V;
			M2 = V;
		} else if (maxZ == *V) {
			if (middle == *T1) {
				M1 = T1;
				M2 = T2;
				D1 = T3;
				D2 = T4;
			} else {
				M1 = T3;
				M2 = T4;
				D1 = T1;
				D2 = T2;
			}
			U1 = V;
			U2 = V;
		} else {
			if (middle == *T1) {
				M1 = T1;
				M2 = T2;
				U1 = T3;
				U2 = T4;
			} else {
				M1 = T3;
				M2 = T4;
				U1 = T1;
				U2 = T2;
			}
			D1 = V;
			D2 = V;
		}

		btVector3 mPoints[4];
		mPoints[0] = M1;
		mPoints[1] = M2;
		IntersectSegmentZPlane(U1, D1, middle->getZ(), t, mPoints[2]);
		IntersectSegmentZPlane(U2, D2, middle->getZ(), t, mPoints[3]);

		btVector3 zPoints[qZMax - qZMin + 1][4];
		for (int i = qZMin; i <= qZMax; i++) {
			IntersectSegmentZPlane(U1, D1, quantizedZ[i], t,
					zPoints[i - qZMin][0]);
			IntersectSegmentZPlane(U2, D2, quantizedZ[i], t,
					zPoints[i - qZMin][1]);
			if (quantizedZ[i] < M1.getZ()) {
				IntersectSegmentZPlane(M1, D1, quantizedZ[i], t,
						zPoints[i - qZMin][2]);
				IntersectSegmentZPlane(M2, D2, quantizedZ[i], t,
						zPoints[i - qZMin][3]);
			} else {
				IntersectSegmentZPlane(M1, U1, quantizedZ[i], t,
						zPoints[i - qZMin][2]);
				IntersectSegmentZPlane(M2, U2, quantizedZ[i], t,
						zPoints[i - qZMin][3]);
			}
		}

		//Now we have all points and fun can begin
		//
		//bottom interval
		points[stop++] = D1;
		points[stop++] = D2;
		if (middle->getZ() <= quantizedZ[qZMin]) {
			//two sections below the first Z plane
			//this results in three shapes to be flattened
			points[stop++] = mPoints[0];
			points[stop++] = mPoints[1];
			points[stop++] = mPoints[2];
			points[stop++] = mPoints[3];
		}
		points[stop++] = zPoints[0][0];
		points[stop++] = zPoints[0][1];
		points[stop++] = zPoints[0][2];
		points[stop++] = zPoints[0][3];
		ConvexHull ch;
		flatten(points, stop - 1, right, ln, ch);
		markZRowTiles(ch, qZMin, spot->lightIndex);

		for (int i = 1; i < qZMax - qZMin; i++) {
			stop = 0;
			points[stop++] = zPoints[i][0];
			points[stop++] = zPoints[i][1];
			points[stop++] = zPoints[i][2];
			points[stop++] = zPoints[i][3];
			if (middle->getZ() <= quantizedZ[qZMin + i + 1]) {
				//two sections below the upper Z plane
				//this results in three shapes to be flattened
				points[stop++] = mPoints[0];
				points[stop++] = mPoints[1];
				points[stop++] = mPoints[2];
				points[stop++] = mPoints[3];
			}
			points[stop++] = zPoints[i + 1][0];
			points[stop++] = zPoints[i + 1][1];
			points[stop++] = zPoints[i + 1][2];
			points[stop++] = zPoints[i + 1][3];
			flatten(points, stop - 1, right, ln, ch);
			markZRowTiles(ch, qZMin + i, spot->lightIndex);
		}

		//top interval
		stop = 0;
		int zpindex = qZMax - qZMin;
		points[stop++] = zPoints[zpindex][0];
		points[stop++] = zPoints[zpindex][1];
		points[stop++] = zPoints[zpindex][2];
		points[stop++] = zPoints[zpindex][3];
		if (middle->getZ() <= quantizedZ[qZMin]) {
			//two sections above the first Z plane
			//this results in three shapes to be flattened
			points[stop++] = mPoints[0];
			points[stop++] = mPoints[1];
			points[stop++] = mPoints[2];
			points[stop++] = mPoints[3];
		}

		points[stop++] = U1;
		points[stop++] = U2;
		ConvexHull ch;
		flatten(points, stop - 1, right, ln, ch);
		markZRowTiles(ch, qZMax, spot->lightIndex);

	}

}

//Returns index of a plane above min and below max
void LightTiler::getQZRange(GLfloat ZMin, GLfloat ZMax, int* lower,
		int* upper) {
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

//return a convex hull connectivity from
//points .... array of points
//count .... size of array
//l .... line vector(T1-T2)
//ln .... line normal
//prallel lines on a Z=const plane are parallel under perspective projections -> similar
//Projecting points to a single Z plane
//If we always project to a fixed plane than koeficients can be fixed
void LightTiler::flatten(btVector3 *points, int count, btVector3 l,
		btVector3 ln, ConvexHull &ch) {
	//TODO : For now we are using a simplified method of returning a bounding rectangle
	//Maybe good enough, but it should return a convex hull
	//NOT GOOD

	//FIND max line length and max distance between lines
	// distance between lines by finding min and max signed distance between line point and first point projected to "ln"

	btVector3 T1, T2, T3, T4;
	//ConvexH c; - bullet versionn

	int idmin = 0, idmax = 0;
	float lmax = 0, len;
	float d[count / 2];
	points[0] = points[0] * m_fZNear / points[0].getZ();
	for (int i = 0; i < count / 2; i++) {
		points[i] = points[i] * m_fZNear / points[i].getZ();
		d[i] = ln.dot(btVector3(points[i] - points[0]));
		len = btVector3(points[i] - points[i + 1]).length();
		idmin = d[i] < d[idmin] ? i : idmin;
		idmax = d[i] > d[idmax] ? i : idmax;
		lmax = len > lmax ? len : lmax;
	}

	//bounding rectangle is on dmin and dmax lines
	// and extends lmax/2 in l direction from the middle on both ends

	//1. dmin
	T1 = btVector3(points[idmin] + points[idmax]) / 2 + l * lmax / 2;
	T2 = btVector3(points[idmin] + points[idmax]) / 2 - l * lmax / 2;

	//2. dmax
	T3 = btVector3(points[idmax] + points[idmax]) / 2 + l * lmax / 2;
	T4 = btVector3(points[idmax] + points[idmax]) / 2 - l * lmax / 2;

	btVector3 chPoints[4] { T1, T2, T3, T4 };
	int conn[8] { 1, 2, 0, 3, 0, 3, 1, 2 };
	ch = new ConvexHull(chPoints, 4, conn);

}

//First order points in Y direction VIEW SPACE!!!
void LightTiler::markZRowTiles(ConvexHull& ch, int Z, int lightIndex) {
	float *indArr[ch.count];
	float yzk[ch.count];
	int sortdIndex[ch.count];
	for (int i = 0; i < ch.count; i++) {
		yzk[i] = ch.points[i].getY(); // / ch.points[i].getZ(); //They are already projected
		indArr[i] = &yzk[i];
	}

	qsort(indArr, ch.count, sizeof *indArr, compre_floats);

	for (int i = 0; i < ch.count; i++) {
		sortdIndex[i] = indArr[i] - yzk;
	}

	//find Y extents
	int iqYmin, iqYmax;
	getQYRange(yzk[sortdIndex[0]], yzk[sortdIndex[ch.count - 1]], &iqYmin,
			&iqYmax);

	//loop throgh points min + 1 to max - 1 and find cross sections
	//so there are count-2 cross sections

	float crossections[(ch.count - 2 + iqYmax - iqYmin) * 2];
	int T1, T2, T3, T4;
	T1 = sortdIndex[0];
	T2 = ch.connectivity[2 * T1];
	T3 = T1;
	T4 = ch.connectivity[2 * T1 + 1];
	int crosIndex = 0, iY = iqYmin;
	for (int i = 0; i < ch.count - 2; i++) {
		int Ttemp = sortdIndex[i];

		//cross section is y[i] with T1-T2 and T3-T4
		float x1, x2, x3, x4, y1, y2, y3, y4;
		x1 = ch.points[T1].getX();
		x2 = ch.points[T2].getX();
		x3 = ch.points[T3].getX();
		x4 = ch.points[T4].getX();

		y1 = ch.points[T1].getY();
		y2 = ch.points[T2].getY();
		y3 = ch.points[T3].getY();
		y4 = ch.points[T4].getY();
		//TODO : Check if dX = 0 -> divide by 0
		float a12 = x1 != x2 ? (y2 - y1) / (x2 - x1) : 500000;
		float a34 = x3 != x4 ? (y4 - y3) / (x4 - x3) : 500000;

		if (ch.points[Ttemp] > quantizedY[iY]) {
			float a = quantizedY[iY];
			crossections[2 * crosIndex] = (y1 - a12 * x1) / a; //(a - a12);
			crossections[2 * crosIndex + 1] = (y3 - a34 * x3) / a; //(a - a34);
			markYRowTiles(crossections, crosIndex + 1, Z, iY, lightIndex);
			crossections[0] = crossections[2 * crosIndex];
			crossections[1] = crossections[2 * crosIndex + 1];
			crosIndex = 1;
		}

		float a = ch.points[Ttemp].getY(); // / ch.points[Ttemp].getX();
		crossections[2 * crosIndex] = (y1 - a12 * x1) / a; //(a - a12);
		crossections[2 * crosIndex + 1] = (y3 - a34 * x3) / a; //(a - a34);
		crosIndex++;

		int C1temp, C2temp;
		C1temp = ch.connectivity[2 * Ttemp];
		C2temp = ch.connectivity[2 * Ttemp + 1];
		if (T2 == Ttemp) {
			T2 = T1 == C1temp ? C2temp : C1temp;
			T1 = Ttemp;
		} else if (T4 == Ttemp) {
			T4 = T3 == C1temp ? C2temp : C1temp;
			T3 = Ttemp;
		}
	}
}

void LightTiler::getQYRange(GLfloat YMin, GLfloat YMax, int* lower,
		int* upper) {
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

//Marks tiles
//Input is an array of pairs of points that represent ranges of X
void LightTiler::markYRowTiles(float* crosssections, int count, int Z, int Y,
		int lightIndex) {
	for (int i = 0; i < count; i++) {
		float xStart =
				crosssections[2 * i] < crosssections[2 * i + 1] ?
						crosssections[2 * i] : crosssections[2 * i + 1];
		float xStop =
				crosssections[2 * i] > crosssections[2 * i + 1] ?
						crosssections[2 * i] : crosssections[2 * i + 1];
		int ixStart = (int) (xStart / xStep);
		int ixStop = (int) (xStop / xStep);
		for (int j = ixStart; j <= ixStop; j++) {
			int index = 256 * Z + 16 * Y + j;
			if (lists[index][counts[index]] != lightIndex)
				lists[index][counts[index]++] = lightIndex; //TODO : check if this works
		}
	}

}

LightTiler::LightTiler(GLfloat m_fFOVy, int32_t m_iWinWidth,
		int32_t m_iWinHeight) {
	this->m_fFOVy = m_fFOVy;
	this->m_iWinWidth = m_iWinWidth;
	this->m_iWinHeight = m_iWinHeight;

	fRightEdge = (GLfloat) (m_fZNear * tan(m_fFOVy * M_PI / 360.0));
	GLfloat fAspectRatio = ((GLfloat) m_iWinHeight) / ((GLfloat) m_iWinWidth);
	fTopEdge = fRightEdge / fAspectRatio;

	xStep = 2 * fRightEdge / 16;
	yStep = 2 * fTopEdge / 16;

	for (int i = 0; i < 17; i++) {
		quantizedY[i] = -fTopEdge + i * yStep;
		quantizedX[i] = -fRightEdge + i * xStep;
	}
}
;

void LightTiler::tileIt(PointLight* point) {
	//Thinking about it :
	//First we will cut the sphere by Z planes from min to max and center
	//becouse projection from the same Z preserves angles we wil project all x,y points
	//to m_fNear, scale radius and do the same as with spot light

	//Now we calculate Z extents of spotlight points
	btVector3 R = point->position;
	float r = point->r;

	float minZ = R.getZ() - r;
	float maxZ = R.getZ() + r;

	//get Quantized Z range
	int qZMax, qZMin;
	getQZRange(minZ, maxZ, &qZMin, &qZMax);

	int centerZ;
	for (centerZ = qZMin; centerZ < qZMax; centerZ++) {
		if (R.getZ() < quantizedZ[centerZ])
			break;
	}

	//check if whole thing is in one z subdivision
	if (qZMax < qZMin) {
		// flatten the whole thing
		return;
	}

	//Projected center cut
	btVector3 Rcenter;
	float rcenter = r;

	project2Near(&Rcenter);
	project2Near(r, R.getZ());

	if (qZMax == qZMin) {
		//only one Z cut
		btVector3 Rcut;
		float rcut;
		cutSphere(quantizedZ[qZMax], point, Rcut, rcut);
		project2Near(&Rcenter);
		project2Near(r, R.getZ());
		if (R.getZ() < quantizedZ[qZMax]) {
			//sphere center below the cut
			markZRowTilesSphere(&Rcenter, r, qZMin - 1, point->lightIndex);
			markZRowTilesSphere(&Rcut, rcut, qZMin, point->lightIndex);
		} else {
			markZRowTilesSphere(&Rcenter, r, qZMin, point->lightIndex);
			markZRowTilesSphere(&Rcut, rcut, qZMin - 1, point->lightIndex);
		}

		return;
	}

	//Need top and bottom cuts
	float rtop, rbottom;
	btVector3 Rtop, Rbottom;
	for (int z = qZMin; z <= qZMax; z++) {
		cutSphere(quantizedZ[z], point, Rtop, rtop);
		project2Near(&Rtop);
		project2Near(rtop, Rtop.getZ());
		if(z==qZMin){
			//there's no bottom
			markZRowTilesSphere(&Rtop, rtop, qZMin - 1, point->lightIndex);
		}else{
			if(z==centerZ) flattenSphere(&Rbottom,&Rcenter,&rbottom,&rcenter);
			flattenSphere(&Rbottom,&Rtop,&rbottom,&rtop);
			markZRowTilesSphere(&Rbottom, rbottom, z - 1, point->lightIndex);
			Rbottom = Rtop;//TODO : does this work?
			rbottom = rtop;
		}
	}
	//Need to mark the top row
	markZRowTilesSphere(&Rtop, rtop, qZMax, point->lightIndex);

}

//returns the position and radius of a circle
//rc^2  = r^2 - dZ^2
//RC = (R.x, R.y, Z)
void LightTiler::cutSphere(float Z, PointLight* p, btVector3& RCut, float& rc) {
	RCut.m_floats[0] = p->position.getX();
	RCut.m_floats[1] = p->position.getY();
	RCut.m_floats[2] = Z;

	float dZ = Z - p->position.getZ();
	rc = sqrt(p->r * p->r + dZ + dZ);
}

void LightTiler::project2Near(float& l, float Z) {
	l = l * m_fZNear / Z;
}

void LightTiler::project2Near(btVector3* point) {
	*point = *point * m_fZNear / point->getZ();
}

void LightTiler::markZRowTilesSphere(btVector3* R, float r, int Z,
		int lightIndex) {
	float rx = R->getX();
	float ry = R->getY();

	//TODO : fix it
	int iyMin, iyMax;
	iyMin = (int)((ry - r + fTopEdge)/yStep);
	iyMax = (int)((ry + r + fTopEdge)/yStep);

	int center = 0;
	for(int i = iyMin; i<iyMax; i++){
		float y = -fTopEdge + i*yStep;
		float dy = ry - y;
		float rc;
		if(y>ry && y<ry+yStep){
			dy = 0;
			center = 1;
			rc = r;
		}else{
			rc = sqrt(r*r - dy*dy);
		}

		markYRowTilesSphere(rx-rc,rx+rc,0,Z,i+center,lightIndex);
	}
}

void LightTiler::markYRowTilesSphere(float from, float to, int count, int Z,
		int Y, int lightIndex) {
			int ixStart = (int) (from / xStep);
			int ixStop = (int) (to / xStep);
			for (int j = ixStart; j <= ixStop; j++) {
				int index = 256 * Z + 16 * Y + j;
				if (lists[index][counts[index]] != lightIndex)
					lists[index][counts[index]++] = lightIndex; //TODO : check if this works
			}

}

//flattens 2 circles into bottom
//we have 2 circles
void LightTiler::flattenSphere(btVector3 *Bottom, btVector3* Top, float* rbottom, float* rtop) {
	btVector3 delta = *Top - *Bottom;
	btVector3 delta_norm = delta.normalized();
	float r = rtop + delta.length() + rbottom;
	r = r/2;
	btVector3 R = *Top + delta_norm * (r-rtop);

}

}
