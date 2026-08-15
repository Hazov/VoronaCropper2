#include "LightState.h"

LightState::LightState()
    : m_curvesValue(0.0),
      m_levelsValue(0.0)
{
}

double LightState::curvesValue() const
{
    return m_curvesValue;
}

double LightState::levelsValue() const
{
    return m_levelsValue;
}

void LightState::setCurvesValue(double value)
{
    m_curvesValue = value;
}

void LightState::setLevelsValue(double value)
{
    m_levelsValue = value;
}