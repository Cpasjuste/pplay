#include <cstdio>
#include "stick_direction.h"

using namespace pplay;
static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)
typedef StickDir D;

int main() {
    // centre and small drift: nothing
    CHECK(stickDirection(0, 0, D::None) == D::None);
    CHECK(stickDirection(900, -1200, D::None) == D::None);
    CHECK(stickDirection(0, -(STICK_PRESS - 1), D::None) == D::None);

    // four directions (y negative = up, like SDL)
    CHECK(stickDirection(0, -STICK_PRESS, D::None) == D::Up);
    CHECK(stickDirection(0, 32767, D::None) == D::Down);
    CHECK(stickDirection(-32768, 0, D::None) == D::Left);
    CHECK(stickDirection(20000, 0, D::None) == D::Right);

    // diagonals: the strongest axis, never two directions
    CHECK(stickDirection(-20000, -10000, D::None) == D::Left);
    CHECK(stickDirection(10000, 20000, D::None) == D::Down);
    CHECK(stickDirection(15000, -15000, D::None) == D::Up);   // tie: vertical

    // hysteresis: pressed stays until under STICK_RELEASE
    CHECK(stickDirection(0, -(STICK_RELEASE + 1), D::Up) == D::Up);
    CHECK(stickDirection(0, -(STICK_RELEASE - 1), D::Up) == D::None);
    CHECK(stickDirection(0, -(STICK_RELEASE + 1), D::None) == D::None);

    // sweeping from up to right passes through a single direction at each point
    CHECK(stickDirection(3000, -20000, D::Up) == D::Up);
    CHECK(stickDirection(20000, -3000, D::Up) == D::Right);

    if (failures) { printf("%d FAILURE(S)\n", failures); return 1; }
    printf("stick: all tests passed\n");
    return 0;
}
