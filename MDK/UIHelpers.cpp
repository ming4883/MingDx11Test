/*
  ==============================================================================

    UIHelpers.cpp
    Created: 16 Jun 2014 12:13:14pm
    Author:  ming

  ==============================================================================
*/

#include "UIHelpers.h"

//==============================================================================
void TableListBoxModelLambda::addColumn (const ColumnMeta& meta)
{
    columnIdMapping_.set (meta.name.hashCode(), columnMetas_.size());
    columnMetas_.add (meta);
}

void TableListBoxModelLambda::applyToView (juce::TableListBox& view)
{
    view.getHeader().setSize (0, 22);
    view.setRowHeight (15);
    for (int i = 0; i < columnMetas_.size(); ++i)
    {
        ColumnMeta& meta = columnMetas_.getReference (i);
        view.getHeader().addColumn (meta.name, meta.name.hashCode(), meta.width, meta.minWidth, meta.maxWidth, meta.flags);
    }
}

void TableListBoxModelLambda::paintRowBackground (juce::Graphics& g, int row, int width, int height, bool isSelected)
{
    juce::Colour bgcolour = juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::TextEditor::backgroundColourId);

    bgcolour = bgcolour.darker ((row % 2 == 0) ? 0.0625f : 0.125f);

    if (isSelected)
        bgcolour = juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::TextEditor::highlightColourId);

    g.fillAll (bgcolour);
}

void TableListBoxModelLambda::paintCell (juce::Graphics& g, int row, int colId, int width, int height, bool isSelected)
{
    if (!columnIdMapping_.contains (colId))
        return;

    ColumnMeta& meta = columnMetas_.getReference (columnIdMapping_[colId]);
    meta.onPaintCell (g, row, width, height, isSelected);
}

void TableListBoxModelLambda::drawText (juce::Graphics& g, int width, int height, bool isSelected, const juce::String& text)
{
    juce::Colour bgcolour;

    if (isSelected)
        bgcolour = juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::TextEditor::highlightedTextColourId);
    else
        bgcolour = juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::TextEditor::textColourId);

    g.setColour (bgcolour);
    g.drawText (text, 0, 0, width, height, juce::Justification::centredLeft, true);
}

//==============================================================================
CustomLookAndFeel::CustomLookAndFeel()
{
    juce::Colour commonBg = juce::Colour::fromRGB (63, 63, 63);
    juce::Colour commonFg = juce::Colour::fromRGB (225, 225, 225);

    juce::Colour commonHlBg = juce::Colour::fromRGB (127, 127, 255);
    juce::Colour commonHlFg = juce::Colour::fromRGB (31, 31, 31);

    juce::Colour commonLbBg = juce::Colour::fromRGB (97, 97, 97);
    juce::Colour commonLbFg = juce::Colour::fromRGB (225, 225, 225);

    // text related
    setColour (juce::PropertyComponent::backgroundColourId, commonBg.brighter (0.125f));
    setColour (juce::PropertyComponent::labelTextColourId, commonFg);


    setColour (juce::ListBox::backgroundColourId, commonLbBg);
    setColour (juce::ListBox::textColourId, commonLbFg);

    setColour (juce::DirectoryContentsDisplayComponent::textColourId, commonLbFg.darker (0.5f));

    setColour (juce::TextEditor::backgroundColourId, commonLbBg);
    setColour (juce::TextEditor::textColourId, commonLbFg);

    setColour (juce::Label::backgroundColourId, commonLbBg);
    setColour (juce::Label::textColourId, commonLbFg);

    setColour (juce::ComboBox::backgroundColourId, commonLbBg);
    setColour (juce::ComboBox::textColourId, commonLbFg);

    setColour (juce::ComboBox::buttonColourId, commonBg);
    setColour (juce::ComboBox::arrowColourId, commonFg);

    setColour (juce::Slider::textBoxBackgroundColourId, commonLbBg);
    setColour (juce::Slider::textBoxTextColourId, commonLbFg);

    setColour (juce::DirectoryContentsDisplayComponent::highlightColourId, commonHlBg);
    setColour (juce::TextEditor::highlightColourId, commonHlBg);
    setColour (juce::TextEditor::highlightedTextColourId, commonHlFg);
    setColour (juce::Slider::textBoxHighlightColourId, commonHlBg);

    setColour (juce::TextEditor::focusedOutlineColourId, commonLbBg.contrasting (0.25f));

    // menu
    setColour (juce::PopupMenu::backgroundColourId, commonBg);
    setColour (juce::PopupMenu::textColourId, commonFg);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, commonHlBg);
    setColour (juce::PopupMenu::highlightedTextColourId, commonHlFg);

    // buttons
    setColour (juce::DrawableButton::backgroundColourId, commonBg);
    setColour (juce::DrawableButton::textColourId, commonFg);
    setColour (juce::DrawableButton::backgroundOnColourId, commonHlBg);
    setColour (juce::DrawableButton::textColourOnId, commonHlFg);

    setColour (juce::TextButton::buttonColourId, commonBg);
    setColour (juce::TextButton::textColourOffId, commonFg);
    setColour (juce::TextButton::buttonOnColourId, commonHlBg);
    setColour (juce::TextButton::textColourOnId, commonHlFg);

    setColour (juce::ToggleButton::textColourId, commonFg);

    // windows
    setColour (juce::ResizableWindow::backgroundColourId, commonBg);
    setColour (juce::DocumentWindow::backgroundColourId, commonBg);
    setColour (juce::AlertWindow::backgroundColourId, commonBg);
    setColour (juce::AlertWindow::textColourId, commonFg);
    setColour (juce::DialogWindow::textColourId, commonFg);

    setColour (juce::FileChooserDialogBox::titleTextColourId, commonFg);

}

