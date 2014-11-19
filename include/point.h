#ifndef BYKADOROV_ROMAN_POINT_H
#define BYKADOROV_ROMAN_POINT_H

class point {
 public:
     point();
     point(const double x, const double y);
     point(const point&);
     void setPosition(double x, double y);
     double getX() const;
     double getY() const;
     bool operator ==(const point p) const;
 private:
     double x_;
     double y_;
};

#endif // BYKADOROV_ROMAN_POINT_H