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

/**
 \brief Cache of all active Image_Asset objects for a project, keyed by filename.

 Holds weak references so assets are released automatically when no
 Widget_Node holds a shared_ptr to them anymore.
 */
class Image_Asset_Map {
  std::map<std::string, std::weak_ptr<Image_Asset>> map_;
public:
  std::shared_ptr<Image_Asset> find(const std::string& name) const;
  void insert(const std::string& name, std::shared_ptr<Image_Asset> asset);
  void erase(const std::string& name);
};

#endif // APP_IMAGE_ASSET_MAP_H
