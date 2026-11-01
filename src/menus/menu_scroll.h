//
// Scrolling rules of the menus, kept free of any UI code so they can be tested on a PC.
//

#ifndef PPLAY_MENU_SCROLL_H
#define PPLAY_MENU_SCROLL_H

#include <algorithm>

namespace pplay {

    // How many "top" rows fit between the title area and the "bottom" rows (at least 1).
    // step: height of a row, top: where the first row starts, margin: free space at the very bottom.
    inline int menuMaxVisible(float menuHeight, float top, int bottomCount, float step, float margin) {
        float avail = menuHeight - top - ((float) bottomCount * step) - margin;
        return std::max(1, (int) (avail / step));
    }

    // First visible row for a list of topCount rows when the cursor is on row selectedRank
    // (-1: the cursor is on a "bottom" button, the list stays where it was).
    //
    // - the list fits: no scrolling;
    // - the list is longer than the menu: the row under the cursor is always the middle slot
    //   (maxVisible / 2), also for the first and the last rows, whose outer slots stay empty.
    inline int menuScrollIndex(int topCount, int maxVisible, int selectedRank, int current) {
        if (topCount <= maxVisible) {
            return 0;
        }
        if (selectedRank < 0) {
            return current;
        }
        return selectedRank - maxVisible / 2;
    }
}

#endif //PPLAY_MENU_SCROLL_H
