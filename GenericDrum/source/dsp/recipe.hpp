// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../parameter.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace Steinberg::Synth {

enum RecipeIndex : size_t {
  full = 0,
  bassDrum = 1,
  snareDrum = 2,
  hiHat = 3,
  lightPercussion = 4,
  echoDrum = 5,
  jaggyConcertBassDrum = 6,
  muffled = 7,
  synthetic = 8,
};

inline double dbToAmp(double dB) { return std::pow(10.0, dB / 20.0); }

inline void rndParam(
  GlobalParameter &param,
  ParameterID::ID id,
  double minNorm,
  double maxNorm,
  std::mt19937_64 &rng)
{
  if (minNorm > maxNorm) std::swap(minNorm, maxNorm);
  if (minNorm == maxNorm) {
    param.value[id]->setFromNormalized(minNorm);
  } else {
    std::uniform_real_distribution<double> dist{minNorm, maxNorm};
    param.value[id]->setFromNormalized(dist(rng));
  }
}

inline void applyRecipe(GlobalParameter &param, size_t recipeIndex, std::mt19937_64 &rng)
{
  using ID = ParameterID::ID;
  std::uniform_real_distribution<double> uniform{0.0, 1.0};

  switch (recipeIndex) {
    case bassDrum: {
      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.01),
        Scales::noiseDecaySeconds.invmap(0.5), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(800.0),
        Scales::delayTimeHz.invmap(1500.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(100.0),
        Scales::delayTimeHz.invmap(1000.0), rng);

      rndParam(param, ID::impactWireMix, 0.95, 1.0, rng);
      rndParam(param, ID::membraneWireMix, 0.0, 0.1, rng);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(100.0),
        Scales::wireFrequencyHz.invmap(1000.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(0.1),
        Scales::wireDecaySeconds.invmap(1.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.01),
        Scales::collisionDistance.invmap(0.1), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.75, 1.0, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-2.0)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-1.0)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.9, 1.0, rng);
      }

      rndParam(param, ID::delayTimeSpread, 0.5, 1.0, rng);
      rndParam(param, ID::bandpassCutSpread, 0.9, 1.0, rng);
      rndParam(
        param, ID::pitchRandomCent, Scales::pitchRandomCent.invmap(0.0),
        Scales::pitchRandomCent.invmap(100.0), rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      param.value[ID::pitchType]->setFromInt(6); // Circular Membrane Mode
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(20.0),
        Scales::delayTimeHz.invmap(100.0), rng);
      rndParam(
        param, ID::delayTimeModAmount, Scales::delayTimeModAmount.invmap(200.0),
        Scales::delayTimeModAmount.invmap(10000.0), rng);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(-3.0),
        Scales::bandpassCutRatio.invmap(0.0), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.01),
        Scales::bandpassQ.invmap(1.0), rng);

      param.value[ID::secondaryFdnMix]->setFromNormalized(0.5);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-1.0),
        Scales::bandpassCutRatio.invmap(1.0), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-3.0),
        Scales::bandpassCutRatio.invmap(0.0), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(0.75), rng);
    } break;

    case snareDrum: {
      param.value[ID::preventBlowUp]->setFromInt(1);

      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.05),
        Scales::noiseDecaySeconds.invmap(0.15), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(50.0),
        Scales::delayTimeHz.invmap(200.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(500.0),
        Scales::delayTimeHz.invmap(4000.0), rng);

      rndParam(param, ID::impactWireMix, 0.75, 1.0, rng);
      param.value[ID::membraneWireMix]->setFromNormalized(0.0);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(200.0),
        Scales::wireFrequencyHz.invmap(300.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(4.0),
        Scales::wireDecaySeconds.invmap(10.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(0.015), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.0, 0.1, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-2.5)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-0.5)), rng);

      param.value[ID::crossFeedbackRatio0]->setFromNormalized(1.0);
      // crossFeedbackRatio1..4 are bypassed

      rndParam(param, ID::delayTimeSpread, 0.0, 1.0, rng);
      rndParam(param, ID::bandpassCutSpread, 0.5, 1.0, rng);
      rndParam(
        param, ID::pitchRandomCent, Scales::pitchRandomCent.invmap(0.0),
        Scales::pitchRandomCent.invmap(500.0), rng);

      // envelopeAttackSeconds, envelopeDecaySeconds, envelopeModAmount are bypassed

      rndParam(
        param, ID::pitchType, Scales::pitchType.invmap(0), Scales::pitchType.invmap(8),
        rng);
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(50.0),
        Scales::delayTimeHz.invmap(150.0), rng);
      rndParam(
        param, ID::delayTimeModAmount, Scales::delayTimeModAmount.invmap(200.0),
        Scales::delayTimeModAmount.invmap(1500.0), rng);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(0.7),
        Scales::bandpassCutRatio.invmap(2.0), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.1),
        Scales::bandpassQ.invmap(0.8), rng);

      rndParam(param, ID::secondaryFdnMix, 0.25, 0.5, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-1.0),
        Scales::bandpassCutRatio.invmap(1.0), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-2.0),
        Scales::bandpassCutRatio.invmap(2.0), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(0.3), rng);
    } break;

    case hiHat: {
      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.1),
        Scales::noiseDecaySeconds.invmap(0.5), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(50.0),
        Scales::delayTimeHz.invmap(100.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(100.0),
        Scales::delayTimeHz.invmap(300.0), rng);

      rndParam(param, ID::impactWireMix, 0.2, 0.5, rng);
      rndParam(param, ID::membraneWireMix, 0.05, 0.15, rng);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(1.0),
        Scales::wireFrequencyHz.invmap(1000.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(1.0),
        Scales::wireDecaySeconds.invmap(2.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(0.3), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.4, 0.6, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-1.0)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-0.2)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.9, 1.0, rng);
      }

      param.value[ID::delayTimeSpread]->setFromNormalized(0.5);
      param.value[ID::bandpassCutSpread]->setFromNormalized(0.5);
      rndParam(param, ID::pitchRandomCent, 0.0, 1.0, rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      rndParam(
        param, ID::pitchType, Scales::pitchType.invmap(0), Scales::pitchType.invmap(8),
        rng);
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(50.0),
        Scales::delayTimeHz.invmap(300.0), rng);
      rndParam(
        param, ID::delayTimeModAmount, Scales::delayTimeModAmount.invmap(0.0),
        Scales::delayTimeModAmount.invmap(5.0), rng);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(4.0),
        Scales::bandpassCutRatio.invmap(7.0), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.05),
        Scales::bandpassQ.invmap(0.12), rng);

      rndParam(param, ID::secondaryFdnMix, 0.45, 0.55, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-0.2),
        Scales::bandpassCutRatio.invmap(0.0), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-0.5),
        Scales::bandpassCutRatio.invmap(0.9), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(0.2), rng);
    } break;

    case lightPercussion: {
      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.05),
        Scales::noiseDecaySeconds.invmap(0.2), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(180.0),
        Scales::delayTimeHz.invmap(240.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(1600.0),
        Scales::delayTimeHz.invmap(3200.0), rng);

      rndParam(param, ID::impactWireMix, 0.75, 1.0, rng);
      param.value[ID::membraneWireMix]->setFromNormalized(0.0);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(80.0),
        Scales::wireFrequencyHz.invmap(120.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(0.1),
        Scales::wireDecaySeconds.invmap(2.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(0.01), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.5, 1.0, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-3.0)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-0.3)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.9, 1.0, rng);
      }

      rndParam(param, ID::delayTimeSpread, 0.0, 0.2, rng);
      rndParam(param, ID::bandpassCutSpread, 0.33, 1.0, rng);
      rndParam(param, ID::pitchRandomCent, 0.0, 1.0, rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      rndParam(
        param, ID::pitchType, Scales::pitchType.invmap(0), Scales::pitchType.invmap(8),
        rng);
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(60.0),
        Scales::delayTimeHz.invmap(400.0), rng);
      param.value[ID::delayTimeModAmount]->setFromNormalized(0.0);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(0.0),
        Scales::bandpassCutRatio.invmap(2.0), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.3),
        Scales::bandpassQ.invmap(1.0), rng);

      rndParam(param, ID::secondaryFdnMix, 0.0, 0.1, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-0.01),
        Scales::bandpassCutRatio.invmap(0.01), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-1.0),
        Scales::bandpassCutRatio.invmap(3.0), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(100.0), rng);
    } break;

    case echoDrum: {
      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.03),
        Scales::noiseDecaySeconds.invmap(0.2), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(60.0),
        Scales::delayTimeHz.invmap(1000.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(2.0),
        Scales::delayTimeHz.invmap(40.0), rng);

      rndParam(param, ID::impactWireMix, 0.1, 0.75, rng);
      param.value[ID::membraneWireMix]->setFromNormalized(0.0);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(1.0),
        Scales::wireFrequencyHz.invmap(80.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(10.0),
        Scales::wireDecaySeconds.invmap(100.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.05),
        Scales::collisionDistance.invmap(0.35), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.5, 1.0, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-1.2)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-0.05)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.0, 1.0, rng);
      }

      param.value[ID::delayTimeSpread]->setFromNormalized(1.0);
      param.value[ID::bandpassCutSpread]->setFromNormalized(1.0);
      rndParam(
        param, ID::pitchRandomCent, Scales::pitchRandomCent.invmap(0.0),
        Scales::pitchRandomCent.invmap(800.0), rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      rndParam(
        param, ID::pitchType, Scales::pitchType.invmap(0), Scales::pitchType.invmap(8),
        rng);
      param.value[ID::delayTimeHz]->setFromNormalized(Scales::delayTimeHz.invmap(55.0));
      rndParam(
        param, ID::delayTimeModAmount, Scales::delayTimeModAmount.invmap(0.0),
        Scales::delayTimeModAmount.invmap(10.0), rng);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(-2.0),
        Scales::bandpassCutRatio.invmap(1.3), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(1.0),
        Scales::bandpassQ.invmap(100.0), rng);

      rndParam(param, ID::secondaryFdnMix, 0.35, 0.75, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-1.2),
        Scales::bandpassCutRatio.invmap(0.6), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-1.5),
        Scales::bandpassCutRatio.invmap(1.8), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(1.0), rng);
    } break;

    case jaggyConcertBassDrum: {
      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.05),
        Scales::noiseDecaySeconds.invmap(0.5), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(40.0),
        Scales::delayTimeHz.invmap(500.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(100.0),
        Scales::delayTimeHz.invmap(6400.0), rng);

      rndParam(param, ID::impactWireMix, 0.0, 1.0, rng);
      param.value[ID::membraneWireMix]->setFromNormalized(0.0);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(30.0),
        Scales::wireFrequencyHz.invmap(800.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(1.0),
        Scales::wireDecaySeconds.invmap(100.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.005),
        Scales::collisionDistance.invmap(2.5), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.0, 1.0, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-0.7)),
        Scales::crossFeedbackGain.invmap(dbToAmp(0.0)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.9, 1.0, rng);
      }

      rndParam(param, ID::delayTimeSpread, 0.0, 1.0, rng);
      rndParam(param, ID::bandpassCutSpread, 0.0, 1.0, rng);
      rndParam(param, ID::pitchRandomCent, 0.0, 1.0, rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      rndParam(
        param, ID::pitchType, Scales::pitchType.invmap(0), Scales::pitchType.invmap(8),
        rng);
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(2.0),
        Scales::delayTimeHz.invmap(40.0), rng);
      rndParam(
        param, ID::delayTimeModAmount, Scales::delayTimeModAmount.invmap(100.0),
        Scales::delayTimeModAmount.invmap(2300.0), rng);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(0.0),
        Scales::bandpassCutRatio.invmap(1.2), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.03),
        Scales::bandpassQ.invmap(0.7), rng);

      rndParam(param, ID::secondaryFdnMix, 0.1, 0.6, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-0.5),
        Scales::bandpassCutRatio.invmap(2.0), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-4.0),
        Scales::bandpassCutRatio.invmap(0.5), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(15.0), rng);
    } break;

    case muffled: {
      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.08),
        Scales::noiseDecaySeconds.invmap(0.5), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(12.0),
        Scales::delayTimeHz.invmap(110.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(80.0),
        Scales::delayTimeHz.invmap(2500.0), rng);

      rndParam(param, ID::impactWireMix, 0.45, 1.0, rng);
      param.value[ID::membraneWireMix]->setFromNormalized(0.0);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(100.0),
        Scales::wireFrequencyHz.invmap(900.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(2.5),
        Scales::wireDecaySeconds.invmap(10.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(0.01),
        Scales::collisionDistance.invmap(0.2), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.0, 1.0, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-2.5)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-0.3)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.9, 1.0, rng);
      }

      rndParam(param, ID::delayTimeSpread, 0.0, 1.0, rng);
      rndParam(param, ID::bandpassCutSpread, 0.0, 1.0, rng);
      rndParam(
        param, ID::pitchRandomCent, Scales::pitchRandomCent.invmap(0.0),
        Scales::pitchRandomCent.invmap(100.0), rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      param.value[ID::pitchType]->setFromInt(6); // Circular Membrane Mode
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(25.0),
        Scales::delayTimeHz.invmap(200.0), rng);
      rndParam(
        param, ID::delayTimeModAmount, Scales::delayTimeModAmount.invmap(0.0),
        Scales::delayTimeModAmount.invmap(300.0), rng);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(-2.0),
        Scales::bandpassCutRatio.invmap(0.5), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.6),
        Scales::bandpassQ.invmap(2.0), rng);

      rndParam(param, ID::secondaryFdnMix, 0.35, 0.55, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-1.0),
        Scales::bandpassCutRatio.invmap(1.0), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-2.5),
        Scales::bandpassCutRatio.invmap(0.0), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.0),
        Scales::collisionDistance.invmap(3.0), rng);
    } break;

    case synthetic: {
      param.value[ID::preventBlowUp]->setFromInt(1);

      rndParam(param, ID::seed, 0.0, 1.0, rng);
      rndParam(
        param, ID::noiseDecaySeconds, Scales::noiseDecaySeconds.invmap(0.01),
        Scales::noiseDecaySeconds.invmap(0.5), rng);
      rndParam(
        param, ID::noiseLowpassHz, Scales::delayTimeHz.invmap(70.0),
        Scales::delayTimeHz.invmap(1800.0), rng);
      rndParam(
        param, ID::noiseAllpassMaxTimeHz, Scales::delayTimeHz.invmap(2.0),
        Scales::delayTimeHz.invmap(4500.0), rng);

      rndParam(param, ID::impactWireMix, 0.7, 1.0, rng);
      param.value[ID::membraneWireMix]->setFromNormalized(0.0);
      rndParam(
        param, ID::wireFrequencyHz, Scales::wireFrequencyHz.invmap(1.0),
        Scales::wireFrequencyHz.invmap(1000.0), rng);
      rndParam(
        param, ID::wireDecaySeconds, Scales::wireDecaySeconds.invmap(0.3),
        Scales::wireDecaySeconds.invmap(100.0), rng);
      rndParam(
        param, ID::wireDistance, Scales::collisionDistance.invmap(10.0),
        Scales::collisionDistance.invmap(100.0), rng);
      rndParam(param, ID::wireCollisionTypeMix, 0.5, 1.0, rng);

      rndParam(
        param, ID::crossFeedbackGain, Scales::crossFeedbackGain.invmap(dbToAmp(-1.0)),
        Scales::crossFeedbackGain.invmap(dbToAmp(-0.03)), rng);
      for (size_t i = 0; i < maxFdnSize; ++i) {
        rndParam(param, ID(ID::crossFeedbackRatio0 + i), 0.0, 1.0, rng);
      }

      rndParam(param, ID::delayTimeSpread, 0.0, 1.0, rng);
      rndParam(param, ID::bandpassCutSpread, 0.0, 1.0, rng);
      rndParam(param, ID::pitchRandomCent, 0.0, 1.0, rng);

      // envelopeAttackSeconds & envelopeDecaySeconds are bypassed
      param.value[ID::envelopeModAmount]->setFromNormalized(0.0);

      rndParam(
        param, ID::pitchType, Scales::pitchType.invmap(0), Scales::pitchType.invmap(6),
        rng);
      rndParam(
        param, ID::delayTimeHz, Scales::delayTimeHz.invmap(50.0),
        Scales::delayTimeHz.invmap(110.0), rng);
      param.value[ID::delayTimeModAmount]->setFromNormalized(0.0);
      rndParam(
        param, ID::bandpassCutRatio, Scales::bandpassCutRatio.invmap(0.0),
        Scales::bandpassCutRatio.invmap(2.5), rng);
      rndParam(
        param, ID::bandpassQ, Scales::bandpassQ.invmap(0.01),
        Scales::bandpassQ.invmap(0.5), rng);

      rndParam(param, ID::secondaryFdnMix, 0.25, 0.65, rng);
      rndParam(
        param, ID::secondaryPitchOffset, Scales::bandpassCutRatio.invmap(-1.0),
        Scales::bandpassCutRatio.invmap(1.0), rng);
      rndParam(
        param, ID::secondaryQOffset, Scales::bandpassCutRatio.invmap(-2.5),
        Scales::bandpassCutRatio.invmap(0.5), rng);
      rndParam(
        param, ID::secondaryDistance, Scales::collisionDistance.invmap(0.01),
        Scales::collisionDistance.invmap(100.0), rng);
    } break;

    case full:
    default: {
      for (auto id : ParameterID::randomizedIDs) {
        if (id == ID::pitchType) {
          rndParam(
            param, id, Scales::pitchType.invmap(0), Scales::pitchType.invmap(8), rng);
        } else if (id == ID::preventBlowUp) {
          // preventBlowUp remains untouched in Full recipe
        } else {
          param.value[id]->setFromNormalized(uniform(rng));
        }
      }
    } break;
  }
}

} // namespace Steinberg::Synth
