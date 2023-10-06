//
// Hello, World! program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2021 by Bill Spitzak and others.
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

#include <FL/Fl.H>
#include <FL/Fl_String.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/filename.H>


#include <filesystem>
namespace fs = std::filesystem;



#if 0

  BOOST_FILESYSTEM_DECL path path::lexically_relative(const path& base) const
  {
    path::iterator b = begin(), e = end(), base_b = base.begin(), base_e = base.end();
    std::pair<path::iterator, path::iterator> mm = detail::mismatch(b, e, base_b, base_e);
    if (mm.first == b && mm.second == base_b)
      return path();
    if (mm.first == e && mm.second == base_e)
      return detail::dot_path();

    std::ptrdiff_t n = 0;
    for (; mm.second != base_e; ++mm.second)
    {
      path const& p = *mm.second;
      if (p == detail::dot_dot_path())
        --n;
      else if (!p.empty() && p != detail::dot_path())
        ++n;
    }
    if (n < 0)
      return path();
    if (n == 0 && (mm.first == e || mm.first->empty()))
      return detail::dot_path();

    path tmp;
    for (; n > 0; --n)
      tmp /= detail::dot_dot_path();
    for (; mm.first != e; ++mm.first)
      tmp /= *mm.first;
    return tmp;
  }


/*
 Returns *this made relative to base.
 First, if root_name() != base.root_name() is true or is_absolute() != base.is_absolute() is true or (!has_root_directory() && base.has_root_directory()) is true or any filename in relative_path() or base.relative_path() can be interpreted as a root-name, returns a default-constructed path.
 Otherwise, first determines the first mismatched element of *this and base as if by auto [a, b] = mismatch(begin(), end(), base.begin(), base.end()), then
 if a == end() and b == base.end(), returns path("."),
 otherwise, define N as the number of nonempty filename elements that are neither dot nor dot-dot in [b, base.end()), minus the number of dot-dot filename elements, If N < 0, returns a default-constructed path,
 otherwise, if N = 0 and a == end() || a->empty(), returns path("."),
 otherwise returns an object composed from
 a default-constructed path() followed by
 N applications of operator/=(path("..")), followed by
 one application of operator/= for each element in the half-open range [a, end()).

 These conversions are purely lexical. They do not check that the paths exist, do not follow symlinks, and do not access the filesystem at all. For symlink-following counterparts of lexically_relative and lexically_proximate, see relative and proximate.
 On Windows, the returned path has backslashes (the preferred separators).
 On POSIX, no filename in a relative path is acceptable as a root-name.


 Overview: Returns *this made relative to base. Treats empty or identical paths as corner cases, not errors. Does not resolve symlinks. Does not first normalize *this or base.

 Remarks: Uses std::mismatch(begin(), end(), base.begin(), base.end()), to determine the first mismatched element of *this and base. Uses operator== to determine if elements match.

 Returns:

 path() if the first mismatched element of *this is equal to begin() or the first mismatched element of base is equal to base.begin(), or

 path(".") if the first mismatched element of *this is equal to end() and the first mismatched element of base is equal to base.end(), or

 An object of class path composed via application of operator/= path("..") for each element in the half-open range [first mismatched element of base, base.end()), and then application of operator/= for each element in the half-open range [first mismatched element of *this, end()).


 \\?\ -> verbatim
 \\.\ -> device
*/


  797  BOOST_FILESYSTEM_DECL path path::lexically_relative(path const& base) const
  798  {
    799      path::iterator b = begin(), e = end(), base_b = base.begin(), base_e = base.end();
    800      std::pair< path::iterator, path::iterator > mm = detail::mismatch(b, e, base_b, base_e);
    801      if (mm.first == b && mm.second == base_b)
      802          return path();
    803      if (mm.first == e && mm.second == base_e)
      804          return detail::dot_path();
    805
    806      std::ptrdiff_t n = 0;
    807      for (; mm.second != base_e; detail::path_algorithms::increment_v4(mm.second))
      808      {
        809          path const& p = *mm.second;
        810          if (detail::path_algorithms::compare_v4(p, detail::dot_dot_path()) == 0)
          811              --n;
        812          else if (!p.empty() && detail::path_algorithms::compare_v4(p, detail::dot_path()) != 0)
          813              ++n;
        814      }
    815      if (n < 0)
      816          return path();
    817      if (n == 0 && (mm.first == e || mm.first->empty()))
      818          return detail::dot_path();
    819
    820      path tmp;
    821      for (; n > 0; --n)
      822          detail::path_algorithms::append_v4(tmp, detail::dot_dot_path());
    823      for (; mm.first != e; detail::path_algorithms::increment_v4(mm.first))
      824          detail::path_algorithms::append_v4(tmp, *mm.first);
    825      return tmp;
    826  }


  157  inline std::pair< path::iterator, path::iterator > mismatch(path::iterator it1, path::iterator it1end, path::iterator it2, path::iterator it2end)
  158  {
    159      for (; it1 != it1end && it2 != it2end && path_algorithms::compare_v4(*it1, *it2) == 0;)
      160      {
        161          path_algorithms::increment_v4(it1);
        162          path_algorithms::increment_v4(it2);
        163      }
    164      return std::make_pair(it1, it2);
    165  }


  BOOST_FILESYSTEM_DECL int path_algorithms::lex_compare_v4
  473  (
        474      path_detail::path_iterator first1, path_detail::path_iterator const& last1,
        475      path_detail::path_iterator first2, path_detail::path_iterator const& last2
        476  )
  477  {
    478      for (; first1 != last1 && first2 != last2;)
      479      {
        480          if (first1->native() < first2->native())
          481              return -1;
        482          if (first2->native() < first1->native())
          483              return 1;
        484          BOOST_ASSERT(first2->native() == first1->native());
        485          path_algorithms::increment_v4(first1);
        486          path_algorithms::increment_v4(first2);
        487      }
    488      if (first1 == last1 && first2 == last2)
      489          return 0;
    490      return first1 == last1 ? -1 : 1;
    491  }
  492

