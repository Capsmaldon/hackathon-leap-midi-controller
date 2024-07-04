#include <juce_gui_extra/juce_gui_extra.h>
#include <vector>
#include <memory>

class NoteTable : public juce::Component
{
public:
    NoteTable()
    {
        on_colour = juce::Colour::fromRGB(120, 120, 120);
        off_colour = juce::Colour::fromRGB(80, 80, 80);

        for (int i = 0; i < 4; ++i)
        {
            auto l = std::make_unique<juce::Label>();
            l->setText(std::to_string(i), juce::NotificationType::dontSendNotification);
            addAndMakeVisible(*l);
            l->setBorderSize(juce::BorderSize<int>(2));
            l->setJustificationType(juce::Justification::centred);
            l->setColour(juce::Label::outlineColourId, juce::Colour(255, 255, 255));
            l->setColour(juce::Label::backgroundColourId, off_colour);
            notes.push_back(std::move(l));
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto box_width = area.getWidth() / notes.size();
        for (unsigned int i = 0; i < notes.size(); ++i)
        {
            auto label_area = area.removeFromLeft(box_width);
            notes[i]->setBounds(label_area);
        }
    }

    void toggle_note(int index, int value)
    {
        notes[index]->setColour(juce::Label::backgroundColourId, value == 1 ? on_colour : off_colour);
    }

private:
    juce::Colour on_colour;
    juce::Colour off_colour;
    std::vector<std::unique_ptr<juce::Label>> notes;
};

class XYPad : public juce::Component
{
public:
    void paint(juce::Graphics &g)
    {
        float radius = 5.0f;
        juce::Rectangle<float> r(radius, radius);
        float x = std::clamp(x_, 0.0f, 1.0f);
        float y = 1.0f - std::clamp(y_, 0.0f, 1.0f);
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

        addAndMakeVisible(note_table);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto w = area.getWidth();
        auto h = area.getHeight();
        label.setBounds(area.removeFromTop(h * 0.1));
        note_table.setBounds(area.removeFromBottom(h * 0.25));
        xy_pad.setBounds(area.removeFromLeft(w * 0.8));
        z_slider.setBounds(area);
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

    void toggle_note(int index, int value)
    {
        note_table.toggle_note(index, value);
    }

private:
    juce::Label label;
    XYPad xy_pad;
    juce::Slider z_slider;
    NoteTable note_table;
};
