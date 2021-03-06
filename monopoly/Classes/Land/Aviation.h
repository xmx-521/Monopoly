#ifndef _AVIATION_H_
#define _AVIATION_H_

#include "cocos2d.h"
#include "Land.h"
#include "Common/CommonMethod.h"
#include "Scene/StockScene.h"

#include<string>

USING_NS_CC;

class Aviation : public Land {
protected:

public:
	virtual bool onLand(Character* standing) override;
	static Aviation* create(int index);

};

#endif // !_AVIATION_H_
