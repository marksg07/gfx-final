#include <iostream>
#include "BrushManager.h"
#include "brush/ConstantBrush.h"
#include "brush/LinearBrush.h"
#include "brush/QuadraticBrush.h"
#include "brush/SmudgeBrush.h"
#include "brush/SnakeBrush.h"
#include "brush/FoodBrush.h"
#include "ui/Settings.h"


BrushManager::BrushManager()
    : m_brush(nullptr), m_curType(BRUSH_CONSTANT)
{
}

/**
 * @brief update
 */
std::shared_ptr<Brush> BrushManager::update()
{

    int type = settings.brushType;


    switch(type)
    {
    case BRUSH_CONSTANT:
        m_brush = std::make_shared<ConstantBrush>(settings.brushColor, settings.brushRadius);
        break;
    case BRUSH_LINEAR:
        m_brush = std::make_shared<LinearBrush>(settings.brushColor, settings.brushRadius);
        break;
    case BRUSH_QUADRATIC:
        m_brush = std::make_shared<QuadraticBrush>(settings.brushColor, settings.brushRadius);
        break;
    case BRUSH_SMUDGE:
        m_brush = std::make_shared<SmudgeBrush>(settings.brushColor, settings.brushRadius);
        break;
    case BRUSH_SPECIAL_1:
        m_brush = std::make_shared<SnakeBrush>(settings.brushColor, settings.brushRadius);
        break;
    case BRUSH_SPECIAL_2:
        m_brush = std::make_shared<FoodBrush>(settings.brushColor, settings.brushRadius);
        break;
    default:
        std::cerr << "No valid brush type selected! Defaulting to constant" << std::endl;
        m_brush = std::make_shared<ConstantBrush>(settings.brushColor, settings.brushRadius);
    }

    m_curType = type;

    return m_brush;
}

/**
 * @brief getBrush
 * @return
 */
std::shared_ptr<Brush> BrushManager::getBrush()
{
    return m_brush;
}


int BrushManager::getCurType()
{
    return m_curType;
}
