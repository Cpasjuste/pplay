// Scrolling rules of the menus (src/menus/menu_scroll.h): the row under the cursor must
// always be in the middle slot of a list that does not fit, and always on screen.
#include <cstdio>
#include <cstdlib>
#include "menus/menu_scroll.h"

static int failures = 0;

#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

int main() {
    // 720p layout of pplay: menu 720 high, rows of 64 starting at 200, 32 free at the bottom
    CHECK(pplay::menuMaxVisible(720, 200, 0, 64, 32) == 7, "no bottom button: 7 rows");
    CHECK(pplay::menuMaxVisible(720, 200, 1, 64, 32) == 6, "one bottom button: 6 rows");
    CHECK(pplay::menuMaxVisible(720, 200, 2, 64, 32) == 5, "two bottom buttons: 5 rows");
    // 1080p (scaling 1.5)
    CHECK(pplay::menuMaxVisible(1080, 300, 0, 96, 48) == 7, "1080p: 7 rows");
    // tiny menu: never less than one row
    CHECK(pplay::menuMaxVisible(100, 200, 3, 64, 32) == 1, "at least one row");

    for (int maxVisible = 1; maxVisible <= 9; maxVisible++) {
        for (int count = 1; count <= 120; count++) {
            int current = 0;
            for (int rank = 0; rank < count; rank++) {
                int first = pplay::menuScrollIndex(count, maxVisible, rank, current);
                int slot = rank - first;
                if (count <= maxVisible) {
                    CHECK(first == 0, "short list must not scroll (count=%d vis=%d)", count, maxVisible);
                } else {
                    CHECK(slot == maxVisible / 2, "row %d/%d not in the middle slot (vis=%d slot=%d)",
                          rank, count, maxVisible, slot);
                }
                CHECK(slot >= 0 && slot < maxVisible, "row %d/%d is off screen (vis=%d)", rank, count, maxVisible);
                current = first;
            }
            // cursor on a bottom button: the list stays where it was
            if (count > maxVisible) {
                CHECK(pplay::menuScrollIndex(count, maxVisible, -1, 5) == 5, "bottom button keeps the scroll");
            }
        }
    }

    // walking down then wrapping to the top moves the list by exactly one row each time
    {
        const int count = 40, vis = 7;
        int prev = pplay::menuScrollIndex(count, vis, 0, 0);
        for (int rank = 1; rank < count; rank++) {
            int first = pplay::menuScrollIndex(count, vis, rank, prev);
            CHECK(first == prev + 1, "list must move one row per step (rank %d)", rank);
            prev = first;
        }
    }

    if (failures) {
        printf("%d menu scroll test(s) failed\n", failures);
        return 1;
    }
    printf("menu scroll tests passed\n");
    return 0;
}
