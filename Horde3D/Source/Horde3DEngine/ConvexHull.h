/*
 * ConvexHull.h
 *
 *  Created on: 10. sep. 2013
 *      Author: ZCule
 */

#ifndef CONVEXHULL_H_
#define CONVEXHULL_H_

namespace Horde3D {
//count points forming a convex hull vith 2*count line segments
class ConvexHull {
public:
	Vec3f points[];
	int connectivity[];
	float count;

	ConvexHull();
	ConvexHull(Vec3f *points, int count, int *connectivity) {
		this->points = points;
		this->count = count;
		this->connectivity = connectivity;
	}
	;

	virtual ~ConvexHull();
};
}
#endif /* CONVEXHULL_H_ */
