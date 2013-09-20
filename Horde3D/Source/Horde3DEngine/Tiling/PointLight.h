/*
 * PointLight.h
 *
 *  Created on: 12. sep. 2013
 *      Author: ZCule
 */

#ifndef POINTLIGHT_H_
#define POINTLIGHT_H_

namespace Horde3D {
class PointLight {
public:
	int lightIndex;
	float r;
	Vec3f position;
	PointLight();
	virtual ~PointLight();
};
}
#endif /* POINTLIGHT_H_ */
