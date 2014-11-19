#include "point.h"

point::point(double x, double y) : x_(x), y_(y) {
}

point::point() : x_(0), y_(0) {
}

point::point(const point &point) : x_(point.getX()), y_(point.getY()) {
}

void point::setPosition(double x, double y) {
    x_ = x;
	y_ = y;
}

double point::getX() const{
    return x_;
}

double point::getY() const{
    return y_;
}

const double eps = 1e-8;
bool point::operator==(const point p) const {
    if ((p.getX() == x_) && (p.getY() == y_)) return 1;
	return 0;
}