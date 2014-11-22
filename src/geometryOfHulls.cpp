#include "geometryOfHulls.h"
#include <iostream>
#include <algorithm>
#include <mpi.h>

GeometryOfHulls::GeometryOfHulls(int procNum) {
    procNum_=procNum;
}

GeometryOfHulls::GeometryOfHulls() {

}

std::vector<int> GeometryOfHulls::getSideElems(const std::vector<point> &vertex) {
	int rightPos = 0;
	int leftPos = 0;
	for (int i=1; i<vertex.size(); ++i) {
		if (less(vertex[i].getX(), vertex[leftPos].getX()))
			leftPos = i;
		else
			if (less(vertex[rightPos].getX(), vertex[i].getX()))
			    rightPos = i;
	}
	std::vector<int> lr (2);
	lr[0] = leftPos;
	lr[1] = rightPos;
	return lr;
}

std::vector<int> GeometryOfHulls::getSeparatingLine(const std::vector<point> &vertex) {
	std::vector<int> lr(2);
	lr = getSideElems(vertex);
	std::cout<<std::endl<<lr[0]<<" LEFT "<<std::endl;
	std::cout<<lr[1]<<" RIGHT "<<std::endl;
	return lr;
}

void GeometryOfHulls::getLinesForProcesses(const std::vector<point> &vertex, const std::vector<int> &h,
    std::vector<line> &vline) {
	for (int i=0; i<h.size(); i++) {
		if (i!=h.size()-1)
		vline[i] = line(vertex[h[i]],vertex[h[i+1]]);
	}
}

void GeometryOfHulls::initializeTopAndBotSets(const std::vector<point> &vertex, int displs,
    line &lr,  std::vector<int> &h1, std::vector<int> &h2) {
	for (int i=0; i<vertex.size(); i++) {
		if (lr.isLeft(vertex[i])) {
		  	h1.push_back(i+displs);
		}
		if (lr.isRight(vertex[i])) {
		    h2.push_back(i+displs);
		}
    }
}

void GeometryOfHulls::quickHull(const std::vector<point> &vertex, std::vector<int> &hull,
    int leftPos, int rightPos, const std::vector<int> &h) {
	if (h.size()==0) {
		hull.push_back(rightPos);
		return;
	}
	line lr(vertex[leftPos],vertex[rightPos]);
    //Находим самую верхнюю левую точку
	int topPos = h[0];
	line topLine = line(vertex[leftPos], vertex[topPos]);
	double maxDist = lr.distanceToPoint(vertex[topPos]);
	for (int i=1; i<h.size(); i++) {
		if (h[i] != leftPos && h[i] != rightPos) {
			double curDist = lr.distanceToPoint(vertex[h[i]]);
			if (equal(maxDist, curDist)) {
				if (topLine.isLeft(vertex[h[i]])) {
					topPos = h[i];
					topLine = line(vertex[leftPos], vertex[topPos]);
				}
			}
			if (less(maxDist, curDist)) {
				maxDist = curDist;
				topPos = h[i];
				topLine = line(vertex[leftPos], vertex[topPos]);
			}
		}
	}
	std::vector<int> S11;
    line LT = line(vertex[leftPos],vertex[topPos]);
    // формируем множество точек, находящихся слева от прямой LT
    getPointsLeftByLine(vertex,h,LT,leftPos, topPos, S11);
    quickHull(vertex, hull, leftPos,topPos, S11);
    std::vector<int> S12;
    line TR = line(vertex[topPos],vertex[rightPos]);
    // формируем множество точек, находящихся слева от прямой TR
    getPointsLeftByLine(vertex, h,TR, topPos, rightPos ,S12);
    quickHull(vertex, hull, topPos, rightPos, S12);

}

bool GeometryOfHulls::less (double left, double right) {
	return !equal(left,right) && left<right;
}

double GeometryOfHulls::fabs(double left) {
    if (left<0)
        return -left;
    return left;
}

const double eps = 1e-8;
bool GeometryOfHulls::equal(double left, double right) {
    return fabs(left-right) <= eps;
}

void GeometryOfHulls::getPointsLeftByLine(const std::vector<point> &vertex, const std::vector<int> &h,
    line &line, int topPos, int lrPos, std::vector<int> &leftSetPoints) {
	for (int i=0; i<h.size(); i++)
    {
    	if (h[i] != topPos && h[i] != lrPos) {
	        if (line.isLeft(vertex[h[i]])) {
	            leftSetPoints.push_back(h[i]);
            }
        }
    }
}

bool GeometryOfHulls::nesting(point &vertex, std::vector<line> &vline){
	if (vline.size() == 0) return false;
    bool isLeft = false;
	for (int i=0; i<vline.size(); i++)
	    if (vline[i].isLeft(vertex))
	        return false;
    return true;
}

bool GeometryOfHulls::intersection(line l1, line l2, int proc) {
	point p1 =l1.l_;
    point q1 =l1.r_;
    point p2 =l2.l_;
    point q2 =l2.r_;
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
    // Общий случай
    if (o1 != o2 && o3 != o4) {
        return true;
    }
    // Специальные случаи
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
    return false;
 }

int GeometryOfHulls::orientation(point p, point q, point r)
{
    double val = (q.getY() - p.getY()) * (r.getX() - q.getX()) -
              (q.getX() - p.getX()) * (r.getY() - q.getY());

    if (val < 0) return 0;

    return (val > 0)? 1: 2;
}

bool GeometryOfHulls::onSegment(point p, point q, point r)
{
    if ((q.getX() - std::max(p.getX(), r.getX())) <= eps  && (q.getX()-std::min(p.getX(), r.getX())>= eps) &&
        (q.getY() -std::max(p.getY(), r.getY())<=eps ) && (q.getY() -std::min(p.getY(), r.getY())) >= eps )
       return true;
    return false;
}