#ifndef BRUSHMANAGER_H
#define BRUSHMANAGER_H

#include <memory>
#include "brush/Brush.h"

class BrushManager
{
public:
    BrushManager();

    std::shared_ptr<Brush> update();

    std::shared_ptr<Brush> getBrush();

    int getCurType();

private:

    std::shared_ptr<Brush> m_brush;

    int m_curType;
};

#endif // BRUSHMANAGER_H
