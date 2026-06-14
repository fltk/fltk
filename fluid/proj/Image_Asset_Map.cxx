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


std::shared_ptr<Image_Asset> Image_Asset_Map::find(const std::string& name) const {
  auto it = map_.find(name);
  if (it != map_.end()) return it->second.lock();
  return nullptr;
}

void Image_Asset_Map::insert(const std::string& name, std::shared_ptr<Image_Asset> asset) {
  map_[name] = asset;
}

void Image_Asset_Map::erase(const std::string& name) {
  map_.erase(name);
}
