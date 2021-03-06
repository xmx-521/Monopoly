#include "Scene/StartScene.h"
#include "Scene/MapScene.h"
#include "Scene/SettingScene.h"
#include "Scene/StockScene.h"
#include "Scene/ItemScene.h"
#include "Common/CommonMethod.h"
#include "Common/CommonConstant.h"
#include "Character/Character.h"
#include "StockScene.h"
#include "Land/Business.h"
#include "Land/Hotel.h"
#include "Land/Jail.h"
#include "Land/Hospital.h"
#include "Land/Insurance.h"
#include "Land/Aviation.h"
#include "Land/Oil.h"
#include "Land/Technology.h"
#include "Land/Bank.h"
#include "Land/Life.h"
#include "Land/Chance.h"
#include "Incident/Incident.h"
#include "AudioEngine.h"
#include <algorithm>

USING_NS_CC;

MapScene *MapScene::createScene()
{
	return MapScene::create();
}

bool MapScene::init()
{

	if (!Scene::init())
	{
		return false;
	}

	MenuItemFont::setFontName("fonts/STHUPO.ttf");
	MenuItemFont::setFontSize(45);

	if (!MapScene::mapInit())
	{
		return false;
	}
	if (!MapScene::panelInit())
	{
		return false;
	}
	if (!MapScene::informationInit())
	{
		return false;
	}
	if (!MapScene::landInit())
	{
		return false;
	}
	if (!MapScene::touchInit())
	{
		return false;
	}
	if (!MapScene::rollMap())
	{
		return false;
	}
	if (!MapScene::miniMapInit())
	{
		return false;
	}
	perspectiveJump(45.f * 32.f, 18.f * 32.f);

	/*//测试
	log("test %f %f", pos_[2].x, pos_[2].y);
	auto sp = Sprite::create("hotel2.png");
	map_->addChild(sp, 5);
	sp->setAnchorPoint(Vec2(0.5, 0));
	sp->setPosition(pos_[11].x, pos_[11].y+32);
	*/
	return true;
}

//地图初始化
bool MapScene::mapInit()
{
	auto visible_size = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	map_ = TMXTiledMap::create("map.tmx");
	if (!map_)
		return false;
	map_->setAnchorPoint(Vec2::ZERO);
	map_->getLayer("grass")->setLocalZOrder(-10);
	map_->getLayer("lake")->setLocalZOrder(-8);
	map_->getLayer("road")->setLocalZOrder(-6);
	map_->getLayer("land")->setLocalZOrder(-4);
	map_->getLayer("house")->setVisible(false);

	//测试
	//auto listener = EventListenerTouchOneByOne::create();
	//listener->onTouchBegan = [=](Touch *touch, Event *event) {
	//	auto touch_pos = touch->getLocation() - event->getCurrentTarget()->getPosition();
	//	auto tile_size = map_->getTileSize();
	//	auto map_size = map_->getMapSize();
	//	//log("visible_size:%.2f,%.2f", visible_size.width, visible_size.height);
	//	//log("tile_size:%.2f,%.2f", tile_size.width, tile_size.height);
	//	//log("map_size:%.2f,%.2f", map_size.width, map_size.height);
	//	auto x = static_cast<float>(static_cast<int>(touch_pos.x / tile_size.width));
	//	auto y = static_cast<float>(static_cast<int>(touch_pos.y / tile_size.height));
	//	//log("touched tile position to tiles in tilemap direction is x:%f,y:%f", x, y);
	//	return true;
	//};
	//map_->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, map_);

	this->addChild(map_, -20, "map");
	return true;
}

//添加地图滚动的监听和回调方法
bool MapScene::rollMap()
{
	auto visible_size = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto map_size = map_->getMapSize();
	auto tile_size = map_->getTileSize();
	auto listener = EventListenerTouchOneByOne::create();
	static Vec2 start_pos(0, 0);
	listener->onTouchBegan = [=](Touch *touch, Event *event) {
		if (touch->getLocation().x > 24 * tile_size.width)
			return false;
		start_pos = touch->getLocation() - event->getCurrentTarget()->getPosition();
		return true;
	};
	listener->onTouchMoved = [=](Touch *touch, Event *event) {
		auto delta = touch->getLocation() - start_pos;
		auto x = std::min(0.0f, delta.x);
		x = std::max(x, visible_size.width - 8 * tile_size.width - map_size.width * tile_size.width);
		auto y = std::min(0.0f, delta.y);
		y = std::max(y, visible_size.height - map_size.height * tile_size.height);
		event->getCurrentTarget()->setPosition(x, y);

		auto camera = mini_map_->getChildByName("camera");
		camera->setPosition(-x, -y);
		//log("your present touching position to the screen in GL direction is x:%.2f y:%.2f", finish_pos.x, finish_pos.y);
		//log("map position to the parent in GL direction change to x:%.2f y:%.2f", delta.x, delta.y);
	};
	map_->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, map_);
	return true;
}

