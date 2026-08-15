#include "CropState.h"

CropState::CropState()
    : m_x(0.0),
      m_y(0.0),
      m_whiteLeftMm(0.0),
      m_whiteRightMm(0.0),
      m_whiteTopMm(0.0),
      m_whiteBottomMm(0.0)
{
}

double CropState::x() const
{
    return m_x;
}

double CropState::y() const
{
    return m_y;
}

void CropState::setPosition(double x, double y)
{
    m_x = x;
    m_y = y;
}

double CropState::whiteLeftMm() const
{
    return m_whiteLeftMm;
}

double CropState::whiteRightMm() const
{
    return m_whiteRightMm;
}

double CropState::whiteTopMm() const
{
    return m_whiteTopMm;
}

double CropState::whiteBottomMm() const
{
    return m_whiteBottomMm;
}

void CropState::setWhiteLeftMm(double value)
{
    m_whiteLeftMm = value;
}

void CropState::setWhiteRightMm(double value)
{
    m_whiteRightMm = value;
}

void CropState::setWhiteTopMm(double value)
{
    m_whiteTopMm = value;
}

void CropState::setWhiteBottomMm(double value)
{
    m_whiteBottomMm = value;
}