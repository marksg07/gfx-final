#ifndef SpecialBrush_Header
#define SpecialBrush_Header

#include "Brush.h"

/**
 * @class SmudgeBrush
 *
 * Smudge brush, which smudges color values across the canvas as the mouse is dragged.
 */
class SmudgeBrush : public Brush {

public:
    SmudgeBrush(BGRA color, int radius);
    virtual ~SmudgeBrush();

    virtual void brushDown(int x, int y, Canvas2D *canvas) override;
    virtual void brushDragged(int x, int y, Canvas2D *canvas) override;
    virtual void brushUp(int x, int y, Canvas2D *canvas) override;

protected:
    // Constructs the mask for this brush.
    void makeMask();

    //! Copy paint in the bounding box of the brush from canvas to m_paint
    virtual void pickUpPaint(int x, int y, Canvas2D* canvas);

    virtual void placePaint(int x, int y, Canvas2D *canvas) override;

    std::vector<BGRA> m_buffer;

};

#endif