//初始化地图旁边的操作面板
bool MapScene::panelInit()
{
	auto visible_size = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	auto map_size = map_->getMapSize();
	auto tile_size = map_->getTileSize();

	//面板初始化
	this->panel_ = Layer::create();
	panel_->setAnchorPoint(Vec2::ZERO);
	panel_->setPosition(Vec2::ZERO);
	this->addChild(panel_, 10, "panel");
	auto sprite = Sprite::create("panel.png");
	sprite->setAnchorPoint(Vec2::ZERO);
	sprite->setPosition(Vec2::ZERO);
	panel_->addChild(sprite, 10);

	//向这个成员变量中添加按钮
	menu_item_.insert("close", MenuItemFont::create(ZH(" 退出 ")));
	menu_item_.insert("setting", MenuItemFont::create(ZH(" 音乐 ")));
	menu_item_.insert("item", MenuItemFont::create(ZH(" 道具 ")));
	menu_item_.insert("stock", MenuItemFont::create(ZH(" 股市 ")));

	//从这个变量获取按钮项
	auto close_item = menu_item_.at("close");
	auto setting_item = menu_item_.at("setting");
	auto item_item = menu_item_.at("item");
	auto stock_item = menu_item_.at("stock");

	//按钮排列
	auto menu_up = Menu::create();
	auto menu_down = Menu::create();
	menu_up->addChild(item_item, 30);
	menu_down->addChild(stock_item, 30);
	menu_down->addChild(setting_item, 30);
	menu_up->addChild(close_item, 30);

	menu_up->alignItemsHorizontally();
	menu_down->alignItemsHorizontally();

	menu_up->setAnchorPoint(Vec2(0.5f, 0.5f));
	menu_up->setPosition(Vec2(28 * tile_size.width, visible_size.height - 1.5f * tile_size.height));
	menu_down->setAnchorPoint(Vec2(0.5f, 0.5f));
	menu_down->setPosition(Vec2(28 * tile_size.width, visible_size.height - 3.f * tile_size.height));
	panel_->addChild(menu_up, 20);
	panel_->addChild(menu_down, 20);

	//更改按钮回调的执行
	close_item->setCallback([=](Ref *render) {
		auto sound_effect = AudioEngine::play2d("bottom_down.mp3", false);
		AudioEngine::stopAll();
		auto scene = StartScene::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(0.5f, scene, Color3B(0, 255, 255)));
	});

	/*setting_item->setCallback([=](Ref *render) {
		auto sound_effect = AudioEngine::play2d("bottom_down.mp3", false);
		auto scene = SettingScene::createScene();
		Director::getInstance()->pushScene(scene);
	});*/
	return true;
}

bool MapScene::informationInit()
{
	auto information_layer = Layer::create();
	panel_->addChild(information_layer, 30, "information_layer");
	return true;
}

void MapScene::setInfoOnDisplay(Character *player)
{
	info_on_display_ = player->getTag();
}

