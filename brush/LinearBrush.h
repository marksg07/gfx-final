#ifndef LinearBrush_Header
#define LinearBrush_Header


#include "Brush.h"

/**
 * @class LinearBrush
 *
 * Linear brush, whose mask has linear falloff.
 */
class LinearBrush
    : public Brush
{

public:
    LinearBrush(BGRA color, int radius);
    virtual ~LinearBrush();

protected:
    void makeMask(); // Constructs the mask for this brush.
};

#endif
