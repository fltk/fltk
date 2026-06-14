//
// Image Asset Map code for the Fast Light Tool Kit (FLTK).
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

#include "proj/Image_Asset_Map.h"
#include "proj/Image_Asset.h"

#include "Fluid.h"
#include "Project.h"

#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>


std::shared_ptr<Image_Asset> Image_Asset_Map::find(const std::string& name) const {
  auto it = map_.find(name);
  if (it != map_.end()) return it->second.lock();
  return nullptr;
}

/**
 \brief Return the cached asset for \p iname, or load and cache it from disk.

 If the asset has already been loaded, it is returned from the cache.
 If not, the file is opened in the project directory, an Image_Asset is
 constructed, and the result is stored in the cache.

 \param iname Filename of the image, relative to the project directory.
 \return The loaded asset, or nullptr if the file cannot be opened or read.
 */
std::shared_ptr<Image_Asset> Image_Asset_Map::find_or_create(const std::string& iname) {
  if (iname.empty()) return nullptr;

  // Return cached entry if still alive.
  auto existing = find(iname);
  if (existing) return existing;

  // Verify the file exists in the project directory.
  proj_.enter_project_dir();
  FILE *f = fl_fopen(iname.c_str(), "rb");
  if (!f) {
    if (Fluid.batch_mode)
      fprintf(stderr, "Can't open image file:\n%s\n%s", iname.c_str(), strerror(errno));
    else
      fl_message("Can't open image file:\n%s\n%s", iname.c_str(), strerror(errno));
    proj_.leave_project_dir();
    return nullptr;
  }
  fclose(f);

  // Construct the asset (loads the image via Fl_Shared_Image while still in project dir).
  std::shared_ptr<Image_Asset> asset(new Image_Asset(iname, *this));
  if (!asset->image_ || !asset->image_->w() || !asset->image_->h()) {
    if (Fluid.batch_mode)
      fprintf(stderr, "Can't read image file:\n%s\nunrecognized image format", iname.c_str());
    else
      fl_message("Can't read image file:\n%s\nunrecognized image format", iname.c_str());
    proj_.leave_project_dir();
    return nullptr;
  }

  insert(iname, asset);
  proj_.leave_project_dir();
  return asset;
}

void Image_Asset_Map::insert(const std::string& name, std::shared_ptr<Image_Asset> asset) {
  map_[name] = asset;
}

void Image_Asset_Map::erase(const std::string& name) {
  map_.erase(name);
}