#endif

int main(int argc, char **argv) {

#if 1

  const char *prefix_seg[] = {
    "C:", "c:", "//127.0.0.1/c", "D:", "//?/C:",
  };
  const char *path_seg[] = {
    "/test", "/tesgg", "/Test", "/abc", "/def", "/ghi", "/ÄÖÜ", "/äöü"
  };

  for (int c = 0; c < 10; c++) {

    Fl_String to, base;

//    Windows:
//    if (rand() & 1) to.append(prefix_seg[rand()%5]);
//    if (rand() & 1) base.append(prefix_seg[rand()%5]);

    int i, n = rand() % 6 + 1;
    for (i = 0; i < n; i++) {
      to.append(path_seg[rand()%8]);
    }
    n = rand() % 6 + 1;
    for (i = 0; i < n; i++) {
      base.append(path_seg[rand()%8]);
    }

    if (rand() & 1) to.append("/");
    if (rand() & 1) base.append("/");

    auto rel = fs::path(to.c_str()).lexically_relative(base.c_str());
    if (strcmp(rel.c_str(), fl_filename_relative(to, base).c_str())) {
      printf("Dest: %s\n", to.c_str());
      printf("Base: %s\n", base.c_str());
      printf(" std: %s\n", rel.c_str());
      printf("  FL: %s\n", fl_filename_relative(to, base).c_str());
    }
  }

#else
//  char from[] = "/test/path/folder/file";
//  char base[] = "/test/path_suffix/folder";

  Fl_String   to = "/test/path/folder/file";
  Fl_String base = "/test/path_suffix/folder";

  printf("Dest: %s\n", to.c_str());
  printf("Base: %s\n", base.c_str());
  auto rel = fs::path(to.c_str()).lexically_relative(base.c_str());
  printf(" std: %s\n", rel.c_str());
  printf("  FL: %s\n", fl_filename_relative(to, base).c_str());

#endif

//  char from[] = "/test/path/folder/file";
//  char base[] = "/test/path_suffix/folder";
//  char to[256] = "";
//  fl_filename_relative(to, 256, from, base);
//  printf("%s\n", to);
//
//  printf("%s\n", fs::path(from).lexically_relative(base).c_str());


}
