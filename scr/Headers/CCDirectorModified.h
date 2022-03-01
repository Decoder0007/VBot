#pragma once
#include "cocos2d.h"
using namespace cocos2d;
class CCDirectorModified : public CCDirector {
public:
	CCScene* getPreviousScene()
	{
		return (CCScene*)m_pobScenesStack->objectAtIndex(m_pobScenesStack->count() - 2);
	}
};