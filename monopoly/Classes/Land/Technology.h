#ifndef _TECHNOLOGY_H_
#define _TECHNOLOGY_H_

#include "cocos2d.h"
#include "Land.h"
#include "Common/CommonMethod.h"
#include "Scene/StorkScene.h"

#include<string>

USING_NS_CC;

class Technology : public Land {
protected:
	std::string name_;

public:
	virtual bool onLand(Character* standing) override;
	static Technology* create(MapScene* map_scene, int index);

};

#endif // !_TECHNOLOGY_H_
