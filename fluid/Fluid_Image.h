// Fluid_Image.H

// This class stores the image labels for widgets in fluid.  This is
// not a class in fltk itself, and this will produce different types of
// code depending on what the image type is.  There are private subclasses
// in Fluid_Image.C for each type of image format.  Right now only xpm
// files are supported.

class Fluid_Image {
  const char *name_;
  int refcount;
protected:
  Fluid_Image(const char *name); // no public constructor
  virtual ~Fluid_Image(); // no public destructor
public:
  int written;
  static Fluid_Image* find(const char *);
  void decrement(); // reference counting & automatic free
  void increment();
  virtual void label(Fl_Widget *) = 0; // set the label of this widget
  virtual void write_static() = 0;
  virtual void write_code() = 0;
  const char *name() const {return name_;}
};

// pop up file chooser and return a legal image selected by user,
// or zero for any errors:
Fluid_Image *ui_find_image(const char *);
