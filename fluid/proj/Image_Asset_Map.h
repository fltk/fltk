//
// Image Asset Map header file for the Fast Light Tool Kit (FLTK).
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

#ifndef APP_IMAGE_ASSET_MAP_H
#define APP_IMAGE_ASSET_MAP_H

#include <map>
#include <memory>
#include <string>

class Image_Asset;

namespace fld { class Project; }

/**
 \brief Cache of all active Image_Asset objects for a project, keyed by filename.

 Holds weak references so assets are released automatically when no
 Widget_Node holds a shared_ptr to them anymore. Each map is owned by
 one fld::Project and holds a reference to it to avoid using the global Fluid.proj.
 */
class Image_Asset_Map {
  friend class Image_Asset;

  fld::Project& proj_;
  std::map<std::string, std::weak_ptr<Image_Asset>> map_;

public:
  Image_Asset_Map(fld::Project& proj) : proj_(proj) {}

  fld::Project& project() const { return proj_; }

  /// Look up an asset by filename. Returns nullptr if not cached.
  std::shared_ptr<Image_Asset> find(const std::string& name) const;

  /// Return the cached asset for \p name, or load and cache it from disk.
  std::shared_ptr<Image_Asset> find_or_create(const std::string& name);

  void insert(const std::string& name, std::shared_ptr<Image_Asset> asset);
  void erase(const std::string& name);
};

#endif // APP_IMAGE_ASSET_MAP_H
