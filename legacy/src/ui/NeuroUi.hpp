#pragma once

#include <imgui.h>

class NeuroUI
{
public:
    struct BrainViewport
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        bool hovered = false;
        bool dragging = false;
    };

public:
    void render();

    BrainViewport brainViewport() const
    {
        return brainViewport_;
    }

    bool cortexVisible() const
    {
        return showCortex_;
    }

    bool activityMapEnabled() const
    {
        return showActivity_;
    }

    float cortexOpacity() const
    {
        return cortexOpacity_;
    }

    float activityIntensity() const
    {
        return activityIntensity_;
    }

    bool consumeResetViewRequest()
    {
        const bool requested =
            resetViewRequested_;

        resetViewRequested_ =
            false;

        return requested;
    }

private:
    void renderHeader();
    void renderLeftPanel();
    void renderViewport();
    void renderRightPanel();
    void renderBottomPanel();
    void renderSystemBar();

    void renderSectionTitle(
        const char* title
    );

    void renderViewButton(
        const char* label,
        int index
    );

    void renderLayerRow(
        const char* label,
        bool& enabled,
        float& opacity
    );

    void renderActivityGraph(
        const char* label,
        float value,
        int graphIndex
    );

    void renderRegionCard(
        const char* label,
        int index
    );

    void renderMetric(
        const char* title,
        const char* value
    );

    void drawMiniBrain(
        ImDrawList* drawList,
        ImVec2 center,
        ImVec2 size
    );

private:
    bool showSkull_ = false;
    bool showCortex_ = true;
    bool showWhiteMatter_ = false;
    bool showNeurons_ = true;
    bool showSynapses_ = true;
    bool showVessels_ = false;
    bool showActivity_ = true;

    float skullOpacity_ = 0.20f;
    float cortexOpacity_ = 0.68f;
    float whiteMatterOpacity_ = 0.40f;
    float neuronOpacity_ = 1.00f;
    float synapseOpacity_ = 0.80f;
    float vesselOpacity_ = 0.30f;
    float activityIntensity_ = 0.72f;

    float transparency_ = 0.68f;
    float brightness_ = 0.72f;

    float timeline_ = 0.24f;

    int selectedView_ = 0;
    int selectedRegion_ = 2;

    bool playing_ = true;

    BrainViewport brainViewport_;

    bool resetViewRequested_ = false;
};