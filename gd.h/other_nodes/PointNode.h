#ifndef __POINTNODE_H__
#define __POINTNODE_H__

#include <gd.h>

namespace gd {
    class PointNode : public cocos2d::CCObject {
    public:
        cocos2d::CCPoint m_point;

        static PointNode* create(cocos2d::CCPoint point) {
            return reinterpret_cast<PointNode*(__stdcall*)(cocos2d::CCPoint)>(base + 0x14ec80)(point);
        }
    };
}

#endif