#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <memory>

class XYPad : public juce::Component
{
public:
    void paint(juce::Graphics &g)
    {
        float radius = 5.0f;
        juce::Rectangle<float> r(radius, radius);
        float x = std::clamp(x_, 0.0f, 1.0f);
        float y = std::clamp(y_, 0.0f, 1.0f);
        r.setCentre(x * getLocalBounds().getWidth(), y * getLocalBounds().getHeight());
        g.setColour(juce::Colour::fromRGB(255, 255, 255));
        g.drawEllipse(r, 2.0f);
        g.drawRect(getLocalBounds(), 1.0f);
    }

    void set_position_x(float x)
    {
        x_ = x;
        repaint();
    }

    void set_position_y(float y)
    {
        y_ = y;
        repaint();
    }

private:
    float x_ = 0.1f;
    float y_ = 0.1f;
};

class HandUI : public juce::Component
{
public:
    HandUI()
    {
        addAndMakeVisible(label);
        label.setText("Hand", juce::NotificationType::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(xy_pad);

        z_slider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
        z_slider.setRange(0.0f, 1.0f, 0.0f);
        addAndMakeVisible(z_slider);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label.setBoundsRelative(0.0f, 0.0f, 1.0f, 0.1f);
        area = area.withTrimmedTop(area.getHeight() * 0.1);

        juce::FlexBox fb;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;

        juce::FlexItem xy(static_cast<float>(area.getWidth()) / 3.0f * 2.0f, static_cast<float>(area.getHeight()), xy_pad);
        juce::FlexItem z(static_cast<float>(area.getWidth()) / 3.0f, static_cast<float>(area.getHeight()), z_slider);

        fb.items.addArray({xy, z});
        fb.performLayout(area);
    }

    void set_position_x(float x)
    {
        xy_pad.set_position_x(x);
    }

    void set_position_y(float y)
    {
        xy_pad.set_position_y(y);
    }

    void set_position_z(float z)
    {
        z_slider.setValue(z);
    }

private:
    juce::Label label;
    XYPad xy_pad;
    juce::Slider z_slider;
};
