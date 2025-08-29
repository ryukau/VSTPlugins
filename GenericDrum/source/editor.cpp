// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "editor.hpp"
#include "../../lib/pcg-cpp/pcg_random.hpp"
#include "gui/randomizebutton.hpp"
#include "version.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

constexpr const char *wireDidntCollidedText = "Wire didn't collide.";
constexpr const char *membraneDidntCollidedText = "Membrane didn't collide.";

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();
}

ParamValue Editor::getPlainValue(ParamID id)
{
  auto normalized = controller->getParamNormalized(id);
  return controller->normalizedParamToPlain(id, normalized);
}

template<typename ControllerPtr, typename LabelPtr>
inline void resetStatusText(
  ControllerPtr controller, LabelPtr label, Synth::ParameterID::ID id, const char *text)
{
  controller->setParamNormalized(id, 0.0);
  controller->performEdit(id, 0.0);
  if (label.get()) {
    label->setText(text);
    label->setDirty();
  }
}

void Editor::valueChanged(CControl *pControl)
{
  using ID = Synth::ParameterID::ID;

  ParamID id = pControl->getTag();

  if (id != ID::isWireCollided && id != ID::isSecondaryCollided) {
    resetStatusText(
      controller, labelWireCollision, ID::isWireCollided, wireDidntCollidedText);
    resetStatusText(
      controller, labelMembraneCollision, ID::isSecondaryCollided,
      membraneDidntCollidedText);
  }

  ParamValue value = pControl->getValueNormalized();
  controller->setParamNormalized(id, value);
  controller->performEdit(id, value);
}

