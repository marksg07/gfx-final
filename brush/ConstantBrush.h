#ifndef ConstantBrush_Header
#define ConstantBrush_Header

#include "Brush.h"
/**
 * @class ConstantBrush
 *
 * Constant brush with equal mask distribution in a circle.
 */
class ConstantBrush
    : public Brush
{

public:
    ConstantBrush(BGRA color, int radius);
    virtual ~ConstantBrush();

protected:
    void makeMask(); // Constructs the mask for this brush.
};

#endif