CustomLookAndFeel::~CustomLookAndFeel()
{
}

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    return juce::Font (15.0f);
}

class CustomLookAndFeel::DocumentWindowButton : public juce::Button
{
public:
    DocumentWindowButton (const juce::String& name, juce::Colour c, const juce::Path& normal, const juce::Path& toggled)
        : juce::Button (name), colour (c), normalShape (normal), toggledShape (toggled)
    {
    }

    void paintButton (juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        using namespace juce;

        const float cx = getWidth() * 0.5f, cy = getHeight() * 0.5f;
        const float diam = jmin (cx, cy) * (isButtonDown ? 0.60f : 0.65f);

        Colour c (colour);

        if (!isEnabled())
            c = c.withAlpha (0.5f);

        g.setColour (c);

        if (isMouseOverButton)
            g.drawEllipse (cx - diam, cy - diam, diam * 2.0f, diam * 2.0f, diam * 0.2f);

        Path& p = getToggleState() ? toggledShape : normalShape;

        float scale = 0.55f;
        g.fillPath (p, p.getTransformToScaleToFit (cx - diam * scale, cy - diam * scale,
                    diam * 2.0f * scale, diam * 2.0f * scale, true));
    }

private:
    juce::Colour colour;
    juce::Path normalShape, toggledShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentWindowButton)
};

juce::Button* CustomLookAndFeel::createDocumentWindowButton (int buttonType)
{
    using namespace juce;
    Path shape;
    const float crossThickness = 0.25f;

    Colour colour = findColour (DocumentWindow::backgroundColourId).contrasting();

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (Line<float> (0.0f, 0.0f, 1.0f, 1.0f), crossThickness * 1.4f);
        shape.addLineSegment (Line<float> (1.0f, 0.0f, 0.0f, 1.0f), crossThickness * 1.4f);

        return new DocumentWindowButton ("close", colour, shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        return new DocumentWindowButton ("minimise", colour, shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment (Line<float> (0.5f, 0.0f, 0.5f, 1.0f), crossThickness);
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new DocumentWindowButton ("maximise", colour, shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

void CustomLookAndFeel::drawDocumentWindowTitleBar (juce::DocumentWindow& window, juce::Graphics& g, int w, int h, int titleSpaceX, int titleSpaceW, const juce::Image* icon, bool drawTitleTextOnLeft)
{
    using namespace juce;
    const bool isActive = window.isActiveWindow();

    g.setGradientFill (ColourGradient (window.getBackgroundColour().contrasting (isActive ? 0.15f : 0.05f),
                                       0.0f, 0.0f,
                                       window.getBackgroundColour().contrasting (0.05f),
                                       0.0f, (float) h, false));
    g.fillAll();

    Font font (h * 0.5f, Font::bold);
    g.setFont (font);

    int textW = font.getStringWidth (window.getName());
    int iconW = 0;
    int iconH = 0;

    if (icon != nullptr)
    {
        iconH = (int) font.getHeight();
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    int textX = drawTitleTextOnLeft ? titleSpaceX
                : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
                           RectanglePlacement::centred, false);
        textX += iconW;
        textW -= iconW;
    }

    if (window.isColourSpecified (DocumentWindow::textColourId) || isColourSpecified (DocumentWindow::textColourId))
        g.setColour (window.findColour (DocumentWindow::textColourId));
    else
        g.setColour (window.getBackgroundColour().contrasting (isActive ? 0.75f : 0.25f));

    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

void CustomLookAndFeel::drawTableHeaderBackground (juce::Graphics& g, juce::TableHeaderComponent& header)
{
    using namespace juce;
    Rectangle<int> r (header.getLocalBounds());

    g.setColour (findColour (ListBox::textColourId).withAlpha (0.5f));
    g.fillRect (r.removeFromBottom (1));

    g.setColour (findColour (ListBox::backgroundColourId));
    g.fillRect (r);

    g.setColour (findColour (ListBox::textColourId).withAlpha (0.5f));

    for (int i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).removeFromRight (1));
}

void CustomLookAndFeel::drawTableHeaderColumn (juce::Graphics& g, const juce::String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags)
{
    using namespace juce;

    Rectangle<int> area (width, height);
    area.reduce (4, 0);

    if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
    {
        Path sortArrow;
        sortArrow.addTriangle (0.0f, 0.0f,
                               0.5f, (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
                               1.0f, 0.0f);

        g.setColour (findColour (ListBox::textColourId));
        g.fillPath (sortArrow, sortArrow.getTransformToScaleToFit (area.removeFromRight (height / 2).reduced (2).toFloat(), true));
    }

    g.setColour (findColour (ListBox::textColourId));
    Font font (height * 0.5f, Font::bold);

    g.setFont (font);
    g.drawFittedText (columnName, area, Justification::centredLeft, 1);
}