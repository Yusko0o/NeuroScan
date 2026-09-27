#include "NeuroUi.hpp"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
    constexpr float HeaderHeight = 68.0f;
    constexpr float LeftWidth = 220.0f;
    constexpr float RightWidth = 315.0f;
    constexpr float BottomHeight = 170.0f;
    constexpr float SystemHeight = 40.0f;
    constexpr float Gap = 7.0f;

    const ImVec4 Transparent =
        ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    const ImVec4 Panel =
        ImVec4(0.018f, 0.033f, 0.052f, 0.95f);

    const ImVec4 PanelDark =
        ImVec4(0.012f, 0.024f, 0.039f, 0.97f);

    const ImVec4 PanelLight =
        ImVec4(0.025f, 0.050f, 0.075f, 0.98f);

    const ImVec4 Hover =
        ImVec4(0.025f, 0.075f, 0.115f, 1.0f);

    const ImVec4 Active =
        ImVec4(0.030f, 0.115f, 0.175f, 1.0f);

    const ImVec4 Border =
        ImVec4(0.10f, 0.22f, 0.31f, 0.52f);

    const ImVec4 BorderBright =
        ImVec4(0.08f, 0.42f, 0.68f, 0.80f);

    const ImVec4 Cyan =
        ImVec4(0.08f, 0.57f, 0.90f, 1.0f);

    const ImVec4 CyanBright =
        ImVec4(0.18f, 0.72f, 1.00f, 1.0f);

    const ImVec4 Blue =
        ImVec4(0.18f, 0.48f, 0.95f, 1.0f);

    const ImVec4 Purple =
        ImVec4(0.55f, 0.30f, 1.00f, 1.0f);

    const ImVec4 Green =
        ImVec4(0.18f, 0.78f, 0.55f, 1.0f);

    const ImVec4 Orange =
        ImVec4(0.95f, 0.50f, 0.20f, 1.0f);

    const ImVec4 Red =
        ImVec4(0.95f, 0.23f, 0.28f, 1.0f);

    const ImVec4 Yellow =
        ImVec4(0.88f, 0.75f, 0.24f, 1.0f);

    const ImVec4 Text =
        ImVec4(0.86f, 0.90f, 0.95f, 1.0f);

    const ImVec4 Muted =
        ImVec4(0.40f, 0.49f, 0.60f, 1.0f);

    const ImVec4 VeryMuted =
        ImVec4(0.25f, 0.32f, 0.40f, 1.0f);

    ImU32 c(const ImVec4& color)
    {
        return ImGui::GetColorU32(color);
    }

    void textMuted(const char* text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Muted);
        ImGui::TextUnformatted(text);
        ImGui::PopStyleColor();
    }

    void textNormal(const char* text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Text);
        ImGui::TextUnformatted(text);
        ImGui::PopStyleColor();
    }

    void textCyan(const char* text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, CyanBright);
        ImGui::TextUnformatted(text);
        ImGui::PopStyleColor();
    }

    void beginPanel()
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, Panel);
        ImGui::PushStyleColor(ImGuiCol_Border, Border);

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding,
            ImVec2(12.0f, 10.0f)
        );
    }

    void endPanel()
    {
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    void separator()
    {
        ImGui::PushStyleColor(
            ImGuiCol_Separator,
            ImVec4(
                Border.x,
                Border.y,
                Border.z,
                0.55f
            )
        );

        ImGui::Separator();
        ImGui::PopStyleColor();
    }

    bool button(
        const char* label,
        ImVec2 size
    )
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            PanelDark
        );

        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            Hover
        );

        ImGui::PushStyleColor(
            ImGuiCol_ButtonActive,
            Active
        );

        ImGui::PushStyleColor(
            ImGuiCol_Border,
            Border
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_FrameBorderSize,
            1.0f
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_FrameRounding,
            3.0f
        );

        bool pressed =
            ImGui::Button(
                label,
                size
            );

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);

        return pressed;
    }

    void drawCorner(
        ImDrawList* dl,
        ImVec2 p,
        float sx,
        float sy
    )
    {
        const float length = 20.0f;

        dl->AddLine(
            p,
            ImVec2(
                p.x + length * sx,
                p.y
            ),
            IM_COL32(40, 135, 190, 110),
            1.0f
        );

        dl->AddLine(
            p,
            ImVec2(
                p.x,
                p.y + length * sy
            ),
            IM_COL32(40, 135, 190, 110),
            1.0f
        );
    }
}

