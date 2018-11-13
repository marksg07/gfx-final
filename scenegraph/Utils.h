#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <algorithm>
#include <functional>

class Utils
{
public:

    template<typename T, typename F>
    static void transform(std::vector<T>& in, std::vector<T>& out, const F& func, bool back_insert = true)
    {
        out.reserve(in.size() + out.size());

        if (back_insert)
        {
            std::transform(in.begin(), in.end(), std::back_inserter(out), func);
        } else {
            std::transform(in.begin(), in.end(), out.begin(), func);
        }
    }


private:
    Utils();
};

#endif // UTILS_H
