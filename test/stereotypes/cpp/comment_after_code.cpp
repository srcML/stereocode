// stereocode -m moves each element after other code on its line below its "// @stereotype" comment, as in this file

// @stereotype data-class
class Point {
    int x; int y;
public:
    // @stereotype get
    int getX() const { return x; }
    // @stereotype get
    int getY() const { return y; }
    /* Sets x
       to a value */
       // @stereotype set
       void setX(int value) { x = value; }
};

// @stereotype data-provider data-struct small-struct
struct Size { int width;
// @stereotype get
int getWidth() const { return width; } };

int counter;
// @stereotype global-command
int next() { return ++counter; }