void NeuroUI::render()
{
    ImGuiIO& io =
        ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(0.0f, 0.0f)
    );

    ImGui::SetNextWindowSize(
        io.DisplaySize
    );

    ImGui::PushStyleColor(
        ImGuiCol_WindowBg,
        Transparent
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        Transparent
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(7.0f, 7.0f)
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowBorderSize,
        0.0f
    );

    ImGui::Begin(
        "##NeuroScanRoot",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoBackground
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding,
        3.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_GrabRounding,
        5.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding,
        ImVec2(7.0f, 5.0f)
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2(6.0f, 6.0f)
    );

    ImGui::PushStyleColor(ImGuiCol_Text, Text);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, Muted);

    ImGui::PushStyleColor(
        ImGuiCol_FrameBg,
        ImVec4(0.020f, 0.040f, 0.062f, 1.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgHovered,
        Hover
    );

    ImGui::PushStyleColor(
        ImGuiCol_FrameBgActive,
        Active
    );

    ImGui::PushStyleColor(
        ImGuiCol_SliderGrab,
        ImVec4(0.45f, 0.62f, 0.80f, 1.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_SliderGrabActive,
        CyanBright
    );

    ImGui::PushStyleColor(
        ImGuiCol_CheckMark,
        CyanBright
    );

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        PanelDark
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        Hover
    );

    ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        Active
    );

    ImGui::PushStyleColor(
        ImGuiCol_Header,
        PanelLight
    );

    ImGui::PushStyleColor(
        ImGuiCol_HeaderHovered,
        Hover
    );

    ImGui::PushStyleColor(
        ImGuiCol_HeaderActive,
        Active
    );

    renderHeader();
    renderLeftPanel();
    renderViewport();
    renderRightPanel();
    renderBottomPanel();
    renderSystemBar();

    ImGui::PopStyleColor(14);
    ImGui::PopStyleVar(4);

    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void NeuroUI::renderHeader()
{
    const ImVec2 root =
        ImGui::GetWindowSize();

    ImGui::SetCursorPos(
        ImVec2(7.0f, 7.0f)
    );

    beginPanel();

    ImGui::BeginChild(
        "##Header",
        ImVec2(
            root.x - 14.0f,
            HeaderHeight - 7.0f
        ),
        true,
        ImGuiWindowFlags_NoScrollbar
    );

    ImDrawList* dl =
        ImGui::GetWindowDrawList();

    const ImVec2 wp =
        ImGui::GetWindowPos();

    const ImVec2 logo(
        wp.x + 29.0f,
        wp.y + 28.0f
    );

    dl->AddCircle(
        logo,
        17.0f,
        c(Cyan),
        6,
        1.7f
    );

    dl->AddCircle(
        logo,
        8.0f,
        IM_COL32(30, 135, 210, 130),
        20,
        1.0f
    );

    dl->AddCircleFilled(
        logo,
        2.7f,
        c(CyanBright)
    );

    ImGui::SetCursorPos(
        ImVec2(57.0f, 8.0f)
    );

    ImGui::SetWindowFontScale(
        1.13f
    );

    textNormal(
        "NEUROSCAN"
    );

    ImGui::SetWindowFontScale(
        1.0f
    );

    ImGui::SetCursorPos(
        ImVec2(58.0f, 34.0f)
    );

    textMuted(
        "v2.7.4"
    );

    ImGui::SetCursorPos(
        ImVec2(245.0f, 7.0f)
    );

    textMuted(
        "SUBJECT"
    );

    ImGui::SetCursorPos(
        ImVec2(245.0f, 30.0f)
    );

    textNormal(
        "John Doe"
    );

    ImGui::SetCursorPos(
        ImVec2(410.0f, 7.0f)
    );

    textMuted(
        "SESSION"
    );

    ImGui::SetCursorPos(
        ImVec2(410.0f, 30.0f)
    );

    textNormal(
        "2025_06_01_001"
    );

    ImGui::SetCursorPos(
        ImVec2(630.0f, 7.0f)
    );

    textMuted(
        "MODE"
    );

    ImGui::SetCursorPos(
        ImVec2(630.0f, 26.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_Button,
        ImVec4(0.025f, 0.095f, 0.140f, 1.0f)
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        Text
    );

    ImGui::Button(
        "   LIVE",
        ImVec2(72.0f, 27.0f)
    );

    ImGui::PopStyleColor(2);

    const ImVec2 livePos =
        ImGui::GetItemRectMin();

    dl->AddCircleFilled(
        ImVec2(
            livePos.x + 13.0f,
            livePos.y + 13.5f
        ),
        4.0f,
        c(CyanBright)
    );

    ImGui::SetCursorPos(
        ImVec2(750.0f, 20.0f)
    );

    button(
        "~",
        ImVec2(43.0f, 29.0f)
    );

    ImGui::SameLine();

    button(
        "^",
        ImVec2(43.0f, 29.0f)
    );

    ImGui::SameLine();

    button(
        "O",
        ImVec2(43.0f, 29.0f)
    );

    ImGui::SameLine();

    button(
        "*",
        ImVec2(43.0f, 29.0f)
    );

    const float right =
        ImGui::GetWindowWidth();

    ImGui::SetCursorPos(
        ImVec2(
            right - 355.0f,
            7.0f
        )
    );

    textMuted(
        "DATA QUALITY"
    );

    ImGui::SetCursorPos(
        ImVec2(
            right - 355.0f,
            29.0f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        Green
    );

    ImGui::TextUnformatted(
        "EXCELLENT"
    );

    ImGui::PopStyleColor();

    const ImVec2 q(
        wp.x + right - 252.0f,
        wp.y + 28.0f
    );

    dl->AddCircle(
        q,
        17.0f,
        IM_COL32(25, 60, 70, 230),
        32,
        2.5f
    );

    dl->PathArcTo(
        q,
        17.0f,
        -1.5708f,
        -1.5708f +
        6.28318f * 0.96f,
        32
    );

    dl->PathStroke(
        c(Green),
        false,
        2.5f
    );

    dl->AddText(
        ImVec2(q.x - 10.0f, q.y - 7.0f),
        c(Text),
        "96%"
    );

    ImGui::SetCursorPos(
        ImVec2(
            right - 145.0f,
            7.0f
        )
    );

    textNormal(
        "14:37:42"
    );

    ImGui::SetCursorPos(
        ImVec2(
            right - 145.0f,
            29.0f
        )
    );

    textMuted(
        "01.06.2025"
    );

    ImGui::SetCursorPos(
        ImVec2(
            right - 43.0f,
            19.0f
        )
    );

    button(
        "===",
        ImVec2(31.0f, 29.0f)
    );

    ImGui::EndChild();

    endPanel();
}

void NeuroUI::renderSectionTitle(
    const char* title
)
{
    ImGui::PushStyleColor(
        ImGuiCol_Text,
        ImVec4(0.53f, 0.62f, 0.72f, 1.0f)
    );

    ImGui::TextUnformatted(
        title
    );

    ImGui::PopStyleColor();

    ImGui::Dummy(
        ImVec2(0.0f, 3.0f)
    );
}

void NeuroUI::renderViewButton(
    const char* label,
    int index
)
{
    const bool selected =
        selectedView_ == index;

    ImGui::PushID(index);

    if (selected)
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(0.030f, 0.095f, 0.155f, 1.0f)
        );

        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(0.035f, 0.120f, 0.180f, 1.0f)
        );

        ImGui::PushStyleColor(
            ImGuiCol_Border,
            BorderBright
        );

        ImGui::PushStyleColor(
            ImGuiCol_Text,
            Text
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_FrameBorderSize,
            1.0f
        );
    }

    if (
        ImGui::Button(
            label,
            ImVec2(-1.0f, 34.0f)
        )
    )
    {
        selectedView_ =
            index;
    }

    if (selected)
    {
        const ImVec2 min =
            ImGui::GetItemRectMin();

        const ImVec2 max =
            ImGui::GetItemRectMax();

        ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(min.x, min.y + 3.0f),
            ImVec2(min.x + 3.0f, max.y - 3.0f),
            c(CyanBright),
            1.0f
        );

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    }

    ImGui::PopID();
}

