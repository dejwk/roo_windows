#pragma once

#include <array>
#include <cstring>
#include <memory>

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/application.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/card/flex_card.h"
#include "roo_windows/material3/palette.h"
#include "roo_windows/material3/progress_indicator/progress_indicator.h"
#include "roo_windows/material3/schemes/catalog.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/material3/utilities/dropdown_button.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows::coordinated_palette_example {

/// Compares catalog palettes on M3 controls and borrowed M2 components.
class PalettePreview final : public FlexLayout {
 public:
  /// Builds a scrollable preview borrowing @p app and its mutable @p theme.
  /// Both must outlive the preview. Selection installs coordinated colors only.
  PalettePreview(Application& app, Theme& theme)
      : FlexLayout(app.context(), FlexDirection::kColumn),
        app_(app),
        theme_(theme) {
    ApplicationContext& context = app.context();
    setPadding(Padding(Scaled(8)));
    setGap(Scaled(8));

    // Include every catalog entry, so expanding the import also expands this
    // scrolling menu without duplicating a hand-maintained list of choices.
    // DropdownButton borrows a contiguous pointer table. The catalog's names
    // are static; this member array stays alive until the selector is deleted.
    size_t initial_preset = 0;
    for (size_t i = 0; i < preset_names_.size(); ++i) {
      preset_names_[i] = material3::schemes::kPresets[i].name;
      if (std::strcmp(material3::schemes::kPresets[i].id,
                      "indigo-san-marino") == 0) {
        initial_preset = i;
      }
    }
    auto selector = std::make_unique<material3::DropdownButton>(
        context, preset_names_.data(), preset_names_.size(),
        material3::ButtonVariant::kOutlined);
    preset_ = selector.get();
    preset_->setDensityOverride(
        material3::DensityOverride::Explicit(material3::Density::kMinus2));
    // Choose the starting palette by identity, independently of list ordering.
    preset_->setSelectedIndex(initial_preset);
    preset_->setOnInteractiveChange([this] { installPalette(); });
    add(std::move(selector));

    // Keep both selectors outside the scrolling content so comparing another
    // palette never requires returning to the top of the preview.
    auto mode_row = std::make_unique<FlexLayout>(context, FlexDirection::kRow);
    mode_row->setAlignItems(AlignItems::kCenter);
    mode_row->add(
        std::make_unique<TextLabel>(context, "Dark mode",
                                    material3::text_style_body_medium()),
        {.flex_grow = 1});
    auto mode = std::make_unique<material3::Switch>(context);
    dark_ = mode.get();
    dark_->setOnInteractiveChange([this] { installPalette(); });
    mode_row->add(std::move(mode));
    add(std::move(mode_row));

    auto content =
        std::make_unique<FlexLayout>(context, FlexDirection::kColumn);
    content->setPadding(Padding(Scaled(4)));
    content->setGap(Scaled(12));

    // Explicit card roles resolve their matching on-container text and accent.
    // Generic labels and the borrowed M2 checkbox inherit those resolved
    // colors.
    auto cards = std::make_unique<FlexLayout>(context, FlexDirection::kRow);
    cards->setGap(Scaled(8));
    auto comfort = std::make_unique<material3::FlexCard>(context);
    comfort->setContainerRole(material3::ColorToken::kSecondaryContainer);
    comfort->setPadding(Padding(Scaled(8)));
    comfort->setGap(Scaled(4));
    comfort->add(std::make_unique<TextLabel>(
        context, "Comfort", material3::text_style_title_small()));
    comfort->add(std::make_unique<TextLabel>(
        context, "Secondary", material3::text_style_body_medium()));
    cards->add(std::move(comfort),
               {.flex_grow = 1, .flex_basis = FlexBasis::kZero});
    auto boost = std::make_unique<material3::FlexCard>(context);
    boost->setContainerRole(material3::ColorToken::kTertiaryContainer);
    boost->setPadding(Padding(Scaled(8)));
    boost->setGap(Scaled(4));
    boost->add(std::make_unique<TextLabel>(
        context, "Boost", material3::text_style_title_small()));
    boost->add(std::make_unique<TextLabel>(
        context, "Tertiary", material3::text_style_body_medium()));
    boost->add(std::make_unique<material2::Checkbox>(
        context, material2::Checkbox::OnOffState::kOn));
    cards->add(std::move(boost),
               {.flex_grow = 1, .flex_basis = FlexBasis::kZero});
    content->add(std::move(cards));

    // The filled action uses primary, the tonal action secondaryContainer.
    // Wrapping keeps the actions usable on narrower display boards too.
    auto actions = std::make_unique<FlexLayout>(context, FlexDirection::kRow);
    actions->setFlexWrap(FlexWrap::kWrap);
    actions->setGap(Scaled(4));
    auto start = std::make_unique<material3::Button>(context, "Start");
    auto pause = std::make_unique<material3::Button>(
        context, "Pause", material3::ButtonVariant::kFilledTonal);
    auto disabled = std::make_unique<material3::Button>(context, "Disabled");
    disabled->setEnabled(false);

    auto power = std::make_unique<material3::FlexCard>(context);
    power->setPadding(Padding(Scaled(12)));
    power->setGap(Scaled(8));
    power->add(std::make_unique<TextLabel>(
        context, "Heating power", material3::text_style_title_small()));
    auto slider = std::make_unique<material3::Slider>(
        context, material3::SliderRange{}, 0.65f);
    auto progress =
        std::make_unique<material3::LinearProgressIndicator>(context);
    material3::Slider* slider_ptr = slider.get();
    material3::LinearProgressIndicator* progress_ptr = progress.get();
    progress->setProgress(slider->value());
    slider->setOnInteractiveChange([slider_ptr, progress_ptr] {
      progress_ptr->setProgress(slider_ptr->value());
    });
    start->setOnInteractiveChange([slider_ptr, progress_ptr] {
      slider_ptr->setValue(0.75f);
      progress_ptr->setProgress(0.75f);
    });
    pause->setOnInteractiveChange([slider_ptr, progress_ptr] {
      slider_ptr->setValue(0.0f);
      progress_ptr->setProgress(0.0f);
    });
    actions->add(std::move(start));
    actions->add(std::move(pause));
    actions->add(std::move(disabled));
    content->add(std::move(actions));
    power->add(std::move(slider));
    power->add(std::move(progress));
    content->add(std::move(power));

    auto name =
        std::make_unique<material3::TextField>(context, "Schedule name");
    name->setText("Weekday comfort");
    content->add(std::move(name));
    // This contained M2 button owns its surface, so it uses the coordinated
    // M2 palette rather than the surrounding card's inherited color pair.
    content->add(
        std::make_unique<material2::SimpleButton>(context, "Save schedule"));

    auto viewport =
        std::make_unique<SimpleScrollablePanel>(context, std::move(content));
    viewport_ = viewport.get();
    viewport_->setVerticalScrollBarPresence(
        VerticalScrollBar::Presence::kShownWhenScrolling);
    add(std::move(viewport), {.flex_grow = 1, .flex_basis = FlexBasis::kZero});
    installPalette();
  }

  /// Deletes the selector before releasing its borrowed pointer table.
  ~PalettePreview() override { removeAll(); }

  /// Returns the selector for keyboard/menu navigation or external selection.
  material3::DropdownButton& presetSelector() { return *preset_; }

  /// Returns the light/dark control.
  material3::Switch& darkModeSwitch() { return *dark_; }

  /// Returns the scrolling content viewport.
  SimpleScrollablePanel& preview() { return *viewport_; }

 private:
  // Menu selection commits after detachment, so installation and invalidation
  // happen between frames without repainting under an active transient menu.
  void installPalette() {
    const material3::schemes::ColorSchemePair& colors =
        material3::schemes::kPresets[preset_->selectedIndex()].colors();
    ApplyCoordinatedMaterial3Palette(
        theme_, dark_->isOn() ? colors.dark : colors.light);
    app_.invalidateTheme();
  }

  Application& app_;
  Theme& theme_;
  std::array<const char*, material3::schemes::kPresetCount> preset_names_{};
  material3::DropdownButton* preset_ = nullptr;
  material3::Switch* dark_ = nullptr;
  SimpleScrollablePanel* viewport_ = nullptr;
};

}  // namespace roo_windows::coordinated_palette_example
