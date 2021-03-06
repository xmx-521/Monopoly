#include "StockScene.h"
#include "Common/CommonMethod.h"
#include "Character/Character.h"
#include "Common/CommonConstant.h"
#include <ctime>
#include <vector>
#include "Incident/Finance.h"
#include "MapScene.h"
#include "AudioEngine.h"
using namespace std;
Stock* Stock::create(int stock_code, std::string stock_name, int now_price, int make_deal_price, float percent, int store_number) {
	auto pRet = new(std::nothrow) Stock(stock_code,stock_name,now_price,make_deal_price,percent,store_number);
	if (pRet && pRet->init())
	{
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


Stock::Stock(int stock_code, std::string stock_name, int now_price, int make_deal_price, float percent, int store_number){
	this->stock_code_ = stock_code;
	this->stock_name_ = stock_name;
	this->now_price_ = now_price;
	this->percent_ = percent_;
	//this->store_number_ = store_number__;
	this->make_deal_price_ = make_deal_price;
	int num[8] = { 0 };
	vector<int> vec(num, num + 8);
	this->store_number_ = vec;
	condition_ = normal;
	duration_time_ = 0;
}


void StockScene::open(Ref* ref)
{
	is_open_ = true;
	auto sound_effect = AudioEngine::play2d("bottom_down.mp3", false);
	this->setPosition(Vec2(0, 0));
	this->map_scene_->setMenuCallback("stock", [=](Ref* ref) {close(ref); });
}

void StockScene::close(Ref* ref)
{
	is_open_ = false;
	auto sound_effect = AudioEngine::play2d("bottom_down.mp3", false);
	this->setPosition(Vec2(6000, 6000));
	this->map_scene_->setMenuCallback("stock", [=](Ref* ref) {open(ref); });
}


StockScene* StockScene::createScene(MapScene *map_scene)
{
	auto visible_size = Director::getInstance()->getVisibleSize();
	auto stock_layer = StockScene::create();
	stock_layer->map_scene_ = map_scene;
	stock_layer->setPosition(Vec2(6000, 6000));
	stock_layer->map_scene_->addChild(stock_layer, 23,"stock_scene");

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = [=](Touch* touch, Event* event) {
		if (stock_layer->is_open_)
		{
			if (touch->getLocation().x > visible_size.height)
				listener->setSwallowTouches(false);
			else
				listener->setSwallowTouches(true);
		}
		else
		{
			listener->setSwallowTouches(false);
		}
		return true;
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, stock_layer);

	stock_layer->map_scene_->setMenuCallback("stock", [=](Ref* ref) {stock_layer->open(ref); });
	return stock_layer;
}



bool StockScene::init() {
	if (!Layer::init()) {
		return false;
	}
	condition_ = normal_market;
	duration_time_ = 0;
	auto stock_sprite = Sprite::create("pic.png");
	stock_sprite->setAnchorPoint(Vec2(0, 0));
	this->addChild(stock_sprite,24,"sprite");
	stockInfInit(); //初始化股票信息
	stockUpdate();
	initLabel();  //考虑传进来stock_vec_
	
	return true;
}

void StockScene::initLabel() {
	

	Size VisibleSize = Director::getInstance()->getVisibleSize();	//获得屏幕大小
	initFirstLabel();
	for (int i = 0; i < 8; i++) {
		Value val_code, val_name, val_price, val_deal,val_per,val_label,val_store,val_blank, val_percent;
		
		val_code = Value(stock_vec_.at(i)->stock_code_);
		val_name = Value(stock_vec_.at(i)->stock_name_);
		val_price = Value(stock_vec_.at(i)->now_price_);
		val_deal = Value(stock_vec_.at(i)->make_deal_price_);
		val_per = Value(static_cast<int>((stock_vec_.at(i)->percent_)*100));
		val_store = Value(stock_vec_.at(i)->store_number_[0]);
		val_percent = "%";
		val_blank = Value("  ");
		val_per = val_per.asString().c_str() + val_percent.asString();
		val_label = val_code.asString().c_str() +val_blank.asString()+val_name.asString() + val_blank.asString() +
			val_blank.asString()+val_price.asString().c_str() + val_blank.asString() + val_blank.asString() + val_per.asString().c_str() ;
			
		auto label = Label::createWithSystemFont(val_label.asString().c_str(), "fonts/arial.ttf", 40);//创建一个标签
		if (val_per.asFloat() <= 0)
			label->setTextColor(Color4B::GREEN);
		else
			label->setTextColor(Color4B::RED);
		label->setAnchorPoint(Vec2(0, 0.5));
		label->setPosition(Vec2(5, VisibleSize.height - 197 - 64 * (1 + i)));
		this->addChild(label, 25);
		
		auto label_store = Label::createWithSystemFont(val_store.asString().c_str(), "fonts/arial.ttf", 40);//创建一个标签
		if (val_per.asFloat() <= 0)
			label_store->setTextColor(Color4B::GREEN);
		else
			label_store->setTextColor(Color4B::RED);
		label_store->setAnchorPoint(Vec2(0, 0.5));
		label_store->setPosition(Vec2(500, VisibleSize.height - 197 - 64 * (1 + i)));
		this->addChild(label_store, 25);
		
		auto labelBuy = Label::createWithSystemFont(ZH("买入"), "fonts/arial.ttf", 38);
		auto menuItem_buy= MenuItemLabel::create(labelBuy);
		menuItem_buy->setCallback([=](Ref* render) {
			
			});
		menuItem_buy->setAnchorPoint(Vec2(0, 0.5));
		menuItem_buy->setPosition(Vec2(590, VisibleSize.height - 197 - 64 * (1 + i)));
		//this->addChild(menuItem_buy, 25);

		auto label_sell = Label::createWithSystemFont(ZH("卖出"), "fonts/arial.ttf", 38);
		auto menuItem_sell = MenuItemLabel::create(label_sell);
		menuItem_sell->setCallback([=](Ref* render) {


			}
		);
		menuItem_sell->setAnchorPoint(Vec2(0, 0.5));
		menuItem_sell->setPosition(Vec2(675, VisibleSize.height - 197 - 64 * (1 + i)));
		//this->addChild(menuItem_sell, 25);
		auto menu_sell_buy = Menu::create();
		menu_sell_buy->addChild(menuItem_sell);
		menu_sell_buy->addChild(menuItem_buy);
		this->addChild(menu_sell_buy, 40);
		
			/*
				auto label_code = Label::createWithSystemFont(val_code.asString().c_str(), "fonts/arial.ttf", 44);
				if (val_per.asFloat() <= 0)
					label_code->setTextColor(Color4B::GREEN);
				else 
					label_code->setTextColor(Color4B::RED);
				label_code->setAnchorPoint(Vec2(0, 0.5));
				label_code->setPosition(Vec2(5, Visible_size.height - 197 - 64 * (1+i)));
				this->addChild(label_code, 25);
				

			
				auto label_name = Label::createWithSystemFont(val_name.asString().c_str(), "fonts/arial.ttf", 40);
				if (val_per.asFloat() <= 0)
					label_name->setTextColor(Color4B::GREEN);
				else
					label_name->setTextColor(Color4B::RED);
				label_name->setAnchorPoint(Vec2(0, 0.5));
				label_name->setPosition(Vec2(120, Visible_size.height - 197 - 64 * (1+i)));
				this->addChild(label_name, 25);
				


			
				auto label_price = Label::createWithBMFont(val_price.asString().c_str(), "fonts/arial.ttf", 46);
				if (val_per.asFloat() <= 0)
					label_price->setTextColor(Color4B::GREEN);
				else
					label_price->setTextColor(Color4B::RED);
				label_price->setAnchorPoint(Vec2(0, 0.5));
				label_price->setPosition(Vec2(310, Visible_size.height - 197 - 64 * (1+i)));
				this->addChild(label_price, 25);
				

			
				auto label_per = Label::createWithSystemFont(StringUtils::format("%.0f%%",val_per.asFloat()*100),"fonts/arial.ttf", 38);
				if (val_per.asFloat() <= 0)
					label_per->setTextColor(Color4B::GREEN);
				else
					label_per->setTextColor(Color4B::RED);
				label_per->setAnchorPoint(Vec2(0, 0.5));
				label_per->setPosition(Vec2(390, Visible_size.height - 197 - 64 * (1+i)));
				this->addChild(label_per, 25);
				
				auto label_store = Label::createWithSystemFont(val_store.asString().c_str(), "fonts/arial.ttf", 46);
				if (val_per.asFloat() <= 0)
					label_store->setTextColor(Color4B::GREEN);
				else
					label_store->setTextColor(Color4B::RED);
				label_store->setAnchorPoint(Vec2(0, 0.5));
				label_store->setPosition(Vec2(500, Visible_size.height - 197 - 64 * (1+i)));
				this->addChild(label_store, 25);
			
			*/

		


	}




	
}
void StockScene::stockInfInit() {
	this->stock_vec_.pushBack(Stock::create(8010, ZH("天地银行"), 50, 0, static_cast < float>(0.0), 0));       //股票代码 股票名称 当前价格 成交价格 涨幅 持仓数 
	this->stock_vec_.pushBack(Stock::create(8011, ZH("宇宙房产"), 60, 0, static_cast < float>(0.01), 0));
	this->stock_vec_.pushBack(Stock::create(8012, ZH("玄学制药"), 37, 0, static_cast<float>(-0.09), 0));
	this->stock_vec_.pushBack(Stock::create(8015, ZH("缪斯学院"), 70, 0, static_cast < float>(0.04), 0));
	this->stock_vec_.pushBack(Stock::create(8016, ZH("保险公司"), 50, 0, static_cast < float>(0.01), 0));
	this->stock_vec_.pushBack(Stock::create(8017, ZH("石油公司"), 45, 0, static_cast < float>(0.08), 0));
	this->stock_vec_.pushBack(Stock::create(8018, ZH("科技公司"), 80, 0, static_cast < float>(0.03), 0));
	this->stock_vec_.pushBack(Stock::create(8019, ZH("航空公司"), 30, 0, static_cast < float>(0.07), 0));
}


void StockScene::initFirstLabel() {
	Size VisibleSize = Director::getInstance()->getVisibleSize();	//获得屏幕大小
	auto label_code = Label::createWithSystemFont(ZH("代码"), "fonts/arial.ttf", 40);
	
	label_code->setTextColor(Color4B::BLACK);
	label_code->setAnchorPoint(Vec2(0, 0.5));
	label_code->setPosition(Vec2(5, VisibleSize.height - 197 ));
	this->addChild(label_code, 25);



	auto label_name = Label::createWithSystemFont(ZH("股票名称"), "fonts/arial.ttf", 40);
	label_name->setTextColor(Color4B::BLACK);
	label_name->setAnchorPoint(Vec2(0, 0.5));
	label_name->setPosition(Vec2(120, VisibleSize.height - 197 ));
	this->addChild(label_name, 25);




	auto label_price = Label::createWithSystemFont(ZH("价格"), "fonts/arial.ttf", 40);
	label_price->setTextColor(Color4B::BLACK);
	label_price->setAnchorPoint(Vec2(0, 0.5));
	label_price->setPosition(Vec2(305, VisibleSize.height - 197 ));
	this->addChild(label_price, 25);



	auto label_per = Label::createWithSystemFont(ZH("涨幅"), "fonts/arial.ttf", 40);
	label_per->setTextColor(Color4B::BLACK);
	label_per->setAnchorPoint(Vec2(0, 0.5));
	label_per->setPosition(Vec2(390, VisibleSize.height - 197 ));
	this->addChild(label_per, 25);

	auto label_store = Label::createWithSystemFont(ZH("数量"), "fonts/arial.ttf", 40);
	label_store->setTextColor(Color4B::BLACK);
	label_store->setAnchorPoint(Vec2(0, 0.5));
	label_store->setPosition(Vec2(495, VisibleSize.height - 197 ));
	this->addChild(label_store, 25);
}

void StockScene::stockUpdate() {
	srand(static_cast<unsigned int>(time(nullptr)));
	for (int i = 0; i < 8; i++) {
		float per_ = rand() / (10*RAND_MAX+static_cast<float>(0.001));
		auto cur_stock = stock_vec_.at(i);
		switch (cur_stock->getCondition())
		{
		case normal_market:
			if (rand() % 2) per_ *= (-1.0);
			break;
		case up_market:
			cur_stock->setDurationTime(cur_stock->getDurationTime() - 1);
			break;
		case down_market:
			per_ *= (-1.0);
			cur_stock->setDurationTime(cur_stock->getDurationTime() - 1);
			break;
		}
		if (duration_time_ == 0)
		{
			cur_stock->setCondition(normal_market);
		}
		stock_vec_.at(i)->percent_ = per_;
		stock_vec_.at(i)->now_price_ = static_cast<int>(1.0f+stock_vec_.at(i)->now_price_*(1.0+per_));

	}
	if (duration_time_ > 0)
	{
		duration_time_--;
	}
	if (duration_time_ == 0)
	{
		switch(condition_)
		{
		case up_market:
			condition_ = normal_market;
			PopUpMarketCalm();
			break;
		case down_market:
			condition_ = normal_market;
			PopUpMarketRecover();
			break;
		}
	}
}

void StockScene::remakeLabel(Character* player) {
	auto stock_sprite = this->getChildByName("sprite");
	stock_sprite->retain();
	this->removeAllChildrenWithCleanup(true);
	this->addChild(stock_sprite, 24, "sprite");
	stock_sprite->release();
	initFirstLabel();
	Size Visible_size = Director::getInstance()->getVisibleSize();	//获得屏幕大小
	
	for (int i = 0; i < 8; i++) {
		Value val_code, val_name, val_price, val_deal, val_per, valLabel_, val_store, val_blank, val_percent;

		val_code = Value(stock_vec_.at(i)->stock_code_);
		val_name = Value(stock_vec_.at(i)->stock_name_);
		val_price = Value(stock_vec_.at(i)->now_price_);
		val_deal = Value(stock_vec_.at(i)->make_deal_price_);
		val_per = Value(static_cast<int>((stock_vec_.at(i)->percent_) * 100));
		val_store = Value(stock_vec_.at(i)->store_number_[player->getTag()]);
		val_percent = "%";
		val_blank = Value("  ");
		val_per = val_per.asString().c_str() + val_percent.asString();
		valLabel_ = val_code.asString().c_str() + val_blank.asString() + val_name.asString() + val_blank.asString() +
			val_blank.asString() + val_price.asString().c_str() + val_blank.asString() + val_blank.asString() + val_per.asString().c_str();

		auto label_ = Label::createWithSystemFont(valLabel_.asString().c_str(), "fonts/arial.ttf", 40);//创建一个标签
		if (val_per.asFloat() <= 0)
			label_->setTextColor(Color4B::GREEN);
		else
			label_->setTextColor(Color4B::RED);
		label_->setAnchorPoint(Vec2(0, 0.5));
		label_->setPosition(Vec2(5, Visible_size.height - 197 - 64 * (1 + i)));
		this->addChild(label_, 25);

		auto label_store = Label::createWithSystemFont(val_store.asString().c_str(), "fonts/arial.ttf", 40);//创建一个标签
		if (val_per.asFloat() <= 0)
			label_store->setTextColor(Color4B::GREEN);
		else
			label_store->setTextColor(Color4B::RED);
		label_store->setAnchorPoint(Vec2(0, 0.5));
		label_store->setPosition(Vec2(500, Visible_size.height - 197 - 64 * (1 + i)));
		this->addChild(label_store, 25);

		auto label_buy = Label::createWithSystemFont(ZH("买入"), "fonts/arial.ttf", 36);
		auto menuItem_buy = MenuItemLabel::create(label_buy);
		menuItem_buy->setCallback([=](Ref* render) {
			auto sound_effect = AudioEngine::play2d("bottom_up.mp3", false);
			int money_ = player->getMoney();
			if (money_ >= val_price.asInt() * buy_number_min) {
				player->setMoney(money_ - val_price.asInt() * buy_number_min);
				stock_vec_.at(i)->store_number_[player->getTag()] += buy_number_min;
				remakeLabel(player);
				map_scene_->updateInformation(player);
			}
			});
			
		//menuItem_buy->setAnchorPoint(Vec2(0, 0.5));
		//menuItem_buy->setPosition(Vec2(590, Visible_size.height - 197 - 64 * (1 + i)));
		//this->addChild(menuItem_buy, 25);


		auto label_sell = Label::createWithSystemFont(ZH("卖出"), "fonts/arial.ttf", 36);
		//label_sell->setPosition(Vec2(675, Visible_size.height - 197 - 64 * (1 + i)));
		auto menuItem_sell = MenuItemLabel::create(label_sell);

		menuItem_sell->setCallback([=](Ref* render) {
			auto sound_effect = AudioEngine::play2d("bottom_up.mp3", false);
			if (stock_vec_.at(i)->store_number_[player->getTag()] >= buy_number_min) {
				int money_ = player->getMoney();
				stock_vec_.at(i)->store_number_[player->getTag()] -= buy_number_min;
				int stock_money = buy_number_min * val_price.asInt();
				player->setMoney(money_ + stock_money);
				remakeLabel(player);
				map_scene_->updateInformation(player);
			}

			}
		);
		//menuItem_sell->setAnchorPoint(Vec2(0, 0.5));
		//menuItem_sell->setPosition(Vec2(675, Visible_size.height - 197 - 64 * (1 + i)));
		//this->addChild(menuItem_sell, 25);
		auto menu_sell = Menu::create();
		auto menu_buy = Menu::create();
		menu_sell->addChild(menuItem_sell);
		menu_buy->addChild(menuItem_buy);
		menu_sell->setPosition(Vec2(715, Visible_size.height - 197 - 64 * (1 + i)));
		menu_buy->setPosition(Vec2(630, Visible_size.height - 197 - 64 * (1 + i)));
		this->addChild(menu_sell, 26);
		this->addChild(menu_buy, 26);
	}
}
