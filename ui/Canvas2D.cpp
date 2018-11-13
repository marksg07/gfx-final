/**
 * @file Canvas2D.cpp
 *
 * CS123 2-dimensional canvas. Contains support code necessary for Brush, Filter, Intersect, and
 * Ray.
 *
 * YOU WILL NEED TO FILL THIS IN!
 *
 */

// For your convenience, a few headers are included for you.
#include <assert.h>
#include <iostream>
#include <math.h>
#include <memory>
#include <unistd.h>
#include "Canvas2D.h"
#include "Settings.h"
#include "RayScene.h"


#include <QPainter>
#include "brush/Brush.h"
#include "brush/ConstantBrush.h"
#include "brush/LinearBrush.h"
#include "brush/QuadraticBrush.h"
#include "brush/SmudgeBrush.h"
#include "brush/BrushManager.h"
#include "filter/FilterEdge.h"
#include "filter/FilterBlur.h"
#include "filter/FilterScale.h"
#include "filter/FilterRotate.h"
#include "filter/FilterSharpen.h"
#include "filter/FilterUnsharp.h"

Canvas2D::Canvas2D() :
    // @TODO: Initialize any pointers in this class here.
    m_rayScene(nullptr)
{
}

Canvas2D::~Canvas2D()
{
}

// This is called when the canvas size is changed. You can change the canvas size by calling
// resize(...). You probably won't need to fill this in, but you can if you want to.
void Canvas2D::notifySizeChanged(int w, int h) {

}

void Canvas2D::paintEvent(QPaintEvent *e) {
    // You probably won't need to fill this in, but you can if you want to override any painting
    // events for the 2D canvas. For now, we simply call the superclass.
    SupportCanvas2D::paintEvent(e);

}

// ********************************************************************************************
// ** BRUSH
// ********************************************************************************************


void Canvas2D::mouseDown(int x, int y) {
    // @TODO: [BRUSH] Mouse interaction for brush. You will probably want to create a separate
    //        class for each of your brushes. Remember that you can use the static Settings
    //        object to get the currently selected brush and its parameters.

    // You're going to need to leave the alpha value on the canvas itself at 255, but you will
    // need to use the actual alpha value to compute the new color of the pixel

//    bool fixAlphaBlending = settings.fixAlphaBlending; // for extra/half credit

    std::shared_ptr<Brush> brush = m_brushManager.update();
    if (brush == nullptr)
    {
        std::cerr << "Error!" << std::endl;
        return;
    }

    brush->brushDown(x, y, this);

}

void Canvas2D::mouseDragged(int x, int y) {
    // TODO: [BRUSH] Mouse interaction for Brush.

    std::shared_ptr<Brush> brush = m_brushManager.getBrush();
    if (brush == nullptr)
    {
        std::cerr << "Error!" << std::endl;
        return;
    }

    brush->brushDragged(x, y, this);
}

void Canvas2D::mouseUp(int x, int y) {
    // TODO: [BRUSH] Mouse interaction for Brush.

    std::shared_ptr<Brush> brush = m_brushManager.getBrush();
    if (brush == nullptr)
    {
        std::cerr << "Error!" << std::endl;
        return;
    }

    brush->brushUp(x, y, this);

}



// ********************************************************************************************
// ** FILTER
// ********************************************************************************************

void Canvas2D::filterImage() {
    // TODO: [FILTER] Filter the image. Some example code to get the filter type is provided below.

    QPoint start = marqueeStart();
    QPoint stop = marqueeStop();

    if (m_filter != nullptr)
    {
        m_filter->apply(this, start, stop);
    }
}

void Canvas2D::setScene(RayScene *scene) {
    m_rayScene.reset(scene);
}

void Canvas2D::renderImage(Camera *camera, int width, int height) {
    if (m_rayScene) {
        // @TODO: raytrace the scene based on settings
        //        YOU MUST FILL THIS IN FOR INTERSECT/RAY

        // If you want the interface to stay responsive, make sure to call
        // QCoreApplication::processEvents() periodically during the rendering.
        m_rayScene->render(this, width, height);

    }
}

void Canvas2D::cancelRender() {
    // TODO: cancel the raytracer (optional)
}



void Canvas2D::settingsChanged() {
    // TODO: Process changes to the application settings.

    int curType = m_brushManager.getCurType();
    int type = settings.brushType;

    if ((curType == BRUSH_SPECIAL_1 || curType == BRUSH_SPECIAL_2) && type != BRUSH_SPECIAL_1 && type != BRUSH_SPECIAL_2)
    {
        m_snakeManager->clear();
    }

    switch(settings.filterType) {
        case FILTER_EDGE_DETECT:
            m_filter = std::make_unique<FilterEdge>();
            break;
        case FILTER_BLUR:
            m_filter = std::make_unique<FilterBlur>();
            break;
        case FILTER_SCALE:
            m_filter = std::make_unique<FilterScale>();
            break;
        case FILTER_ROTATE:
            m_filter = std::make_unique<FilterRotate>();
            break;
        case FILTER_SPECIAL_1:
            m_filter = std::make_unique<FilterSharpen>();
            break;
        case FILTER_SPECIAL_2:
            m_filter = std::make_unique<FilterUnsharp>();
            break;
        default:
            m_filter = std::make_unique<FilterEdge>();
    }
}
