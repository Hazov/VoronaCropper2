#pragma once

class LightState
{
public:
    LightState();

    double curvesValue() const;
    double levelsValue() const;

    void setCurvesValue(double value);
    void setLevelsValue(double value);

private:
    double m_curvesValue;
    double m_levelsValue;
};