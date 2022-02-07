//
// Created by cpasjuste on 09/01/19.
//

#include <sstream>
#include <iomanip>
#include <ctime>

#include "cross2d/c2d.h"
#include "main.h"
#include "status_bar.h"
#include "utility.h"

#ifdef __SWITCH__

#include <switch.h>

#endif

using namespace c2d;

class Battery : public RectangleShape {

public:

    explicit Battery(const FloatRect &rect, const Vector2f &scaling) : RectangleShape({0, 0}) {

        float outline = std::ceil(scaling.x);
        Battery::setFillColor(Color::Transparent);
        Battery::setOutlineColor(COLOR_FONT);
        Battery::setOutlineThickness(outline);
        Battery::setOrigin(Origin::Right);
        Battery::setPosition((rect.width - (8 * scaling.x)) + outline, (rect.height / 2) + outline);
        Battery::setSize(32 * scaling.x, rect.height - (scaling.y * 16));

        maxWidth = Battery::getSize().x - (4 * scaling.x);
        Vector2f size = {maxWidth, Battery::getSize().y - (4 * scaling.y)};
        batteryRect = new RectangleShape(size);
        batteryRect->setFillColor(COLOR_FONT);
        batteryRect->setOrigin(Origin::Right);
        batteryRect->setPosition(Battery::getSize().x - (2 * scaling.x),
                                 (Battery::getSize().y + (outline / 2)) / 2);
        Battery::add(batteryRect);

        auto leftRect = new RectangleShape({scaling.x * 6, rect.height / 3});
        leftRect->setFillColor(COLOR_FONT);
        leftRect->setOrigin(Origin::Right);
        leftRect->setPosition(0, batteryRect->getPosition().y);
        Battery::add(leftRect);
    }

    void onUpdate() override {

        if (!isVisible()) {
            return;
        }

#ifdef __SWITCH__
        psmGetBatteryChargePercentage(&percent);
#endif
        // ps4: not std::clamp
        if (percent < 1) {
            percent = 1;
        } else if (percent > 100) {
            percent = 100;
        }

        float width = ((float) percent / 100) * maxWidth;
        if (width != batteryRect->getSize().x) {
            batteryRect->setSize(std::max(width, 2.0f), batteryRect->getSize().y);
            if (percent < 15) {
                batteryRect->setFillColor(Color::Red);
            } else if (percent < 30) {
                batteryRect->setFillColor(Color::Orange);
            } else {
                batteryRect->setFillColor(COLOR_FONT);
            }
        }

        RectangleShape::onUpdate();
    }

    RectangleShape *batteryRect = nullptr;
    unsigned int percent = 100;
    float maxWidth;
};

StatusBar::StatusBar(Main *main)
        : GradientRectangle({0, 0,
                             main->getSize().x, 32 * main->getScaling().y}) {

#ifdef __SWITCH__
    psmInitialize();
#endif

    setColor(COLOR_BG, Color::Transparent, Direction::Left);
    StatusBar::setPosition(0, -StatusBar::getSize().y);
    StatusBar::add(new TweenPosition(StatusBar::getPosition(), {0, 0}, 0.5f));

    battery = new Battery(StatusBar::getGlobalBounds(), main->getScaling());
    StatusBar::add(battery);

    // time
    timeText = new Text("12:00", main->getFontSize(Main::FontSize::Medium), main->getFont());
    timeText->setOrigin(Origin::Right);
    timeText->setPosition(battery->getPosition().x - battery->getSize().x - (main->getScaling().x * 20),
                          StatusBar::getSize().y / 2);
    timeText->setFillColor(COLOR_FONT);
    StatusBar::add(timeText);

    StatusBar::setVisibility(Visibility::Visible, true);
}

void StatusBar::onUpdate() {

    if (!isVisible()) {
        return;
    }

    time_t time_raw;
    struct tm *time_struct;

    time(&time_raw);
    time_struct = localtime(&time_raw);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << time_struct->tm_hour << ":";
    oss << std::setfill('0') << std::setw(2) << time_struct->tm_min;
    timeText->setString(oss.str());

    GradientRectangle::onUpdate();
}

StatusBar::~StatusBar() {
#ifdef __SWITCH__
    psmExit();
#endif
}
