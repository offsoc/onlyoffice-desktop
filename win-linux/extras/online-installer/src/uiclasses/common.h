#ifndef COMMON_H
#define COMMON_H


struct Margins {
    Margins();
    Margins(int, int, int, int);
    Margins(const Margins&);
    Margins& operator=(const Margins&);

    int left, top, right, bottom;
};

struct Rect {
    Rect();
    Rect(int, int, int, int);
    Rect(const Rect &rc);
    Rect& operator=(const Rect &rc);

    int x, y, width, height;
};

struct Point {
    Point();
    Point(int, int);
    Point(const Point&);
    Point& operator=(const Point&);

    int x, y;
};

struct Size {
    Size();
    Size(int, int);
    Size(const Size&);
    Size& operator=(const Size&);

    int width, height;
};

enum LayoutDirection : unsigned char {
    LeftToRight = 0,
    RightToLeft
};

#endif // COMMON_H
