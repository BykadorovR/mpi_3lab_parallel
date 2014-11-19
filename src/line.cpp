#include "line.h"
#include "GeometryOfHulls.h"
#include <iostream>
#include <cmath>

// (y1-y2)*x + (x2-x1)*y + (x1*y2 - x2*y1) = 0
line::line(const point &l,const point &r) {
	l_ = l;
	r_ = r;
    a = r.getY() - l.getY();
    b = l.getX() - r.getX();
    c = r.getX()*l.getY() - l.getX()*r.getY();
    //std::cout<<a<<" a "<<b<<" b "<<c<<" c "<<std::endl;
}

line::line() {

}

double line::distanceToPoint(const point &point) {
    return fabs(a*point.getX() + b*point.getY() + c)/sqrt((double)a*a + b*b);
}

const double eps = 1e-8;
bool line::isLeft(const point &point) {
    //std::cout<<point.getX()<<" XP "<<l_.getX()<<" XR "<<r_.getX()<<" XL "<<std::endl;
    //std::cout<<point.getY()<<" YP "<<l_.getY()<<" YR "<<r_.getY()<<" YL "<<std::endl;
    if ((point == l_) || (point == r_)) {
        return false;
    }

    double left = a*point.getX() + b*point.getY() + c;
    //std::cout<<left<<" Left "<<std::endl;
	//return ((point.getX()-l_.getX())*(point.getY()-r_.getY())-(point.getY()-l_.getY())*(point.getX()-r_.getX()))<0;
    return left < -eps;
}

bool line::isRight(const point &point) {
    if ((point == l_) || (point == r_)) {
        //std::cout<<point.getX()<<" XP "<<l_.getX()<<" XR "<<r_.getX()<<" XL "<<std::endl;
        //std::cout<<point.getY()<<" YP "<<l_.getY()<<" YR "<<r_.getY()<<" YL "<<std::endl;
        return false;
    }
    double right = a*point.getX() + b*point.getY() + c;
    //std::cout<<right<<" Right "<<std::endl;
		//return ((point.getX()-l_.getX())*(point.getY()-r_.getY())-(point.getY()-l_.getY())*(point.getX()-r_.getX()))>0;
	return right > eps;
}

