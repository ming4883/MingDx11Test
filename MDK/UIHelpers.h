/*
  ==============================================================================

    UIHelpers.h
    Created: 16 Jun 2014 12:13:14pm
    Author:  ming

  ==============================================================================
*/

#ifndef UIHELPERS_H_INCLUDED
#define UIHELPERS_H_INCLUDED

#include <AppConfig.h>
#include <modules/juce_gui_basics/juce_gui_basics.h>

#include <functional>

#define juce_default_dlg_color juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::DialogWindow::backgroundColourId)

class TableListBoxModelLambda : public juce::TableListBoxModel
{
public:
    struct ColumnMeta
    {
        juce::String name;
        int width;
        int minWidth, maxWidth;
        int flags;
        std::function<void (juce::Graphics& g, int row, int width, int height, bool isSelected)> onPaintCell;

        ColumnMeta() : width (50), minWidth (-1), maxWidth (-1), flags (juce::TableHeaderComponent::notResizableOrSortable)
        {
        }
    };

    void addColumn (const ColumnMeta& meta);

    void applyToView (juce::TableListBox& view);

    void drawText (juce::Graphics& g, int width, int height, bool isSelected, const juce::String& text);

    template<typename Scalar>
    void drawVec2 (juce::Graphics& g, int width, int height, bool isSelected, const Scalar* vec2)
    {
        juce::String text;
        text<< "["
            << juce::String ((double)vec2[0], 2) << ","
            << juce::String ((double)vec2[1], 2) << "]";
        drawText (g, width, height, isSelected, text);
    }

protected:
    void paintRowBackground (juce::Graphics& g, int row, int width, int height, bool isSelected) override;

    void paintCell (juce::Graphics& g, int row, int colId, int width, int height, bool isSelected) override;

private:
    juce::Array<ColumnMeta> columnMetas_;
    juce::HashMap<int, int> columnIdMapping_;
};

class CustomLookAndFeel : public juce::LookAndFeel_V3
{
public:
    CustomLookAndFeel();
    virtual ~CustomLookAndFeel();

    juce::Font getPopupMenuFont() override;

    juce::Button* createDocumentWindowButton (int buttonType) override;

    void drawDocumentWindowTitleBar (juce::DocumentWindow&, juce::Graphics&, int w, int h, int titleSpaceX, int titleSpaceW, const juce::Image* icon, bool drawTitleTextOnLeft) override;

    void drawTableHeaderBackground (juce::Graphics& g, juce::TableHeaderComponent& header) override;

    void drawTableHeaderColumn (juce::Graphics& g, const juce::String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override;

protected:
    class DocumentWindowButton;
};


#endif  // UIHELPERS_H_INCLUDED