void Editor::updateUI(ParamID id, ParamValue normalized)
{
  using ID = Synth::ParameterID::ID;

  PlugEditor::updateUI(id, normalized);

  if (labelExternalInputAmplitude.get() && id == ID::externalInputAmplitudeMeter) {
    if (getPlainValue(ID::useExternalInput) == 0) {
      labelExternalInputAmplitude->setText("External input is disabled.");
      labelExternalInputAmplitude->setDirty();
      extInPeakDecibel = -std::numeric_limits<double>::infinity();
    } else {
      const auto decibel
        = 20 * std::log10(getPlainValue(ID::externalInputAmplitudeMeter));
      if (extInPeakDecibel < decibel) {
        extInPeakDecibel = decibel;
        extInPeakHoldCounter = 60; // Bad, because display refresh rate isn't considered.
      }

      std::ostringstream os;
      os.precision(5);
      os << std::fixed << "Ext. Peak: " << extInPeakDecibel << " [dB]";
      labelExternalInputAmplitude->setText(os.str());
      labelExternalInputAmplitude->setDirty();

      if (extInPeakHoldCounter == 0) {
        extInPeakDecibel -= double(0.1);
        if (extInPeakDecibel < -60) {
          extInPeakDecibel = -std::numeric_limits<double>::infinity();
        }
      } else {
        --extInPeakHoldCounter;
      }
    }
  } else if (labelWireCollision.get() && id == ID::isWireCollided) {
    if (getPlainValue(ID::isWireCollided)) {
      labelWireCollision->setText("Wire collided.");
    } else {
      labelWireCollision->setText(wireDidntCollidedText);
    }
    labelWireCollision->setDirty();
  } else if (labelMembraneCollision.get() && id == ID::isSecondaryCollided) {
    if (getPlainValue(ID::isSecondaryCollided)) {
      labelMembraneCollision->setText("Membrane collided.");
    } else {
      labelMembraneCollision->setText(membraneDidntCollidedText);
    }
    labelMembraneCollision->setDirty();
  }
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;
  using Style = Uhhyou::Style;

  const auto top0 = uiMargin;
  const auto left0 = uiMargin;
  const auto left4 = left0 + 1 * groupLabelWidth + 4 * margin;
  const auto left8 = left0 + 2 * groupLabelWidth + 4 * margin + uiMargin;

  // Mix.
  const auto mixTop0 = top0;
  const auto mixTop1 = mixTop0 + 1 * labelY;
  const auto mixTop2 = mixTop0 + 2 * labelY;
  const auto mixTop3 = mixTop0 + 3 * labelY;
  const auto mixTop4 = mixTop0 + 4 * labelY;
  const auto mixTop5 = mixTop0 + 5 * labelY;
  const auto mixTop6 = mixTop0 + 6 * labelY;
  const auto mixTop7 = mixTop0 + 7 * labelY;
  const auto mixTop8 = mixTop0 + 8 * labelY;
  const auto mixTop9 = mixTop0 + 9 * labelY;
  const auto mixTop10 = mixTop0 + 10 * labelY;
  const auto mixTop11 = mixTop0 + 11 * labelY;
  const auto mixLeft0 = left0;
  const auto mixLeft1 = mixLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mixLeft0, mixTop0, groupLabelWidth, labelHeight, uiTextSize, "Mix & Options");

  addLabel(mixLeft0, mixTop1, labelWidth, labelHeight, uiTextSize, "Output [dB]");
  addTextKnob(
    mixLeft1, mixTop1, labelWidth, labelHeight, uiTextSize, ID::outputGain, Scales::gain,
    true, 5);
  addToggleButton(
    mixLeft0, mixTop2, labelWidth, labelHeight, uiTextSize, "Highpass [Hz]",
    ID::safetyHighpassEnable);
  addTextKnob(
    mixLeft1, mixTop2, labelWidth, labelHeight, uiTextSize, ID::safetyHighpassHz,
    Scales::safetyHighpassHz, false, 5);
  addCheckbox(
    mixLeft0, mixTop3, labelWidth, labelHeight, uiTextSize, "2x Sampling",
    ID::overSampling);
  addCheckbox(
    mixLeft1, mixTop3, labelWidth, labelHeight, uiTextSize, "Normalize Gain",
    ID::normalizeGainWrtNoiseLowpassHz);
  addCheckbox(
    mixLeft0, mixTop4, labelWidth, labelHeight, uiTextSize, "Reset Seed at Note-on",
    ID::resetSeedAtNoteOn);
  addCheckbox(
    mixLeft1, mixTop4, labelWidth, labelHeight, uiTextSize, "Prevent Blow Up",
    ID::preventBlowUp);

  addToggleButton(
    mixLeft0, mixTop5, groupLabelWidth, labelHeight, uiTextSize, "Stereo Unison",
    ID::stereoUnison);
  addLabel(mixLeft0, mixTop6, labelWidth, labelHeight, uiTextSize, "Stereo Balance");
  addTextKnob(
    mixLeft1, mixTop6, labelWidth, labelHeight, uiTextSize, ID::stereoBalance,
    Scales::bipolarScale, false, 5);
  addLabel(mixLeft0, mixTop7, labelWidth, labelHeight, uiTextSize, "Stereo Merge");
  addTextKnob(
    mixLeft1, mixTop7, labelWidth, labelHeight, uiTextSize, ID::stereoMerge,
    Scales::defaultScale, false, 5);

  addToggleButton(
    mixLeft0, mixTop8, groupLabelWidth, labelHeight, uiTextSize, "External Input",
    ID::useExternalInput);
  addLabel(mixLeft0, mixTop9, labelWidth, labelHeight, uiTextSize, "External Gain [dB]");
  addTextKnob(
    mixLeft1, mixTop9, labelWidth, labelHeight, uiTextSize, ID::externalInputGain,
    Scales::gain, true, 5);
  addLabel(
    mixLeft0, mixTop10, labelWidth, labelHeight, uiTextSize, "Trigger Threshold [dB]");
  addTextKnob(
    mixLeft1, mixTop10, labelWidth, labelHeight, uiTextSize,
    ID::automaticTriggerThreshold, Scales::gain, true, 5);
  addCheckbox(
    mixLeft0, mixTop11, labelWidth, labelHeight, uiTextSize, "Automatic Trigger",
    ID::useAutomaticTrigger);
  labelExternalInputAmplitude
    = addLabel(mixLeft1, mixTop11, labelWidth, labelHeight, uiTextSize, "Initialized.");

  // Tuning.
  const auto tuningTop0 = top0 + 13 * labelY;
  const auto tuningTop1 = tuningTop0 + 1 * labelY;
  const auto tuningTop2 = tuningTop0 + 2 * labelY;
  const auto tuningTop3 = tuningTop0 + 3 * labelY;
  const auto tuningTop4 = tuningTop0 + 4 * labelY;
  const auto tuningTop5 = tuningTop0 + 5 * labelY;
  const auto tuningLeft0 = left0;
  const auto tuningLeft1 = tuningLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    tuningLeft0, tuningTop0, groupLabelWidth, labelHeight, uiTextSize, "Tuning");

  addLabel(tuningLeft0, tuningTop1, labelWidth, labelHeight, uiTextSize, "Note -> Pitch");
  addTextKnob(
    tuningLeft1, tuningTop1, labelWidth, labelHeight, uiTextSize, ID::notePitchAmount,
    Scales::bipolarScale, false, 5);
  addLabel(tuningLeft0, tuningTop2, labelWidth, labelHeight, uiTextSize, "Semitone");
  addTextKnob(
    tuningLeft1, tuningTop2, labelWidth, labelHeight, uiTextSize, ID::tuningSemitone,
    Scales::semitone, false, 0, -semitoneOffset);
  addLabel(tuningLeft0, tuningTop3, labelWidth, labelHeight, uiTextSize, "Cent");
  addTextKnob(
    tuningLeft1, tuningTop3, labelWidth, labelHeight, uiTextSize, ID::tuningCent,
    Scales::cent, false, 5);
  addLabel(
    tuningLeft0, tuningTop4, labelWidth, labelHeight, uiTextSize,
    "Pitch Bend Range [st.]");
  addTextKnob(
    tuningLeft1, tuningTop4, labelWidth, labelHeight, uiTextSize, ID::pitchBendRange,
    Scales::pitchBendRange, false, 5);
  addLabel(
    tuningLeft0, tuningTop5, labelWidth, labelHeight, uiTextSize, "Slide Time [s]");
  addTextKnob(
    tuningLeft1, tuningTop5, labelWidth, labelHeight, uiTextSize, ID::noteSlideTimeSecond,
    Scales::noteSlideTimeSecond, false, 5);

  // Impact.
  const auto impactTop0 = top0 + 0 * labelY;
  const auto impactTop1 = impactTop0 + 1 * labelY;
  const auto impactTop2 = impactTop0 + 2 * labelY;
  const auto impactTop3 = impactTop0 + 3 * labelY;
  const auto impactTop4 = impactTop0 + 4 * labelY;
  const auto impactLeft0 = left4;
  const auto impactLeft1 = impactLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    impactLeft0, impactTop0, groupLabelWidth, labelHeight, uiTextSize, "Impact");

  addLabel(impactLeft0, impactTop1, labelWidth, labelHeight, uiTextSize, "Seed");
  auto seedTextKnob = addTextKnob(
    impactLeft1, impactTop1, labelWidth, labelHeight, uiTextSize, ID::seed, Scales::seed,
    false, 0);
  if (seedTextKnob) {
    seedTextKnob->sensitivity = 2048.0 / double(1 << 24);
    seedTextKnob->lowSensitivity = 1.0 / double(1 << 24);
  }
  addLabel(
    impactLeft0, impactTop2, labelWidth, labelHeight, uiTextSize, "Noise Decay [s]");
  addTextKnob(
    impactLeft1, impactTop2, labelWidth, labelHeight, uiTextSize, ID::noiseDecaySeconds,
    Scales::noiseDecaySeconds, false, 5);
  addLabel(
    impactLeft0, impactTop3, labelWidth, labelHeight, uiTextSize, "Noise Lowpass [Hz]");
  addTextKnob(
    impactLeft1, impactTop3, labelWidth, labelHeight, uiTextSize, ID::noiseLowpassHz,
    Scales::delayTimeHz, false, 5);
  addLabel(impactLeft0, impactTop4, labelWidth, labelHeight, uiTextSize, "Echo [Hz]");
  addTextKnob(
    impactLeft1, impactTop4, labelWidth, labelHeight, uiTextSize,
    ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz, false, 5);

  // Wire.
  const auto wireTop0 = top0 + 5 * labelY;
  const auto wireTop1 = wireTop0 + 1 * labelY;
  const auto wireTop2 = wireTop0 + 2 * labelY;
  const auto wireTop3 = wireTop0 + 3 * labelY;
  const auto wireTop4 = wireTop0 + 4 * labelY;
  const auto wireTop5 = wireTop0 + 5 * labelY;
  const auto wireTop6 = wireTop0 + 6 * labelY;
  const auto wireTop7 = wireTop0 + 7 * labelY;
  const auto wireLeft0 = left4;
  const auto wireLeft1 = wireLeft0 + labelWidth + 2 * margin;
  addGroupLabel(wireLeft0, wireTop0, groupLabelWidth, labelHeight, uiTextSize, "Wire");

  addLabel(wireLeft0, wireTop1, labelWidth, labelHeight, uiTextSize, "Impact-Wire Mix");
  addTextKnob(
    wireLeft1, wireTop1, labelWidth, labelHeight, uiTextSize, ID::impactWireMix,
    Scales::defaultScale, false, 5);
  addLabel(wireLeft0, wireTop2, labelWidth, labelHeight, uiTextSize, "Membrane-Wire Mix");
  addTextKnob(
    wireLeft1, wireTop2, labelWidth, labelHeight, uiTextSize, ID::membraneWireMix,
    Scales::defaultScale, false, 5);
  addLabel(wireLeft0, wireTop3, labelWidth, labelHeight, uiTextSize, "Frequency [Hz]");
  addTextKnob(
    wireLeft1, wireTop3, labelWidth, labelHeight, uiTextSize, ID::wireFrequencyHz,
    Scales::wireFrequencyHz, false, 5);
  addLabel(wireLeft0, wireTop4, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    wireLeft1, wireTop4, labelWidth, labelHeight, uiTextSize, ID::wireDecaySeconds,
    Scales::wireDecaySeconds, false, 5);
  addLabel(
    wireLeft0, wireTop5, labelWidth, labelHeight, uiTextSize, "Collision Distance");
  addTextKnob(
    wireLeft1, wireTop5, labelWidth, labelHeight, uiTextSize, ID::wireDistance,
    Scales::collisionDistance, false, 5);
  addLabel(wireLeft0, wireTop6, labelWidth, labelHeight, uiTextSize, "Rattle-Squeak Mix");
  addTextKnob(
    wireLeft1, wireTop6, labelWidth, labelHeight, uiTextSize, ID::wireCollisionTypeMix,
    Scales::defaultScale, false, 5);
  labelWireCollision = addLabel(
    wireLeft0, wireTop7, groupLabelWidth, labelHeight, uiTextSize,
    "Wire collision status.");

  // Membrane Tone.
  const auto toneTop0 = top0 + 13 * labelY;
  const auto toneTop1 = toneTop0 + 1 * labelY;
  const auto toneTop2 = toneTop0 + 2 * labelY;
  const auto toneTop3 = toneTop0 + 3 * labelY;
  const auto toneLeft0 = left4;
  const auto toneLeft1 = toneLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    toneLeft0, toneTop0, groupLabelWidth, labelHeight, uiTextSize, "Membrane Tone");

  addLabel(
    toneLeft0, toneTop1, labelWidth, labelHeight, uiTextSize, "Cross Feedback Gain");
  addTextKnob(
    toneLeft1, toneTop1, labelWidth, labelHeight, uiTextSize, ID::crossFeedbackGain,
    Scales::crossFeedbackGain, false, 5);
  addLabel(toneLeft0, toneTop2, labelWidth, labelHeight, uiTextSize, "Consistency");
  addTextKnob(
    toneLeft1, toneTop2, labelWidth, labelHeight, uiTextSize,
    ID::crossFeedbackConsistency, Scales::crossFeedbackConsistency, false, 5);
  addBarBox(
    toneLeft0, toneTop3, barBoxWidth, barBoxHeight, ID::crossFeedbackRatio0, maxFdnSize,
    Scales::defaultScale, "Cross Feedback Ratio");

  // Pitch Texture.
  const auto textureTop0 = top0;
  const auto textureTop1 = textureTop0 + 1 * labelY;
  const auto textureTop2 = textureTop0 + 2 * labelY;
  const auto textureTop3 = textureTop0 + 3 * labelY;
  const auto textureLeft0 = left8;
  const auto textureLeft1 = textureLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    textureLeft0, textureTop0, groupLabelWidth, labelHeight, uiTextSize, "Pitch Texture");

  addLabel(
    textureLeft0, textureTop1, labelWidth, labelHeight, uiTextSize, "Delay Time Spread");
  addTextKnob(
    textureLeft1, textureTop1, labelWidth, labelHeight, uiTextSize, ID::delayTimeSpread,
    Scales::defaultScale, false, 5);
  addLabel(
    textureLeft0, textureTop2, labelWidth, labelHeight, uiTextSize, "BP Cut Spread");
  addTextKnob(
    textureLeft1, textureTop2, labelWidth, labelHeight, uiTextSize, ID::bandpassCutSpread,
    Scales::defaultScale, false, 5);
  addLabel(
    textureLeft0, textureTop3, labelWidth, labelHeight, uiTextSize,
    "Pitch Random [cent]");
  addTextKnob(
    textureLeft1, textureTop3, labelWidth, labelHeight, uiTextSize, ID::pitchRandomCent,
    Scales::pitchRandomCent, false, 5);

  // Pitch Envelope.
  const auto envTop0 = top0 + 4 * labelY;
  const auto envTop1 = envTop0 + 1 * labelY;
  const auto envTop2 = envTop0 + 2 * labelY;
  const auto envTop3 = envTop0 + 3 * labelY;
  const auto envLeft0 = left8;
  const auto envLeft1 = envLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    envLeft0, envTop0, groupLabelWidth, labelHeight, uiTextSize, "Pitch Envelope");

  addLabel(envLeft0, envTop1, labelWidth, labelHeight, uiTextSize, "Attack [s]");
  addTextKnob(
    envLeft1, envTop1, labelWidth, labelHeight, uiTextSize, ID::envelopeAttackSeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop2, labelWidth, labelHeight, uiTextSize, "Decay [s]");
  addTextKnob(
    envLeft1, envTop2, labelWidth, labelHeight, uiTextSize, ID::envelopeDecaySeconds,
    Scales::envelopeSeconds, false, 5);
  addLabel(envLeft0, envTop3, labelWidth, labelHeight, uiTextSize, "Amount [oct]");
  addTextKnob(
    envLeft1, envTop3, labelWidth, labelHeight, uiTextSize, ID::envelopeModAmount,
    Scales::envelopeModAmount, false, 5);

  // Primary Membrane.
  const auto mainTop0 = top0 + 8 * labelY;
  const auto mainTop1 = mainTop0 + 1 * labelY;
  const auto mainTop2 = mainTop0 + 2 * labelY;
  const auto mainTop3 = mainTop0 + 3 * labelY;
  const auto mainTop4 = mainTop0 + 4 * labelY;
  const auto mainTop5 = mainTop0 + 5 * labelY;
  const auto mainLeft0 = left8;
  const auto mainLeft1 = mainLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    mainLeft0, mainTop0, groupLabelWidth, labelHeight, uiTextSize, "Primary Membrane");

  addLabel(mainLeft0, mainTop1, labelWidth, labelHeight, uiTextSize, "Pitch Type");
  addOptionMenu(
    mainLeft1, mainTop1, labelWidth, labelHeight, uiTextSize, ID::pitchType,
    {
      "Harmonic",
      "Harmonic+12",
      "Harmonic*5",
      "Harmonic Cycle(1, 5)",
      "Harmonic Odd",
      "Semitone (1, 2, 7, 9)",
      "Circular Membrane Mode",
      "Prime Number",
      "Octave",
      "- Reserved 09 -",
      "- Reserved 10 -",
      "- Reserved 11 -",
      "- Reserved 12 -",
      "- Reserved 13 -",
      "- Reserved 14 -",
      "- Reserved 15 -",
      "- Reserved 16 -",
      "- Reserved 17 -",
      "- Reserved 18 -",
      "- Reserved 19 -",
      "- Reserved 20 -",
      "- Reserved 21 -",
      "- Reserved 22 -",
      "- Reserved 23 -",
      "- Reserved 24 -",
      "- Reserved 25 -",
      "- Reserved 26 -",
      "- Reserved 27 -",
      "- Reserved 28 -",
      "- Reserved 29 -",
      "- Reserved 30 -",
      "- Reserved 31 -",
      "- Reserved 32 -",
    });
  addLabel(mainLeft0, mainTop2, labelWidth, labelHeight, uiTextSize, "Delay [Hz]");
  addTextKnob(
    mainLeft1, mainTop2, labelWidth, labelHeight, uiTextSize, ID::delayTimeHz,
    Scales::delayTimeHz, false, 5);
  addLabel(
    mainLeft0, mainTop3, labelWidth, labelHeight, uiTextSize,
    "Delay Modulation [sample]");
  addTextKnob(
    mainLeft1, mainTop3, labelWidth, labelHeight, uiTextSize, ID::delayTimeModAmount,
    Scales::delayTimeModAmount, false, 5);
  addLabel(mainLeft0, mainTop4, labelWidth, labelHeight, uiTextSize, "BP Cut [oct]");
  addTextKnob(
    mainLeft1, mainTop4, labelWidth, labelHeight, uiTextSize, ID::bandpassCutRatio,
    Scales::bandpassCutRatio, false, 5);
  addLabel(mainLeft0, mainTop5, labelWidth, labelHeight, uiTextSize, "BP Q");
  addTextKnob(
    mainLeft1, mainTop5, labelWidth, labelHeight, uiTextSize, ID::bandpassQ,
    Scales::bandpassQ, false, 5);

  // Secondary Membrane.
  const auto secondaryTop0 = top0 + 14 * labelY;
  const auto secondaryTop1 = secondaryTop0 + 1 * labelY;
  const auto secondaryTop2 = secondaryTop0 + 2 * labelY;
  const auto secondaryTop3 = secondaryTop0 + 3 * labelY;
  const auto secondaryTop4 = secondaryTop0 + 4 * labelY;
  const auto secondaryTop5 = secondaryTop0 + 5 * labelY;
  const auto secondaryLeft0 = left8;
  const auto secondaryLeft1 = secondaryLeft0 + labelWidth + 2 * margin;
  addGroupLabel(
    secondaryLeft0, secondaryTop0, groupLabelWidth, labelHeight, uiTextSize,
    "Secondary Membrane");

  addLabel(secondaryLeft0, secondaryTop1, labelWidth, labelHeight, uiTextSize, "Mix");
  addTextKnob(
    secondaryLeft1, secondaryTop1, labelWidth, labelHeight, uiTextSize,
    ID::secondaryFdnMix, Scales::defaultScale, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop2, labelWidth, labelHeight, uiTextSize,
    "Pitch Offset [oct]");
  addTextKnob(
    secondaryLeft1, secondaryTop2, labelWidth, labelHeight, uiTextSize,
    ID::secondaryPitchOffset, Scales::bandpassCutRatio, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop3, labelWidth, labelHeight, uiTextSize, "Q Offset [oct]");
  addTextKnob(
    secondaryLeft1, secondaryTop3, labelWidth, labelHeight, uiTextSize,
    ID::secondaryQOffset, Scales::bandpassCutRatio, false, 5);
  addLabel(
    secondaryLeft0, secondaryTop4, labelWidth, labelHeight, uiTextSize,
    "Collision Distance");
  addTextKnob(
    secondaryLeft1, secondaryTop4, labelWidth, labelHeight, uiTextSize,
    ID::secondaryDistance, Scales::collisionDistance, false, 5);
  labelMembraneCollision = addLabel(
    secondaryLeft0, secondaryTop5, groupLabelWidth, labelHeight, uiTextSize,
    "Membrane collision status.");

  // Randomize button.
  const auto randomButtonTop = top0 + 18 * labelY;
  const auto randomButtonLeft = left0 + labelWidth + 2 * margin;
  auto panicButton = new RandomizeButton(
    CRect(
      randomButtonLeft, randomButtonTop, randomButtonLeft + labelWidth,
      randomButtonTop + splashHeight),
    this, 0, "Random", getFont(pluginNameTextSize), palette, this);
  frame->addView(panicButton);

  // Plugin name.
  const auto splashMargin = uiMargin;
  const auto splashTop = top0 + 18 * labelY;
  const auto splashLeft = left0;
  addSplashScreen(
    splashLeft, splashTop, labelWidth, splashHeight, splashMargin, splashMargin,
    defaultWidth - 2 * splashMargin, defaultHeight - 2 * splashMargin, pluginNameTextSize,
    "GenericDrum", false);

  return true;
}

} // namespace Vst
} // namespace Steinberg
