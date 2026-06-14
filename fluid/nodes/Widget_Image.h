//
// Widget Image Property header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2026 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

#ifndef FLUID_NODES_WIDGET_IMAGE_H
#define FLUID_NODES_WIDGET_IMAGE_H

#include "proj/Image_Asset.h"
#include "io/Code_Writer.h"
#include "io/Project_Writer.h"

#include <memory>
#include <string>

class Fl_Widget;

/**
 Holds the image data and display options for one image slot of a widget node.

 A Widget_Node carries two of these: \c active_image for the normal (active)
 state and \c inactive_image for the deactivated (grayed-out) state.
 */
class Widget_Image {
public:
  std::string name;                    ///< Image filename or resource name
  std::shared_ptr<Image_Asset> asset;  ///< Loaded image asset, or nullptr

  int bind     = 0; ///< If set, image is bound to the widget in generated code
  int compress = 1; ///< If set, image is compressed when embedded in source
  int scale_w  = 0; ///< Target width for scaling (0 = natural size)
  int scale_h  = 0; ///< Target height for scaling (0 = natural size)

  /// Assign a new asset and apply it to the widget slot.
  /// Pass nullptr for w to skip widget update (e.g., for Window nodes).
  /// Returns true if the asset actually changed.
  bool set(std::shared_ptr<Image_Asset> new_asset, Fl_Widget* w, bool deimage);

  /// Load an asset by name, store both name and asset, and apply to the widget.
  /// Calls storestring() so that undo and the modified flag are updated.
  /// Pass nullptr for w to skip widget update.
  void set(const std::string& new_name, Fl_Widget* w, bool deimage);

  /// Apply the current asset to a widget's image or deimage slot.
  /// Only scales when at least one of scale_w/scale_h is nonzero.
  void apply_to_widget(Fl_Widget* w, bool deimage) const;

  /// Write static image data into the generated source file.
  void write_static(fld::io::Code_Writer& f) const;

  /// Write widget image setup code (e.g., widget->image(img); image->scale(...)).
  void write_code(fld::io::Code_Writer& f, const char* var, bool deimage) const;

  /// Write image properties into a .fl project file.
  void write_properties(fld::io::Project_Writer& f, bool deimage) const;
};

#endif // FLUID_NODES_WIDGET_IMAGE_H
