//
// Widget Image Property implementation for the Fast Light Tool Kit (FLTK).
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

#include "nodes/Widget_Image.h"

#include "Fluid.h"
#include "Project.h"
#include "nodes/Node.h"

#include <FL/Fl_Widget.H>

/**
 Assign a new image asset and optionally apply it to the widget.

 Returns true if the asset actually changed; the caller is then responsible for
 calling Widget_Node::redraw() when appropriate.  Passing nullptr for \p w skips
 the widget update (use this for Window-type nodes).
 */
bool Widget_Image::set(std::shared_ptr<Image_Asset> new_asset, Fl_Widget* w, bool deimage) {
  if (new_asset == asset) return false;
  asset = new_asset;
  if (w) apply_to_widget(w, deimage);
  return true;
}

/**
 Load an image asset by name, store both name and asset, and optionally apply to the widget.

 Delegates name storage to storestring() so that undo checkpointing and the project
 modified flag are handled automatically.  If the name is unchanged storestring()
 returns early and no widget update or redraw is needed.  Passing nullptr for \p w
 skips the widget update (use this for Window-type nodes).
 */
void Widget_Image::set(const std::string& new_name, Fl_Widget* w, bool deimage) {
  if (!storestring(new_name, name)) return;
  asset = Fluid.proj.image_assets.find_or_create(name);
  if (w) apply_to_widget(w, deimage);
}

/**
 Apply the current image asset to the widget's active or inactive image slot.

 Sets either \c widget->image() (deimage=false) or \c widget->deimage() (deimage=true)
 to the loaded \c Fl_Image from the asset, then scales it when at least one of
 \c scale_w or \c scale_h is nonzero.  Passing nullptr or a failed asset clears the slot.
 */
void Widget_Image::apply_to_widget(Fl_Widget* w, bool deimage) const {
  Fl_Image* fl_img = asset ? asset->image() : nullptr;
  if (deimage)
    w->deimage(fl_img);
  else
    w->image(fl_img);
  if (fl_img && (scale_w || scale_h)) {
    int iw = scale_w > 0 ? scale_w : fl_img->data_w();
    int ih = scale_h > 0 ? scale_h : fl_img->data_h();
    fl_img->scale(iw, ih, 0, 1);
  }
}

/**
 Write the static image initializer into the generated source file, if needed.

 Delegates to Image_Asset::write_static() using this slot's \c compress setting.
 Does nothing when no asset is loaded or the asset has already been written.
 */
void Widget_Image::write_static(fld::io::Code_Writer& f) const {
  if (asset && !f.c_contains(asset.get()))
    asset->write_static(f, compress);
}

/**
 Write the per-widget image setup code into the generated source file.

 Emits the call to widget->image() or widget->deimage(), and, when at least one
 of \c scale_w or \c scale_h is nonzero, the matching \c ->scale() call.
 Does nothing when no asset is loaded.
 */
void Widget_Image::write_code(fld::io::Code_Writer& f, const char* var, bool deimage) const {
  if (!asset) return;
  asset->write_code(f, bind, var, deimage ? 1 : 0);
  if (scale_w || scale_h) {
    const char* getter = deimage ? "deimage" : "image";
    f.write_c("%s%s->%s()->scale(", f.indent(), var, getter);
    if (scale_w > 0)
      f.write_c("%d, ", scale_w);
    else
      f.write_c("%s->%s()->data_w(), ", var, getter);
    if (scale_h > 0)
      f.write_c("%d, 0, 1);\n", scale_h);
    else
      f.write_c("%s->%s()->data_h(), 0, 1);\n", var, getter);
  }
}

/**
 Write the image-related properties into a .fl project file.

 Uses the "image"/"deimage" keyword family for the active/inactive slot respectively.
 Writes nothing when \c name is empty; omits the scale and bind entries when they
 hold their default values.
 */
void Widget_Image::write_properties(fld::io::Project_Writer& f, bool deimage) const {
  if (!name.empty()) {
    if (scale_w || scale_h)
      f.write_string(deimage ? "scale_deimage {%d %d}" : "scale_image {%d %d}", scale_w, scale_h);
    f.write_string(deimage ? "deimage" : "image");
    f.write_word(name);
    f.write_string(deimage ? "compress_deimage %d" : "compress_image %d", compress);
  }
  if (bind)
    f.write_string(deimage ? "bind_deimage 1" : "bind_image 1");
}