void MapScene::updateInformation(Character *player)
{
	int tag = player->getTag();
	if (info_on_display_ != tag)
		return;
	auto information_layer = panel_->getChildByName("information_layer");
	information_layer->removeAllChildrenWithCleanup(true);

	Size visible_size = Director::getInstance()->getVisibleSize();

	Value val_cash, val_cash_pre, val_cash_label;
	val_cash_pre = Value(ZH("现金: "));
	int money = player->getMoney();
	val_cash = Value(money);
	val_cash_label = val_cash_pre.asString() + val_cash.asString().c_str();
	auto label_cash = Label::createWithSystemFont(val_cash_label.asString().c_str(), "fonts/arial.ttf", 30);
	label_cash->setTextColor(Color4B::BLACK);
	label_cash->setAnchorPoint(Vec2(0, 0.5));
	label_cash->setPosition(Vec2(810, visible_size.height - 370));
	information_layer->addChild(label_cash, 25); //现金标签

	Value val_deposit, val_deposit_pre, val_deposit_label;
	val_deposit_pre = Value(ZH("存款: "));
	int deposit = player->getDeposit();
	val_deposit = Value(deposit);
	val_deposit_label = val_deposit_pre.asString() + val_deposit.asString().c_str();
	auto label_deposit = Label::createWithSystemFont(val_deposit_label.asString().c_str(), "fonts/arial.ttf", 30);
	label_deposit->setTextColor(Color4B::BLACK);
	label_deposit->setAnchorPoint(Vec2(0, 0.5));
	label_deposit->setPosition(Vec2(810, visible_size.height - 370 - 16 * 3));
	information_layer->addChild(label_deposit, 25);

	Value val_loan, val_loan_pre, val_loan_label;
	val_loan_pre = Value(ZH("贷款: "));
	int loan = player->getLoan();
	val_loan = Value(loan);
	val_loan_label = val_loan_pre.asString() + val_loan.asString().c_str();
	auto label_loan = Label::createWithSystemFont(val_loan_label.asString().c_str(), "fonts/arial.ttf", 30);
	label_loan->setTextColor(Color4B::BLACK);
	label_loan->setAnchorPoint(Vec2(0, 0.5));
	label_loan->setPosition(Vec2(810, visible_size.height - 370 - 16 * 6));
	information_layer->addChild(label_loan, 25);

	auto sprite_head = Sprite::create(player->getPlayerName() + string("_avatar.png"));
	sprite_head->setPosition(Vec2(847, visible_size.height - 290));
	information_layer->addChild(sprite_head, 25);
	auto sprite_color = Sprite::create(StringUtils::format("character_avatar%d.png", tag));
	sprite_color->setPosition(Vec2(940, visible_size.height - 290));
	information_layer->addChild(sprite_color, 25);

	auto label_avatar = Label::createWithSystemFont(player->getPlayerName().c_str(), "fonts/arial.ttf", 25);
	label_avatar->setTextColor(Color4B::BLACK);
	label_avatar->setAnchorPoint(Vec2(0.5, 0.5));
	label_avatar->setPosition(Vec2(940, visible_size.height - 290));
	information_layer->addChild(label_avatar, 26);

	Value val_year(ZH("年")), val_mon(ZH("月")), val_day(ZH("日")), val_date;
	if (day_ >= 31)
	{
		day_ = 1;
		month_++;
	}
	if (month_ >= 12)
	{
		month_ = 1;
		year_++;
	}
	val_date = Value(year_).asString().c_str() + val_year.asString() + Value(month_).asString().c_str() +
			   val_mon.asString() + Value(day_).asString().c_str() + val_day.asString();
	auto label_date = Label::createWithSystemFont(val_date.asString().c_str(), "fonts/arial.ttf", 30);
	label_date->setTextColor(Color4B::BLACK);
	label_date->setAnchorPoint(Vec2(0, 0.5));
	label_date->setPosition(Vec2(800, visible_size.height - 200));
	information_layer->addChild(label_date, 25);
}
void MapScene::updateDay()
{
	day_++;
}

bool MapScene::miniMapInit()
{
	auto visible_size = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();
	mini_map_ = TMXTiledMap::create("camera.tmx");
	if (!mini_map_)
		return false;
	auto map_size = mini_map_->getMapSize();
	auto tile_size = mini_map_->getTileSize();
	mini_map_->setAnchorPoint(Vec2::ZERO);
	mini_map_->setScale(0.08f);
	mini_map_->getLayer("grass")->setLocalZOrder(12);
	mini_map_->getLayer("lake")->setLocalZOrder(13);
	mini_map_->getLayer("road")->setLocalZOrder(14);
	mini_map_->getLayer("land")->setVisible(false);
	mini_map_->setPosition(visible_size.width - 8.f * tile_size.width, 0.f);
	panel_->addChild(mini_map_, 11, "mini_map_");

	auto camera = Sprite::create("white.png");
	camera->setAnchorPoint(Vec2::ZERO);
	mini_map_->addChild(camera, 15, "camera");

	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = [=](Touch *touch, Event *event) {
		auto touch_pos = touch->getLocation();
		auto temp_pos = event->getCurrentTarget()->getPosition();
		if (touch_pos.x > temp_pos.x && touch_pos.y < 8 * tile_size.height)
		{
			perspectiveJump((touch_pos - temp_pos) * map_size.width / 8);
		}
		return true;
	};
	listener->onTouchMoved = [=](Touch *touch, Event *event) {
		auto touch_pos = touch->getLocation();
		auto temp_pos = event->getCurrentTarget()->getPosition();
		if (touch_pos.x > temp_pos.x && touch_pos.y < 8 * tile_size.height)
		{
			listener->setSwallowTouches(true);
			perspectiveJump((touch_pos - temp_pos) * map_size.width / 8);
		}
		return true;
	};
	listener->onTouchEnded = [=](Touch *touch, Event *event) {
		listener->setSwallowTouches(false);
	};

	mini_map_->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, mini_map_);
	return true;
}

//将视角跳转到以地图上某个像素坐标为中心的相应位置
bool MapScene::perspectiveJump(Vec2 p)
{
	return perspectiveJump(p.x, p.y);
}

