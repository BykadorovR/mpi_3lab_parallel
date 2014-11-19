#ifndef BYKADOROV_ROMAN_GEOMETRY_OF_HULLS_H
#define BYKADOROV_ROMAN_GEOMETRY_OF_HULLS_H
#include <vector>
#include "point.h"
#include "line.h"

class GeometryOfHulls {
 public:
     explicit GeometryOfHulls(int procNum);
     GeometryOfHulls();
     std::vector<int> getSideElems(const std::vector<point> &vertex);
     int getTopElement(const std::vector<point> &vertex, const std::vector<int> &h, line &topLine);
     void quickHull(const std::vector<point> &vertex, std::vector<int> &hull,
     int leftPos, int rightPos, const std::vector<int> &h);
     void quickHull(const std::vector<point> &vertex, std::vector<int> &hull);
     std::vector<int> getSeparatingLine(const std::vector<point> &vertex);
     void distributePointsForProcesses(const std::vector<point> &vertex,
         std::vector<int> &sendVertex, std::vector<int> &sendDispls);
     void distributeLinesForProcesses(const std::vector<line> &vline, std::vector<int> &sendVertex,
         std::vector<int> &sendDispls);
     void initializeTopAndBotSets(const std::vector<point> &vertex, int displs, line &lr,
          std::vector<int> &h1, std::vector<int> &h2);
     void getLinesForProcesses(const std::vector<point> &vertex, const std::vector<int> &h,
          std::vector<line> &vline);
     bool intersection(line l1, line l2, int proc);
     bool nesting(point &vertex, std::vector<line> &vline);

 private:
     int procNum_;
     double fabs(double left);
     bool less (double left, double right);
     bool equal(double left, double right);
     int orientation(point p, point q, point r);
     bool onSegment(point p, point q, point r);
     void getPointsLeftByLine(const std::vector<point> &vertex,  const std::vector<int> &h,
         line &line, int topPos, int lrPos, std::vector<int> &leftSetPoints);
};

#endif // BYKADOROV_ROMAN_GEOMETRY_OF_HULLS_H