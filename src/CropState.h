#pragma once

class CropState
{
public:
    CropState();

    double x() const;
    double y() const;

    void setPosition(double x, double y);

    double whiteLeftMm() const;
    double whiteRightMm() const;
    double whiteTopMm() const;
    double whiteBottomMm() const;

    void setWhiteLeftMm(double value);
    void setWhiteRightMm(double value);
    void setWhiteTopMm(double value);
    void setWhiteBottomMm(double value);

private:
    double m_x;
    double m_y;

    double m_whiteLeftMm;
    double m_whiteRightMm;
    double m_whiteTopMm;
    double m_whiteBottomMm;
};