void NeuroUI::renderLayerRow(
    const char* label,
    bool& enabled,
    float& opacity
)
{
    ImGui::PushID(label);

    const ImVec2 p =
        ImGui::GetCursorScreenPos();

    const float cy =
        p.y + 7.0f;

    ImGui::InvisibleButton(
        "##eye",
        ImVec2(20.0f, 16.0f)
    );

    if (
        ImGui::IsItemClicked()
    )
    {
        enabled =
            !enabled;
    }

    ImDrawList* dl =
        ImGui::GetWindowDrawList();

    const ImU32 eyeColor =
        enabled
            ? c(Cyan)
            : c(VeryMuted);

    dl->AddEllipse(
        ImVec2(
            p.x + 9.0f,
            cy
        ),
        ImVec2(
            6.0f,
            3.5f
        ),
        eyeColor,
        0.0f,
        20,
        1.0f
    );

    dl->AddCircleFilled(
        ImVec2(
            p.x + 9.0f,
            cy
        ),
        1.5f,
        eyeColor
    );

    ImGui::SameLine();

    if (enabled)
    {
        textNormal(label);
    }
    else
    {
        textMuted(label);
    }

    ImGui::SameLine(
        158.0f
    );

    char value[16]{};

    std::snprintf(
        value,
        sizeof(value),
        "%d%%",
        static_cast<int>(
            opacity * 100.0f
        )
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        enabled
            ? ImVec4(
                0.40f,
                0.52f,
                0.66f,
                1.0f
            )
            : VeryMuted
    );

    ImGui::TextUnformatted(
        value
    );

    ImGui::PopStyleColor();

    ImGui::PopID();
}

