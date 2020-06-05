#include "Aviation.h"
#include "cocos2d.h"
#include "Land.h"
#include "Scene/MapScene.h"
#include "Scene/GameController.h"
#include "Common/CommonMethod.h"
#include "Common/CommonConstant.h"
#include "Incident/PopUpLayer.h"
#include "Incident/Incident.h"

USING_NS_CC;

Aviation* Aviation::create(MapScene* map_scene, int index)
{
	auto pRet = new(std::nothrow) Aviation();
	if (pRet && pRet->init())
	{
		pRet->setMapScene(map_scene);
		pRet->index_ = index;
		pRet->name_ = std::string("航空公司");
		pRet->setAnchorPoint(Vec2(0.5f, 0.5f));
		pRet->setPosition(map_scene->pos(index));
		map_scene->getMap()->addChild(pRet, 1);
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}

}


bool Aviation::onLand(Character* standing)
{
	Character* owner = nullptr;
	auto store = (dynamic_cast<StockScene*>(map_scene_->getChildByName("stock_scene"))->getStock()).at(7)->getStore();
	int total = 0;
	auto size = store.size();
	for (decltype(size) i = 0; i < size; i++)
	{
		//log("store %d :%d", i, store[i]);
		if (store[i] > total)
		{
			total = store[i];
			auto  vec= dynamic_cast<GameController*>(map_scene_->getChildByName("game_controller"))->getCharacters();
			for (auto c : vec)
			{
				if (c->getTag() == i)
				{
					owner = c;
				}
			}
		}
	}
	if (!owner)
	{
		auto pop = PopUpLayer::create();
		pop->setTitle(name_);
		pop->setContent("想要当董事长吗？快快购买本公司的股票哦，当前持仓数最多的人会成为本公司的董事长，享有公司所有权");
		pop->setCallBack([=](Ref* ref) { sendMsg(msg_make_go_apper); });
		pop->setPosition(Vec2(0, 0));
		map_scene_->addChild(pop, 51);
	}
	else
	{
		if (owner != standing)
		{
			auto pop = PopUpLayer::create();
			pop->setTitle(name_);
			pop->setContent(StringUtils::format("你需要向董事长%s缴纳出国旅游的机票%d,并出国旅游%d天", owner->getPlayerName().c_str(), aviation_value,default_stop_times));
			pop->setCallBack([=](Ref* ref)
			{
				GoOnHoliday(standing);
				standing->setMoney(standing->getMoney() - aviation_value);
				sendMsg(msg_make_go_apper);
			});
			pop->setPosition(Vec2(0, 0));
			map_scene_->addChild(pop, 51);
		}
		else sendMsg(msg_make_go_apper);

	}
	return true;
}