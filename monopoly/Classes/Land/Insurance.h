#ifndef _INSURANCE_H_
#define _INSURANCE_H_

#include "cocos2d.h"
#include "Land.h"
#include "Common/CommonMethod.h"
#include "Scene/StorkScene.h"

#include<string>

USING_NS_CC;

class Insurance : public Land {
protected:
	std::string name_;

public:
	virtual bool onLand(Character* standing) override;
	static Insurance* create(MapScene* map_scene, int index);

};

#endif // !_INSURANCE_H_