void NeuroUI::renderLeftPanel()
{
    const ImVec2 root =
        ImGui::GetWindowSize();

    const float y =
        HeaderHeight + Gap;

    const float height =
        root.y -
        HeaderHeight -
        BottomHeight -
        SystemHeight -
        Gap * 4.0f;

    ImGui::SetCursorPos(
        ImVec2(7.0f, y)
    );

    beginPanel();

    ImGui::BeginChild(
        "##LeftPanel",
        ImVec2(
            LeftWidth,
            height
        ),
        true
    );

    renderSectionTitle(
        "VIEWS"
    );

    renderViewButton(
        "     OVERVIEW",
        0
    );

    renderViewButton(
        "     CORTEX",
        1
    );

    renderViewButton(
        "     SUBCORTICAL",
        2
    );

    renderViewButton(
        "     CONNECTOME",
        3
    );

    renderViewButton(
        "     NEURONAL",
        4
    );

    ImGui::Dummy(
        ImVec2(0.0f, 10.0f)
    );

    separator();

    ImGui::Dummy(
        ImVec2(0.0f, 7.0f)
    );

    renderSectionTitle(
        "VISUALIZATION"
    );

    textMuted(
        "Transparency"
    );

    ImGui::SetNextItemWidth(
        145.0f
    );

    ImGui::SliderFloat(
        "##Transparency",
        &transparency_,
        0.05f,
        1.0f,
        ""
    );

    ImGui::SameLine();

    ImGui::Text(
        "%d%%",
        static_cast<int>(
            transparency_ * 100.0f
        )
    );

    cortexOpacity_ =
        transparency_;

    textMuted(
        "Brightness"
    );

    ImGui::SetNextItemWidth(
        145.0f
    );

    ImGui::SliderFloat(
        "##Brightness",
        &brightness_,
        0.0f,
        1.0f,
        ""
    );

    ImGui::SameLine();

    ImGui::Text(
        "%d%%",
        static_cast<int>(
            brightness_ * 100.0f
        )
    );

    activityIntensity_ =
        brightness_;

    ImGui::Dummy(
        ImVec2(0.0f, 4.0f)
    );

    textMuted(
        "Neural Density"
    );

    ImGui::SameLine(
        130.0f
    );

    textNormal(
        "High"
    );

    textMuted(
        "Color Scheme"
    );

    ImGui::SameLine(
        130.0f
    );

    textNormal(
        "Electric"
    );

    ImGui::Dummy(
        ImVec2(0.0f, 9.0f)
    );

    separator();

    ImGui::Dummy(
        ImVec2(0.0f, 7.0f)
    );

    renderSectionTitle(
        "LAYERS"
    );

    renderLayerRow(
        "Skull",
        showSkull_,
        skullOpacity_
    );

    renderLayerRow(
        "Cortex",
        showCortex_,
        cortexOpacity_
    );

    renderLayerRow(
        "White Matter",
        showWhiteMatter_,
        whiteMatterOpacity_
    );

    renderLayerRow(
        "Neurons",
        showNeurons_,
        neuronOpacity_
    );

    renderLayerRow(
        "Synapses",
        showSynapses_,
        synapseOpacity_
    );

    renderLayerRow(
        "Vessels",
        showVessels_,
        vesselOpacity_
    );

    renderLayerRow(
        "Activity Map",
        showActivity_,
        activityIntensity_
    );

    ImGui::Dummy(
        ImVec2(0.0f, 4.0f)
    );

    button(
        "+   ADD LAYER",
        ImVec2(-1.0f, 31.0f)
    );

    ImGui::EndChild();

    endPanel();
}

