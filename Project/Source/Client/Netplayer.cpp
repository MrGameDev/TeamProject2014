#include "Netplayer.h"
#include "Game.hpp"

Netplayer::Netplayer() : TransformCollidable(Vector2(0.0, 0.0), Vector2(0.0f, -1.0f), Vector2(0.0f, 0.0f)){

}

Netplayer::Netplayer(std::string name, Vector2 position, Vector2 forward) : TransformCollidable(position, forward, Vector2(0.0f, 0.0f)){
	//create sprite
	sprite = new Sprite("Sprites\\new_fighter_enemy_sheet.png", position, Vector2(80.f, 80.f), 4);
	std::vector<int> idleAnimation;
	idleAnimation.push_back(0);
	sprite->addAnimation(idleAnimation);
	std::vector<int> thrustingAnimation;
	thrustingAnimation.push_back(1);
	thrustingAnimation.push_back(2);
	thrustingAnimation.push_back(3);
	sprite->addAnimation(thrustingAnimation);

	netRocket = new NetRocket(this, Vector2(-100.0, -100.0), Vector2(0.0f, 0.0f));

	boundingBox = new CircleBoundingBox(position, 25.0f);

	this->name = name;

	this->setTag("netPlayer");
	netRocket->setTag("netRocket");

	isDead = false;
	
	score = 0;

	updateElapsed = 0;

	g_pCollisionObserver->addListener(this);
}

Netplayer::~Netplayer(){
	delete sprite;
	delete netRocket;
}

void Netplayer::CollisionDetected(TransformCollidable *other, Vector2 penetration){
	
}

void Netplayer::update(){
	updateElapsed += g_pTimer->getDeltaTime();
	float currentFraction = updateElapsed / EXPECTEDTICKRATE;
	float alpha = clamp(0.0f, 1.0f, currentFraction);

	int windowWidth = g_pGame->getWindowWidth();
	int windowHeight = g_pGame->getWindowHeight();

	Vector2 pos;

	//We want to hide the fact that the player gets shoved outside the screen when killed, so when that happens, don't interpolate!
	if (frontHistoryCache.pos.getX() > 0 && frontHistoryCache.pos.getX() < windowWidth  &&
		frontHistoryCache.pos.getY() > 0 && frontHistoryCache.pos.getY() < windowHeight &&
		backHistoryCache.pos.getX() > 0  && backHistoryCache.pos.getX()  < windowWidth  &&
		backHistoryCache.pos.getY() > 0  && backHistoryCache.pos.getY()  < windowHeight   ){

		pos = lerp(backHistoryCache.pos, frontHistoryCache.pos, alpha);
	} else{
		pos = frontHistoryCache.pos;
	}

	this->setPosition(pos);
	sprite->setPosition(pos);

	Vector2 forward = lerp(backHistoryCache.forward, frontHistoryCache.forward, alpha);
	this->setForward(forward);

	if (/*forward.getLength() > 0.99f*/abs(backHistoryCache.pos.getX() - frontHistoryCache.pos.getX()) > 3.0f || abs(backHistoryCache.pos.getY() - frontHistoryCache.pos.getY()) > 3.0f)
	{
		if (sprite->getAnimationIndex() != 1)
			sprite->playAnimation(1, 0.05f, true);
	}
	else if (sprite->getAnimationIndex() != 0)
		sprite->playAnimation(0, 0.05f, true);

	float angle = lerp(backHistoryCache.angle, frontHistoryCache.angle, alpha);
	sprite->setAngle(angle);

	this->isDead = frontHistoryCache.isDead;

	Vector2 rocketPos;
	Vector2 rocketForward;

	if (frontHistoryCache.rocketPos.getX() > 0  && frontHistoryCache.rocketPos.getX() < g_pGame->getWindowWidth()  &&
		frontHistoryCache.rocketPos.getY() > 0  && frontHistoryCache.rocketPos.getY() < g_pGame->getWindowHeight() &&
		backHistoryCache.rocketPos.getX()  > 0  && backHistoryCache.rocketPos.getX()  < g_pGame->getWindowWidth()  &&
		backHistoryCache.rocketPos.getY()  > 0  && backHistoryCache.rocketPos.getY()  < g_pGame->getWindowWidth()    ){

		rocketPos = lerp(backHistoryCache.rocketPos, frontHistoryCache.rocketPos, alpha);
		rocketForward = lerp(backHistoryCache.rocketForward, frontHistoryCache.rocketForward, alpha);
	} else{
        rocketPos = frontHistoryCache.rocketPos;
		rocketForward = frontHistoryCache.rocketForward;
	}

	if (backHistoryCache.rocketPos.getX() < 0.0f && frontHistoryCache.rocketPos.getX() > 0.0f){
		//g_pLogfile->fLog("\n%f %f\n", backHistoryCache.rocketPos.getX(), backHistoryCache.rocketPos.getY());
		//g_pLogfile->fLog("%f %f\n", frontHistoryCache.rocketPos.getX(), frontHistoryCache.rocketPos.getY());
		netRocket->playAnimation(1, 0.5f, false);
	}
	
	netRocket->update(rocketPos, rocketForward);
}

void Netplayer::render()
{
	SDL_Color color = { 255, 127, 0 };
	g_pFontRenderer->drawSmallText(name, Vector2(position.getX() - g_pFontRenderer->getSmallTextDimensions(name).getX() / 2.f, position.getY() + 30.f), color);
}

void Netplayer::updateNetData(Vector2 pos, Vector2 forward, float angle, Vector2 rocketPos, Vector2 rocketForward, bool isDead){
	
	backHistoryCache = frontHistoryCache;

	frontHistoryCache.pos = pos;
	frontHistoryCache.forward = forward;
	frontHistoryCache.angle = angle;
	frontHistoryCache.rocketPos = rocketPos;
	frontHistoryCache.rocketForward = rocketForward;
	frontHistoryCache.isDead = isDead;
	
	updateElapsed = 0;
}

void Netplayer::rocketDestroyed(){
	netRocket->update(Vector2(-100.0, -100.0), Vector2(0.0, 0.0));
}

bool Netplayer::getIsDead(){
	return isDead;
}

Sprite *Netplayer::getSprite()
{
	return sprite;
}

int Netplayer::getScore()
{
	return score;
}

void Netplayer::setScore(int score)
{
	this->score = score;
}