bool MapScene::perspectiveJump(float x, float y)
{
	auto visible_size = Director::getInstance()->getVisibleSize();
	auto map_size = map_->getMapSize();
	auto tile_size = map_->getTileSize();

	y = y + 4 * tile_size.height;

	x = visible_size.width / 2.f - 4 * tile_size.width - x;
	y = visible_size.height / 2.f - y;

	x = std::min(0.0f, x);
	x = std::max(x, visible_size.width - 8 * tile_size.width - map_size.width * tile_size.width);
	y = std::min(0.0f, y);
	y = std::max(y, visible_size.height - map_size.height * tile_size.height);

	auto camera = mini_map_->getChildByName("camera");

	camera->setPosition(-x, -y);
	map_->setPosition(x, y);
	return true;
}

bool MapScene::touchInit()
{

	auto len = static_cast<int>(pos_.size());
	auto tile_size = map_->getTileSize();
	auto map_size = map_->getMapSize();
	lands_.resize(pos_.size(), nullptr);
	gods_.resize(pos_.size(), nullptr);
	for (int c = 0; c < len; c++)
	{
		auto x = static_cast<int>(pos_.at(c).x / tile_size.width);
		auto y = static_cast<int>((map_size.height * tile_size.height - pos_.at(c).y) / tile_size.height);
		idx_.insert(std::make_pair((x * 100 + y), c));
	}
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = [=](Touch *touch, Event *event) {
		auto tile_size = map_->getTileSize();
		auto index = touchIndex(touch->getLocation());
		if (index == -1)
			return false;
		auto &land = lands_.at(index);
		if (!land)
		{
			switch (type_.at(index))
			{
			case land_chance:
				land = Chance::create(index);
				break;
			case land_life:
				land = Life::create(index);
				break;
			case land_hotel:
				land = Hotel::create(index);
				break;
			case land_business:
				land = Business::create(index);
				break;
			case land_insurance:
				land = Insurance::create(index);
				break;
			case land_oil:
				land = Oil::create(index);
				break;
			case land_technology:
				land = Technology::create(index);
				break;
			case land_aviation:
				land = Aviation::create(index);
				break;
			case land_hospital:
				land = Hospital::create(index);
				break;
			case land_jail:
				land = Jail::create(index);
				break;
			case land_bank:
				land = Bank::create(index);
				break;
			case land_lottery:
				break;
			default:
				break;
			}
		}
		if (!land)
			return false;
		auto label = Label::createWithSystemFont(ZH(land->getName()), "fonts/arial.ttf", 40);
		auto position = pos(index);
		label->setPosition(position.x, position.y - tile_size.height * 2);
		map_->addChild(label);
		auto fun = CallFunc::create([=]() {
			label->removeFromParentAndCleanup(true);
		});
		auto seq = Sequence::create(DelayTime::create(0.3f), fun, nullptr);
		this->runAction(seq);
		return false;
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
	return true;
}

//添加地图上道路中心位置相对于地图右下角锚点的GL方向以像素为单位的坐标位置
bool MapScene::landInit()
{
	auto tile_size = map_->getTileSize();
	auto map_size = map_->getMapSize();
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(84)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(56), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(90)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(9);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(84)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(78)));
	type_.push_back(16);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(78)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(78)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(72)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(72)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(72)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(11);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(13);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(66)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(56), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(66)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(66)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(11);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(66)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(62)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(62)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(60)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(56)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(44)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(15);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(56), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(44)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(44)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(11);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(28)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(24)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(22)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(88), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(86), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(84), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(82), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(80), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(78), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(76), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(74), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(72), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(70), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(68), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(66), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(64), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(62), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(56), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(14);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(16)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(18)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(20)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(22)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(24)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(24)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(16);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(26)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(28)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(30)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(32)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(34)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(36)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(38)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(40)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(42)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(44)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(46)));
	type_.push_back(16);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(48)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(50)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(11);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(52)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(54)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(56)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(58)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(60)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(62)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(64)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(66)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(12);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(11);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(68)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(70)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(72)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(74)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(78)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(15);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(84)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(10), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(12), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(14), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(16), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(88)));
	type_.push_back(1);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(86)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(84)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(78)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(18), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(20), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(22), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(24), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(4);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(26), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(28), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(30), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(32), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(3);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(34), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(36), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(38), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(40), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(42), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(44), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(46), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(48), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(56), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(10);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(76)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(78)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(80)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(60), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(2);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(58), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(56), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(54), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(52), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	pos_.push_back(Vec2(tile_size.width * static_cast<float>(50), map_size.height * tile_size.height - tile_size.height * static_cast<float>(82)));
	type_.push_back(0);
	return true;
}