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


/**
 \brief Look up an asset by filename in the cache.

 Returns the live asset if one is cached under \p name. If a stale entry
 is found (the weak_ptr has expired but the destructor did not erase it),
 the entry is pruned and nullptr is returned.

 \param name Filename key, relative to the project directory.
 \return The cached asset, or nullptr if not found or already released.
 */
std::shared_ptr<Image_Asset> Image_Asset_Map::find(const std::string& name) const {
  auto it = map_.find(name);
  if (it != map_.end()) {
    auto locked = it->second.lock();
    if (locked) return locked;
    map_.erase(it);  // stale entry — destructor must not have run cleanly
  }
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

/**
 \brief Store an asset in the cache under the given filename.

 Saves a weak_ptr so the cache does not prevent the asset from being
 released when all external shared_ptr holders are gone. Called only
 by find_or_create() immediately after a new asset is successfully loaded.

 \param name  Filename key, relative to the project directory.
 \param asset Newly created asset to cache.
 */
void Image_Asset_Map::insert(const std::string& name, std::shared_ptr<Image_Asset> asset) {
  map_[name] = asset;
}

/**
 \brief Remove the cache entry for the given filename.

 Called automatically by Image_Asset::~Image_Asset() when the last
 shared_ptr to an asset is released, keeping the cache free of dead entries.

 \param name Filename key to remove.
 */
void Image_Asset_Map::erase(const std::string& name) {
  map_.erase(name);
}
