/*
 * util.h
 *
 *  Created on: 5. sep. 2013
 *      Author: ZCule
 */

#ifndef UTIL_H_
#define UTIL_H_

namespace Horde3D {

inline Vec3f* minZ(Vec3f* A, Vec3f* B) {
	return (*A).getZ() > (*B).getZ() ? B : A;
}
inline Vec3f* maxZ(Vec3f* A, Vec3f* B) {
	return (*A).getZ() > (*B).getZ() ? A : B;
}

inline int IntersectSegmentPlane(Vec3f a, Vec3f b, Plane p, float &t,
		Vec3f &q) {
// Compute the t value for the directed line ab intersecting the plane
	Vec3f ab = b - a;
	t = (p.d - p.n->dot(a)) / p.n->dot(ab);
// If t in [0..1] compute and return intersection point
	if (t >= 0.0f && t <= 1.0f) {
		q = a + t * ab;
		return 1;
	}
// Else no intersection
	return 0;
}

inline int IntersectSegmentZPlane(Vec3f a, Vec3f b, float p, float &t,
		Vec3f &q){
	Vec3f planeZVector = new Vec3f(0.0f,0.0f,0.1f);
	Plane Z = new Plane(&planeZVector,p);
	return IntersectSegmentPlane(a, b, Z, t, q);
}

inline int compre_floats(const void *a, const void *b){
	float fa = **(float **)a;
	float fb = **(float **)b;

	if (fa < fb) return 1;
	else if (fa == fb) return 0;
	else return -1;
}

}
#endif /* UTIL_H_ */