void NeuroUI::renderViewport()
{
    const ImVec2 root =
        ImGui::GetWindowSize();

    const float x =
        LeftWidth +
        Gap * 2.0f;

    const float y =
        HeaderHeight +
        Gap;

    const float width =
        root.x -
        LeftWidth -
        RightWidth -
        Gap * 4.0f;

    const float height =
        root.y -
        HeaderHeight -
        BottomHeight -
        SystemHeight -
        Gap * 4.0f;

    ImGui::SetCursorPos(
        ImVec2(x, y)
    );

    ImGui::PushStyleColor(
        ImGuiCol_ChildBg,
        Transparent
    );

    ImGui::PushStyleColor(
        ImGuiCol_Border,
        Border
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ChildBorderSize,
        1.0f
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_ChildRounding,
        4.0f
    );

    ImGui::BeginChild(
        "##Viewport",
        ImVec2(
            width,
            height
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    ImDrawList* dl =
        ImGui::GetWindowDrawList();

    const ImVec2 wp =
        ImGui::GetWindowPos();

    const ImVec2 ws =
        ImGui::GetWindowSize();

    const float toolbarHeight =
        47.0f;

    const ImVec2 brainMin(
        wp.x + 4.0f,
        wp.y + 4.0f
    );

    const ImVec2 brainMax(
        wp.x + ws.x - 4.0f,
        wp.y + ws.y - toolbarHeight
    );

    brainViewport_.x =
        brainMin.x;

    brainViewport_.y =
        brainMin.y;

    brainViewport_.width =
        std::max(
            1.0f,
            brainMax.x -
            brainMin.x
        );

    brainViewport_.height =
        std::max(
            1.0f,
            brainMax.y -
            brainMin.y
        );

    ImGui::SetCursorScreenPos(
        brainMin
    );

    ImGui::InvisibleButton(
        "##BrainViewport",
        ImVec2(
            brainViewport_.width,
            brainViewport_.height
        ),
        ImGuiButtonFlags_MouseButtonLeft
    );

    brainViewport_.hovered =
        ImGui::IsItemHovered();

    brainViewport_.dragging =
        brainViewport_.hovered &&
        ImGui::IsMouseDragging(
            ImGuiMouseButton_Left
        );

    for (
        int i = 0;
        i < 70;
        ++i
    )
    {
        const float px =
            std::fmod(
                float(i * 117 + 27),
                brainViewport_.width
            );

        const float py =
            std::fmod(
                float(i * 71 + 14),
                brainViewport_.height
            );

        float brightness =
            0.04f +
            0.04f *
            std::sin(
                float(ImGui::GetTime()) *
                0.35f +
                float(i)
            );

        dl->AddCircleFilled(
            ImVec2(
                brainMin.x + px,
                brainMin.y + py
            ),
            i % 12 == 0
                ? 1.0f
                : 0.5f,
            IM_COL32(
                80,
                130,
                190,
                int(
                    brightness * 255.0f
                )
            )
        );
    }

    drawCorner(
        dl,
        brainMin,
        1.0f,
        1.0f
    );

    drawCorner(
        dl,
        ImVec2(
            brainMax.x,
            brainMin.y
        ),
        -1.0f,
        1.0f
    );

    drawCorner(
        dl,
        ImVec2(
            brainMin.x,
            brainMax.y
        ),
        1.0f,
        -1.0f
    );

    drawCorner(
        dl,
        brainMax,
        -1.0f,
        -1.0f
    );

    const ImVec2 orientationMin(
        brainMin.x + 16.0f,
        brainMin.y + 16.0f
    );

    const ImVec2 orientationMax(
        orientationMin.x + 72.0f,
        orientationMin.y + 74.0f
    );

    dl->AddRectFilled(
        orientationMin,
        orientationMax,
        IM_COL32(
            4,
            13,
            23,
            205
        ),
        4.0f
    );

    dl->AddRect(
        orientationMin,
        orientationMax,
        IM_COL32(
            30,
            75,
            105,
            150
        ),
        4.0f
    );

    dl->AddText(
        ImVec2(
            orientationMin.x + 32.0f,
            orientationMin.y + 8.0f
        ),
        c(Muted),
        "S"
    );

    dl->AddText(
        ImVec2(
            orientationMin.x + 10.0f,
            orientationMin.y + 31.0f
        ),
        c(Muted),
        "P"
    );

    dl->AddText(
        ImVec2(
            orientationMin.x + 33.0f,
            orientationMin.y + 31.0f
        ),
        c(Text),
        "A"
    );

    dl->AddText(
        ImVec2(
            orientationMin.x + 56.0f,
            orientationMin.y + 31.0f
        ),
        c(Muted),
        "L"
    );

    dl->AddText(
        ImVec2(
            orientationMin.x + 33.0f,
            orientationMin.y + 54.0f
        ),
        c(Muted),
        "I"
    );

    const float toolX =
        brainMin.x + 17.0f;

    const float toolY =
        brainMin.y + 105.0f;

    ImGui::SetCursorScreenPos(
        ImVec2(toolX, toolY)
    );

    button(
        "A",
        ImVec2(40.0f, 38.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(toolX, toolY + 44.0f)
    );

    button(
        "H",
        ImVec2(40.0f, 38.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(toolX, toolY + 88.0f)
    );

    button(
        "+",
        ImVec2(40.0f, 38.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(toolX, toolY + 132.0f)
    );

    button(
        "-",
        ImVec2(40.0f, 38.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(toolX, toolY + 176.0f)
    );

    button(
        "O",
        ImVec2(40.0f, 38.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(toolX, toolY + 220.0f)
    );

    button(
        "+",
        ImVec2(40.0f, 38.0f)
    );

    const float rightToolX =
        brainMax.x - 49.0f;

    ImGui::SetCursorScreenPos(
        ImVec2(
            rightToolX,
            brainMin.y + 17.0f
        )
    );

    button(
        "@",
        ImVec2(36.0f, 35.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(
            rightToolX,
            brainMin.y + 58.0f
        )
    );

    button(
        "O",
        ImVec2(36.0f, 35.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(
            rightToolX,
            brainMin.y + 99.0f
        )
    );

    button(
        "[]",
        ImVec2(36.0f, 35.0f)
    );

    ImGui::SetCursorScreenPos(
        ImVec2(
            rightToolX,
            brainMin.y + 140.0f
        )
    );

    button(
        "#",
        ImVec2(36.0f, 35.0f)
    );

    dl->AddText(
        ImVec2(
            brainMin.x + 20.0f,
            brainMax.y - 33.0f
        ),
        c(Muted),
        "1 cm"
    );

    dl->AddLine(
        ImVec2(
            brainMin.x + 20.0f,
            brainMax.y - 17.0f
        ),
        ImVec2(
            brainMin.x + 66.0f,
            brainMax.y - 17.0f
        ),
        IM_COL32(
            110,
            145,
            165,
            180
        ),
        1.0f
    );

    ImGui::SetCursorScreenPos(
        ImVec2(
            wp.x + 9.0f,
            brainMax.y + 7.0f
        )
    );

    if (
        button(
            playing_
                ? "||"
                : ">",
            ImVec2(38.0f, 31.0f)
        )
    )
    {
        playing_ =
            !playing_;
    }

    ImGui::SameLine();

    if (
        button(
            "RESET VIEW",
            ImVec2(95.0f, 31.0f)
        )
    )
    {
        resetViewRequested_ =
            true;
    }

    ImGui::SameLine();

    button(
        "FOCUS",
        ImVec2(70.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "ISOLATE",
        ImVec2(74.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "MEASURE",
        ImVec2(78.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "ANNOTATE",
        ImVec2(82.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "SCREENSHOT",
        ImVec2(95.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "RECORD",
        ImVec2(70.0f, 31.0f)
    );

    ImGui::EndChild();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void NeuroUI::drawMiniBrain(
    ImDrawList* dl,
    ImVec2 center,
    ImVec2 size
)
{
    dl->AddEllipse(
        center,
        ImVec2(
            size.x * 0.48f,
            size.y * 0.35f
        ),
        IM_COL32(
            35,
            100,
            170,
            180
        ),
        0.0f,
        42,
        1.0f
    );

    for (
        int i = 0;
        i < 8;
        ++i
    )
    {
        const float x =
            (
                float(i) -
                3.5f
            ) * 11.0f;

        dl->AddBezierCubic(
            ImVec2(
                center.x + x,
                center.y - 28.0f
            ),
            ImVec2(
                center.x + x - 10.0f,
                center.y - 8.0f
            ),
            ImVec2(
                center.x + x + 10.0f,
                center.y + 9.0f
            ),
            ImVec2(
                center.x + x,
                center.y + 27.0f
            ),
            IM_COL32(
                35,
                75,
                135,
                120
            ),
            1.0f
        );
    }

    const ImVec2 hotspot(
        center.x + 16.0f,
        center.y + 3.0f
    );

    dl->AddCircleFilled(
        hotspot,
        18.0f,
        IM_COL32(
            130,
            45,
            255,
            28
        )
    );

    dl->AddCircleFilled(
        hotspot,
        10.0f,
        IM_COL32(
            150,
            55,
            255,
            75
        )
    );

    dl->AddCircleFilled(
        hotspot,
        4.5f,
        IM_COL32(
            190,
            90,
            255,
            245
        )
    );
}

void NeuroUI::renderActivityGraph(
    const char* label,
    float value,
    int graphIndex
)
{
    ImGui::PushID(
        graphIndex
    );

    textMuted(
        label
    );

    ImGui::SameLine(
        248.0f
    );

    const ImVec4 graphColor =
        graphIndex == 0
            ? Blue
            : graphIndex == 1
                ? Cyan
                : graphIndex == 2
                    ? Yellow
                    : graphIndex == 3
                        ? Orange
                        : Red;

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        graphColor
    );

    ImGui::Text(
        "%d%%",
        static_cast<int>(
            value * 100.0f
        )
    );

    ImGui::PopStyleColor();

    const ImVec2 start =
        ImGui::GetCursorScreenPos();

    const float width =
        ImGui::GetContentRegionAvail().x;

    constexpr float graphHeight =
        23.0f;

    ImDrawList* dl =
        ImGui::GetWindowDrawList();

    dl->AddLine(
        ImVec2(
            start.x,
            start.y + graphHeight * 0.5f
        ),
        ImVec2(
            start.x + width,
            start.y + graphHeight * 0.5f
        ),
        IM_COL32(
            35,
            55,
            75,
            100
        ),
        1.0f
    );

    ImVec2 previous(
        start.x,
        start.y + graphHeight * 0.5f
    );

    for (
        int i = 1;
        i <= 75;
        ++i
    )
    {
        const float x =
            float(i) / 75.0f;

        const float t =
            float(
                ImGui::GetTime()
            );

        const float wave =
            std::sin(
                x *
                (
                    45.0f +
                    graphIndex * 7.0f
                ) +
                t *
                (
                    1.4f +
                    graphIndex * 0.22f
                )
            ) +
            std::sin(
                x *
                (
                    95.0f +
                    graphIndex * 9.0f
                ) -
                t * 0.55f
            ) *
            0.32f;

        const ImVec2 current(
            start.x + x * width,
            start.y +
            graphHeight *
            (
                0.5f -
                wave * 0.16f
            )
        );

        dl->AddLine(
            previous,
            current,
            c(graphColor),
            1.0f
        );

        previous =
            current;
    }

    ImGui::Dummy(
        ImVec2(
            width,
            graphHeight + 3.0f
        )
    );

    ImGui::PopID();
}

void NeuroUI::renderRightPanel()
{
    const ImVec2 root =
        ImGui::GetWindowSize();

    const float y =
        HeaderHeight + Gap;

    const float height =
        root.y -
        HeaderHeight -
        BottomHeight -
        SystemHeight -
        Gap * 4.0f;

    ImGui::SetCursorPos(
        ImVec2(
            root.x -
            RightWidth -
            7.0f,
            y
        )
    );

    beginPanel();

    ImGui::BeginChild(
        "##RightPanel",
        ImVec2(
            RightWidth,
            height
        ),
        true
    );

    renderSectionTitle(
        "REGION INFORMATION"
    );

    separator();

    ImGui::Dummy(
        ImVec2(0.0f, 5.0f)
    );

    textNormal(
        "PRIMARY AUDITORY CORTEX"
    );

    textMuted(
        "(Heschl's Gyrus)"
    );

    const ImVec2 miniCenter(
        ImGui::GetWindowPos().x +
        ImGui::GetWindowWidth() *
        0.5f,

        ImGui::GetCursorScreenPos().y +
        63.0f
    );

    drawMiniBrain(
        ImGui::GetWindowDrawList(),
        miniCenter,
        ImVec2(160.0f, 110.0f)
    );

    ImGui::Dummy(
        ImVec2(0.0f, 126.0f)
    );

    textMuted(
        "HEMISPHERE"
    );

    ImGui::SameLine(
        225.0f
    );

    textNormal(
        "Left"
    );

    textMuted(
        "BROADMANN AREA"
    );

    ImGui::SameLine(
        225.0f
    );

    textNormal(
        "41, 42"
    );

    textMuted(
        "VOLUME"
    );

    ImGui::SameLine(
        225.0f
    );

    textNormal(
        "2.34 cm3"
    );

    textMuted(
        "NEURON DENSITY"
    );

    ImGui::SameLine(
        225.0f
    );

    textNormal(
        "Very High"
    );

    textMuted(
        "ACTIVITY LEVEL"
    );

    ImGui::SameLine(
        225.0f
    );

    textNormal(
        "High"
    );

    ImGui::PushStyleColor(
        ImGuiCol_PlotHistogram,
        Cyan
    );

    ImGui::ProgressBar(
        0.78f,
        ImVec2(-1.0f, 4.0f),
        ""
    );

    ImGui::PopStyleColor();

    textMuted(
        "STATUS"
    );

    ImGui::SameLine(
        225.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        Green
    );

    ImGui::TextUnformatted(
        "Active"
    );

    ImGui::PopStyleColor();

    ImGui::Dummy(
        ImVec2(0.0f, 8.0f)
    );

    separator();

    ImGui::Dummy(
        ImVec2(0.0f, 6.0f)
    );

    renderSectionTitle(
        "ACTIVITY MONITOR"
    );

    renderActivityGraph(
        "DELTA (0.5 - 4 Hz)",
        0.12f,
        0
    );

    renderActivityGraph(
        "THETA (4 - 8 Hz)",
        0.28f,
        1
    );

    renderActivityGraph(
        "ALPHA (8 - 12 Hz)",
        0.35f,
        2
    );

    renderActivityGraph(
        "BETA (12 - 30 Hz)",
        0.18f,
        3
    );

    renderActivityGraph(
        "GAMMA (30 - 100 Hz)",
        0.07f,
        4
    );

    ImGui::EndChild();

    endPanel();
}

void NeuroUI::renderRegionCard(
    const char* label,
    int index
)
{
    const bool selected =
        selectedRegion_ ==
        index;

    ImGui::PushID(
        index
    );

    if (selected)
    {
        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(
                0.025f,
                0.080f,
                0.125f,
                1.0f
            )
        );

        ImGui::PushStyleColor(
            ImGuiCol_Border,
            BorderBright
        );

        ImGui::PushStyleVar(
            ImGuiStyleVar_FrameBorderSize,
            1.0f
        );
    }

    if (
        ImGui::Button(
            label,
            ImVec2(
                91.0f,
                79.0f
            )
        )
    )
    {
        selectedRegion_ =
            index;
    }

    if (selected)
    {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }

    ImGui::PopID();
}

void NeuroUI::renderBottomPanel()
{
    const ImVec2 root =
        ImGui::GetWindowSize();

    const float y =
        root.y -
        BottomHeight -
        SystemHeight -
        Gap * 2.0f;

    constexpr float timelineWidth =
        300.0f;

    constexpr float inspectorWidth =
        455.0f;

    ImGui::SetCursorPos(
        ImVec2(7.0f, y)
    );

    beginPanel();

    ImGui::BeginChild(
        "##Timeline",
        ImVec2(
            timelineWidth,
            BottomHeight
        ),
        true
    );

    renderSectionTitle(
        "TIME CONTROLLER"
    );

    textCyan(
        "00:02:37"
    );

    ImGui::SameLine();

    textMuted(
        "/ 00:10:00"
    );

    ImGui::SetNextItemWidth(
        -1.0f
    );

    ImGui::SliderFloat(
        "##Timeline",
        &timeline_,
        0.0f,
        1.0f,
        ""
    );

    ImGui::Dummy(
        ImVec2(0.0f, 12.0f)
    );

    button(
        "<<",
        ImVec2(44.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "<",
        ImVec2(44.0f, 31.0f)
    );

    ImGui::SameLine();

    if (
        button(
            playing_
                ? ">"
                : ">",
            ImVec2(44.0f, 31.0f)
        )
    )
    {
        playing_ =
            !playing_;
    }

    ImGui::SameLine();

    button(
        ">>",
        ImVec2(44.0f, 31.0f)
    );

    ImGui::SameLine();

    button(
        "1x",
        ImVec2(52.0f, 31.0f)
    );

    ImGui::EndChild();

    endPanel();

    const float regionsX =
        timelineWidth +
        Gap * 2.0f;

    const float regionsWidth =
        root.x -
        timelineWidth -
        inspectorWidth -
        Gap * 4.0f;

    ImGui::SetCursorPos(
        ImVec2(
            regionsX,
            y
        )
    );

    beginPanel();

    ImGui::BeginChild(
        "##Regions",
        ImVec2(
            regionsWidth,
            BottomHeight
        ),
        true,
        ImGuiWindowFlags_HorizontalScrollbar
    );

    renderSectionTitle(
        "BRAIN REGIONS"
    );

    renderRegionCard(
        "Frontal\nLobe",
        0
    );

    ImGui::SameLine();

    renderRegionCard(
        "Parietal\nLobe",
        1
    );

    ImGui::SameLine();

    renderRegionCard(
        "Temporal\nLobe",
        2
    );

    ImGui::SameLine();

    renderRegionCard(
        "Occipital\nLobe",
        3
    );

    ImGui::SameLine();

    renderRegionCard(
        "Limbic\nSystem",
        4
    );

    ImGui::SameLine();

    renderRegionCard(
        "Cerebellum",
        5
    );

    ImGui::SameLine();

    renderRegionCard(
        "Brainstem",
        6
    );

    ImGui::EndChild();

    endPanel();

    ImGui::SetCursorPos(
        ImVec2(
            root.x -
            inspectorWidth -
            7.0f,
            y
        )
    );

    beginPanel();

    ImGui::BeginChild(
        "##Inspector",
        ImVec2(
            inspectorWidth,
            BottomHeight
        ),
        true
    );

    renderSectionTitle(
        "NEURON INSPECTOR"
    );

    const ImVec2 imagePos =
        ImGui::GetCursorScreenPos();

    const ImVec2 imageSize(
        220.0f,
        115.0f
    );

    ImDrawList* dl =
        ImGui::GetWindowDrawList();

    dl->AddRectFilled(
        imagePos,
        ImVec2(
            imagePos.x + imageSize.x,
            imagePos.y + imageSize.y
        ),
        IM_COL32(
            4,
            8,
            22,
            245
        ),
        3.0f
    );

    const ImVec2 neuron(
        imagePos.x +
        imageSize.x * 0.52f,

        imagePos.y +
        imageSize.y * 0.50f
    );

    for (
        int i = 0;
        i < 18;
        ++i
    )
    {
        const float angle =
            float(i) /
            18.0f *
            6.28318f;

        const float length =
            37.0f +
            float(i % 5) *
            10.0f;

        const ImVec2 end(
            neuron.x +
            std::cos(angle) *
            length,

            neuron.y +
            std::sin(angle) *
            length
        );

        dl->AddLine(
            neuron,
            end,
            IM_COL32(
                110,
                45,
                235,
                195
            ),
            1.0f
        );

        dl->AddCircleFilled(
            end,
            1.4f,
            IM_COL32(
                180,
                85,
                255,
                220
            )
        );
    }

    dl->AddCircleFilled(
        neuron,
        20.0f,
        IM_COL32(
            120,
            40,
            255,
            28
        )
    );

    dl->AddCircleFilled(
        neuron,
        8.0f,
        IM_COL32(
            175,
            65,
            255,
            170
        )
    );

    dl->AddCircleFilled(
        neuron,
        3.5f,
        IM_COL32(
            245,
            190,
            255,
            255
        )
    );

    ImGui::Dummy(
        imageSize
    );

    ImGui::SameLine();

    ImGui::BeginGroup();

    textMuted(
        "NEURON ID"
    );

    ImGui::SameLine(
        104.0f
    );

    textNormal(
        "#7829-A"
    );

    textMuted(
        "TYPE"
    );

    ImGui::SameLine(
        104.0f
    );

    textNormal(
        "Pyramidal"
    );

    textMuted(
        "STATUS"
    );

    ImGui::SameLine(
        104.0f
    );

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        Green
    );

    ImGui::TextUnformatted(
        "Active"
    );

    ImGui::PopStyleColor();

    textMuted(
        "POTENTIAL"
    );

    ImGui::SameLine(
        104.0f
    );

    textCyan(
        "-65 mV"
    );

    textMuted(
        "FIRING RATE"
    );

    ImGui::SameLine(
        104.0f
    );

    textNormal(
        "24.3 Hz"
    );

    button(
        "INSPECT IN 3D",
        ImVec2(
            180.0f,
            29.0f
        )
    );

    ImGui::EndGroup();

    ImGui::EndChild();

    endPanel();
}

void NeuroUI::renderMetric(
    const char* title,
    const char* value
)
{
    textMuted(
        title
    );

    ImGui::SameLine();

    textNormal(
        value
    );
}

void NeuroUI::renderSystemBar()
{
    const ImVec2 root =
        ImGui::GetWindowSize();

    const float y =
        root.y -
        SystemHeight -
        6.0f;

    ImGui::SetCursorPos(
        ImVec2(7.0f, y)
    );

    beginPanel();

    ImGui::BeginChild(
        "##System",
        ImVec2(
            root.x - 14.0f,
            SystemHeight
        ),
        true,
        ImGuiWindowFlags_NoScrollbar
    );

    ImDrawList* dl =
        ImGui::GetWindowDrawList();

    const ImVec2 p =
        ImGui::GetCursorScreenPos();

    dl->AddCircleFilled(
        ImVec2(
            p.x + 5.0f,
            p.y + 8.0f
        ),
        4.0f,
        c(Green)
    );

    ImGui::SetCursorPosX(
        24.0f
    );

    textMuted(
        "SYSTEM STATUS"
    );

    ImGui::SameLine();

    ImGui::PushStyleColor(
        ImGuiCol_Text,
        Green
    );

    ImGui::TextUnformatted(
        "All Systems Operational"
    );

    ImGui::PopStyleColor();

    ImGui::SameLine(
        220.0f
    );

    renderMetric(
        "MEMORY",
        "7.2 / 15.6 GB"
    );

    ImGui::SameLine(
        430.0f
    );

    renderMetric(
        "CPU",
        "23%"
    );

    ImGui::SameLine(
        555.0f
    );

    renderMetric(
        "GPU",
        "45%"
    );

    ImGui::SameLine(
        680.0f
    );

    renderMetric(
        "STORAGE",
        "892 GB / 2 TB"
    );

    ImGui::SameLine(
        895.0f
    );

    renderMetric(
        "TEMP",
        "42 C"
    );

    ImGui::SameLine(
        1010.0f
    );

    renderMetric(
        "NETWORK",
        "12.4 MB/s"
    );

    const float right =
        ImGui::GetWindowWidth();

    ImGui::SetCursorPos(
        ImVec2(
            right - 116.0f,
            3.0f
        )
    );

    button(
        "EXPORT DATA",
        ImVec2(
            104.0f,
            27.0f
        )
    );

    ImGui::EndChild();

    endPanel();
}