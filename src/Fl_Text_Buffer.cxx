//
// Copyright 2001-2023 by Bill Spitzak and others.
// Original code Copyright Mark Edel.  Permission to distribute under
// the LGPL for the FLTK library granted by Mark Edel.
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

#include <stdio.h>
#include <stdlib.h>
#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>
#include "flstring.h"
#include <ctype.h>
#include <FL/Fl.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/fl_ask.H>


/*
 This file is based on a port of NEdit to FLTK many years ago. NEdit at that
 point was already stretched beyond the task it was designed for which explains
 why the source code is sometimes pretty convoluted. It still is a very useful
 widget for FLTK, and we are thankful that the nedit team allowed us to
 integrate their code.

 With the introduction of Unicode and UTF-8, Fl_Text_... has to go into a whole
 new generation of code. Originally designed for monospaced fonts only, many
 features make less sense in the multibyte and multiwidth world of UTF-8.

 Columns are a good example. There is simply no such thing. The new Fl_Text_...
 widget converts columns to pixels by multiplying them with the average
 character width for a given font.

 Rectangular selections were rarely used (if at all) and make little sense when
 using variable width fonts. They have been removed.

 Using multiple spaces to emulate tab stops has been replaced by pixel counting
 routines. They are slower, but give the expected result for proportional fonts.

 And constantly recalculating character widths is just much too expensive. Lines
 of text are now subdivided into blocks of text which are measured at once
 instead of individual characters.
 */


#ifndef min

static int max(int i1, int i2)
{
  return i1 >= i2 ? i1 : i2;
}

static int min(int i1, int i2)
{
  return i1 <= i2 ? i1 : i2;
}

#endif

/*
 Undo/Redo is handled with Fl_Text_Undo_Action. The names of the class members
 relate to the original action.

 Deleting text will store the number of bytes deleted in `undocut`, and store
 the deleted text in `undobuffer`. `undoat` is the insertion position.

 Inserting text will store the number of bytes inserted in `undoinsert` and
 `undoat` will point after the inserted text.

 If text is deleted first and then text is inserted at the same position, it's
 called a yankcut, and the number of bytes that were deleted is stored in
 `undoyankcut`, again storing the deleted text in `undobuffer`.

 If an undo action is run, text is deleted and inserted via the normal
 Fl_Text_Editor methods, generating the inverse undo action (redo) in mUndo.
 */
class Fl_Text_Undo_Action {
public:
  Fl_Text_Undo_Action() :
    undobuffer(NULL),
    undobufferlength(0),
    undoat(0),
    undocut(0),
    undoinsert(0),
    undoyankcut(0)
  { }
  ~Fl_Text_Undo_Action() {
    if (undobuffer)
      ::free(undobuffer);
  }

  char *undobuffer;
  int undobufferlength;
  int undoat;              // points after insertion
  int undocut;             // number of characters deleted there
  int undoinsert;          // number of characters inserted
  int undoyankcut;         // length of valid contents of buffer, even if undocut=0

  /*
   Resize the undo buffer to match at least the requested size.
   */
  void undobuffersize(int n)
  {
    if (n > undobufferlength) {
      undobufferlength = n + 128;
      undobuffer = (char *)realloc(undobuffer, undobufferlength);
    }
  }

  void clear() {
    undocut = undoinsert = 0;
  }

  bool empty() const {
    return (!undocut && !undoinsert);
  }
};

/*
 Undo events are stored in a Last In - First Out stack.

 Any insertion or deletion of text will either add to the current undo event
 in mUndo, or generate a new undo event if cursor positions are not consecutive.
 The previously current undo event will then be pushed to the undo list and
 the redo event list is purged.

 If the user calls undo(), the current undo event in mUndo will be run,
 generating a matching redo event in mUndo. The redo event is then pushed into
 the redo list, and the next undo event is popped from the undo list and made
 current.

 A list can be locked to be protected from purging while running an undo event.
 */
class Fl_Text_Undo_Action_List {
  Fl_Text_Undo_Action** list_;
  int list_size_;
  int list_capacity_;
  bool locked_;
public:
  Fl_Text_Undo_Action_List() :
  list_(NULL),
  list_size_(0),
  list_capacity_(0),
  locked_(false)
  { }

  ~Fl_Text_Undo_Action_List() {
    unlock();
    clear();
  }

  int size() const {
    return list_size_;
  }

  void push(Fl_Text_Undo_Action* action) {
    if (list_size_ == list_capacity_) {
      list_capacity_ += 25;
      list_ = (Fl_Text_Undo_Action**)realloc(list_, list_capacity_ * sizeof(Fl_Text_Undo_Action*));
    }
    list_[list_size_++] = action;
  }

  Fl_Text_Undo_Action* pop() {
    if (list_size_ > 0) {
      return list_[--list_size_];
    } else {
      return NULL;
    }
  }

  void clear() {
    if (locked_) return;
    if (list_) {
      for (int i=0; i<list_size_; i++) {
        delete list_[i];
      }
      ::free(list_);
    }
    list_ = NULL;
    list_size_ = 0;
    list_capacity_ = 0;
  }

  void lock() { locked_ = true; }
  void unlock() { locked_ = false; }
};


static void def_transcoding_warning_action(Fl_Text_Buffer *text)
{
  fl_alert("%s", text->file_encoding_warning_message);
}

/*
 Initialize all variables.
 */
Fl_Text_Buffer::Fl_Text_Buffer(int requestedSize, int preferredGapSize)
{
  mLength = 0;
  mPreferredGapSize = preferredGapSize;
  mBuf = (char *) malloc(requestedSize + mPreferredGapSize);
  mGapStart = 0;
  mGapEnd = requestedSize + mPreferredGapSize;
  mTabDist = 8;
  mPrimary.mSelected = 0;
  mPrimary.mStart = mPrimary.mEnd = 0;
  mSecondary.mSelected = 0;
  mSecondary.mStart = mSecondary.mEnd = 0;
  mHighlight.mSelected = 0;
  mHighlight.mStart = mHighlight.mEnd = 0;
  mModifyProcs = NULL;
  mCbArgs = NULL;
  mNModifyProcs = 0;
  mNPredeleteProcs = 0;
  mPredeleteProcs = NULL;
  mPredeleteCbArgs = NULL;
  mCursorPosHint = 0;
  mCanUndo = 1;
  mUndo = new Fl_Text_Undo_Action();
  mUndoList = new Fl_Text_Undo_Action_List();
  mRedoList = new Fl_Text_Undo_Action_List();
  input_file_was_transcoded = 0;
  transcoding_warning_action = def_transcoding_warning_action;
}


/*
 Free all resources.
 */
Fl_Text_Buffer::~Fl_Text_Buffer()
{
  free(mBuf);
  if (mNModifyProcs != 0) {
    delete[]mModifyProcs;
    delete[]mCbArgs;
  }
  if (mNPredeleteProcs > 0) {
    delete[] mPredeleteProcs;
    delete[] mPredeleteCbArgs;
  }
  delete mUndo;
  delete mUndoList;
  delete mRedoList;
}


/*
 This function copies verbose whatever is in front and after the gap into a
 single buffer.
 */
char *Fl_Text_Buffer::text() const {
  char *t = (char *) malloc(mLength + 1);
  memcpy(t, mBuf, mGapStart);
  memcpy(t+mGapStart, mBuf+mGapEnd, mLength - mGapStart);
  t[mLength] = '\0';
  return t;
}


/*
 Set the text buffer to a new string.
 */
void Fl_Text_Buffer::text(const char *t)
{
  IS_UTF8_ALIGNED(t)

  // if t is null then substitute it with an empty string
  // then don't return so that internal cleanup can happen
  if (!t) t="";

  call_predelete_callbacks(0, length());

  /* Save information for redisplay, and get rid of the old buffer */
  const char *deletedText = text();
  int deletedLength = mLength;
  free((void *) mBuf);

  /* Start a new buffer with a gap of mPreferredGapSize at the end */
  int insertedLength = (int) strlen(t);
  mBuf = (char *) malloc(insertedLength + mPreferredGapSize);
  mLength = insertedLength;
  mGapStart = insertedLength;
  mGapEnd = mGapStart + mPreferredGapSize;
  memcpy(mBuf, t, insertedLength);

  /* Zero all of the existing selections */
  update_selections(0, deletedLength, 0);

  /* Call the saved display routine(s) to update the screen */
  call_modify_callbacks(0, deletedLength, insertedLength, 0, deletedText);
  free((void *) deletedText);

  if (mCanUndo) {
    mUndo->clear();
    mUndoList->clear();
    mRedoList->clear();
  }
}


/*
 Creates a range of text to a new buffer and copies verbose from around the gap.
 */
char *Fl_Text_Buffer::text_range(int start, int end) const {
  IS_UTF8_ALIGNED2(this, (start))
  IS_UTF8_ALIGNED2(this, (end))

  char *s = NULL;

  /* Make sure start and end are ok, and allocate memory for returned string.
   If start is bad, return "", if end is bad, adjust it. */
  if (start < 0 || start > mLength)
  {
    s = (char *) malloc(1);
    s[0] = '\0';
    return s;
  }
  if (end < start) {
    int temp = start;
    start = end;
    end = temp;
  }
  if (end > mLength)
    end = mLength;
  int copiedLength = end - start;
  s = (char *) malloc(copiedLength + 1);

  /* Copy the text from the buffer to the returned string */
  if (end <= mGapStart) {
    memcpy(s, mBuf + start, copiedLength);
  } else if (start >= mGapStart) {
    memcpy(s, mBuf + start + (mGapEnd - mGapStart), copiedLength);
  } else {
    int part1Length = mGapStart - start;
    memcpy(s, mBuf + start, part1Length);
    memcpy(s + part1Length, mBuf + mGapEnd, copiedLength - part1Length);
  }
  s[copiedLength] = '\0';
  return s;
}

/*
 Return a UCS-4 character at the given index.
 Pos must be at a character boundary.
 */
unsigned int Fl_Text_Buffer::char_at(int pos) const {
  if (pos < 0 || pos >= mLength)
    return '\0';

  IS_UTF8_ALIGNED2(this, (pos))

  const char *src = address(pos);
  return fl_utf8decode(src, 0, 0);
}


/*
 Return the raw byte at the given index.
 This function ignores all unicode encoding.
 */
char Fl_Text_Buffer::byte_at(int pos) const {
  if (pos < 0 || pos >= mLength)
    return '\0';
  const char *src = address(pos);
  return *src;
}


/*
 Insert some text at the given index.
 Pos must be at a character boundary.
*/
void Fl_Text_Buffer::insert(int pos, const char *text, int insertedLength)
{
  IS_UTF8_ALIGNED2(this, (pos))
  IS_UTF8_ALIGNED(text)

  /* check if there is actually any text */
  if (!text || !*text)
    return;

  /* if pos is not contiguous to existing text, make it */
  if (pos > mLength)
    pos = mLength;
  if (pos < 0)
    pos = 0;

  /* Even if nothing is deleted, we must call these callbacks */
  call_predelete_callbacks(pos, 0);

  /* insert and redisplay */
  int nInserted = insert_(pos, text, insertedLength);
  mCursorPosHint = pos + nInserted;
  IS_UTF8_ALIGNED2(this, (mCursorPosHint))
  call_modify_callbacks(pos, 0, nInserted, 0, NULL);
}


/**
 Can be used by subclasses that need their own printf() style functionality.

 \note The expanded string is currently limited to 1024 characters.
 \param[in] fmt is a printf format string for the message text.
 \param[in] ap is a va_list created by va_start() and closed with va_end(),
               which the caller is responsible for handling.
*/
void Fl_Text_Buffer::vprintf(const char *fmt, va_list ap) {
  char buffer[1024];    // XXX: 1024 should be user configurable
  ::vsnprintf(buffer, 1024, fmt, ap);
  buffer[1024-1] = 0;   // XXX: MICROSOFT
  append(buffer);
}


/**
 Appends printf formatted messages to the end of the buffer.
 Example:
 \code
 #include <FL/Fl_Text_Display.H>
 int main(..) {
     :
     // Create a text display widget and assign it a text buffer
     Fl_Text_Display *tdsp = new Fl_Text_Display(..);
     Fl_Text_Buffer *tbuf = new Fl_Text_Buffer();
     tdsp->buffer(tbuf);
     :
     // Append three lines of formatted text to the buffer
     tbuf->printf("The current date is: %s.\nThe time is: %s\n", date_str, time_str);
     tbuf->printf("The current PID is %ld.\n", (long)getpid());
     :
 \endcode
 \note The expanded string is currently limited to 1024 characters.
 \param[in] fmt is a printf format string for the message text.
*/
void Fl_Text_Buffer::printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  Fl_Text_Buffer::vprintf(fmt,ap);
  va_end(ap);
}


/*
 Replace a range of text with new text.
 Start and end must be at a character boundary.
*/
void Fl_Text_Buffer::replace(int start, int end, const char *text, int insertedLength)
{
  // Range check...
  if (!text)
    return;
  if (start < 0)
    start = 0;
  if (end > mLength)
    end = mLength;

  IS_UTF8_ALIGNED2(this, (start))
  IS_UTF8_ALIGNED2(this, (end))
  IS_UTF8_ALIGNED(text)

  call_predelete_callbacks(start, end - start);
  const char *deletedText = text_range(start, end);
  remove_(start, end);
  int nInserted = insert_(start, text, insertedLength);
  mCursorPosHint = start + nInserted;
  call_modify_callbacks(start, end - start, nInserted, 0, deletedText);
  free((void *) deletedText);
}


/*
 Remove a range of text.
 Start and End must be at a character boundary.
*/
void Fl_Text_Buffer::remove(int start, int end)
{
  /* Make sure the arguments make sense */
  if (start > end) {
    int temp = start;
    start = end;
    end = temp;
  }
  if (start > mLength)
    start = mLength;
  if (start < 0)
    start = 0;
  if (end > mLength)
    end = mLength;
  if (end < 0)
    end = 0;

  IS_UTF8_ALIGNED2(this, (start))
  IS_UTF8_ALIGNED2(this, (end))

  if (start == end)
    return;

  call_predelete_callbacks(start, end - start);
  /* Remove and redisplay */
  const char *deletedText = text_range(start, end);
  remove_(start, end);
  mCursorPosHint = start;
  call_modify_callbacks(start, end - start, 0, 0, deletedText);
  free((void *) deletedText);
}


/*
 Copy a range of text from another text buffer.
 fromStart, fromEnd, and toPos must be at a character boundary.
 */
void Fl_Text_Buffer::copy(Fl_Text_Buffer * fromBuf, int fromStart,
                          int fromEnd, int toPos)
{
  IS_UTF8_ALIGNED2(fromBuf, fromStart)
  IS_UTF8_ALIGNED2(fromBuf, fromEnd)
  IS_UTF8_ALIGNED2(this, (toPos))

  int copiedLength = fromEnd - fromStart;

  /* Prepare the buffer to receive the new text.  If the new text fits in
   the current buffer, just move the gap (if necessary) to where
   the text should be inserted.  If the new text is too large, reallocate
   the buffer with a gap large enough to accomodate the new text and a
   gap of mPreferredGapSize */
  if (copiedLength > mGapEnd - mGapStart)
    reallocate_with_gap(toPos, copiedLength + mPreferredGapSize);
  else if (toPos != mGapStart)
    move_gap(toPos);

  /* Insert the new text (toPos now corresponds to the start of the gap) */
  if (fromEnd <= fromBuf->mGapStart) {
    memcpy(&mBuf[toPos], &fromBuf->mBuf[fromStart], copiedLength);
  } else if (fromStart >= fromBuf->mGapStart) {
    memcpy(&mBuf[toPos],
           &fromBuf->mBuf[fromStart + (fromBuf->mGapEnd - fromBuf->mGapStart)],
           copiedLength);
  } else {
    int part1Length = fromBuf->mGapStart - fromStart;
    memcpy(&mBuf[toPos], &fromBuf->mBuf[fromStart], part1Length);
    memcpy(&mBuf[toPos + part1Length],
           &fromBuf->mBuf[fromBuf->mGapEnd], copiedLength - part1Length);
  }
  mGapStart += copiedLength;
  mLength += copiedLength;
  update_selections(toPos, 0, copiedLength);
}


/**
 Apply the current undo/redo operation, called from undo() or redo().
 */
int Fl_Text_Buffer::apply_undo(Fl_Text_Undo_Action* action, int* cursorPos)
{
  if (action->empty())
    return 0;

  mRedoList->lock();

  int ilen = action->undocut;
  int xlen = action->undoinsert;
  int b = action->undoat - xlen;

  if (xlen && action->undoyankcut && !ilen) {
    ilen = action->undoyankcut;
  }

  if (xlen && ilen) {
    action->undobuffersize(ilen + 1);
    action->undobuffer[ilen] = 0;
    char *tmp = fl_strdup(action->undobuffer);
    replace(b, action->undoat, tmp);
    if (cursorPos)
      *cursorPos = mCursorPosHint;
    free(tmp);
  } else if (xlen) {
    remove(b, action->undoat);
    if (cursorPos)
      *cursorPos = mCursorPosHint;
  } else if (ilen) {
    action->undobuffersize(ilen + 1);
    action->undobuffer[ilen] = 0;
    insert(action->undoat, action->undobuffer);
    if (cursorPos)
      *cursorPos = mCursorPosHint;
    action->undoyankcut = 0;
  }

  mRedoList->unlock();
  return 1;
}

/**
 Take the previous changes and undo them. Return the previous
 cursor position in cursorPos. Returns 1 if the undo was applied.
 CursorPos will be at a character boundary.
 */
int Fl_Text_Buffer::undo(int *cursorPos) {
  if (!mCanUndo || mUndo->empty())
    return 0;

  // save the current undo action and add an empty action to avoid generating yankcuts
  Fl_Text_Undo_Action* action = mUndo;
  mUndo = new Fl_Text_Undo_Action();

  int ret = apply_undo(action, cursorPos);
  delete action;

  if (ret) {
    // push the generated undo action to the redo list
    mRedoList->push(mUndo);
    // drop the empty action we previously created
    mUndo = mUndoList->pop();
    if (mUndo) {
      delete mUndo;
      // pop the undo action before that and make it the current undo action
      mUndo = mUndoList->pop();
      if (!mUndo) mUndo = new Fl_Text_Undo_Action();
    }
  }

  return ret;
}

/*
 Check if undo is anabled and if the last action can be undone.
 */
bool Fl_Text_Buffer::can_undo() const {
  return (mCanUndo && mUndo && !mUndo->empty());
}

/**
 Redo previous undo action.
 */
int Fl_Text_Buffer::redo(int *cursorPos) {
  if (!mCanUndo)
    return 0;

  Fl_Text_Undo_Action *redo_action = mRedoList->pop();
  if (!redo_action)
    return 0;

  // running the redo action will also generate a new undo action
  // Note: there is a slight chance that the current undo action and the
  //       generated action merge into one.
  int ret = apply_undo(redo_action, cursorPos);

  delete redo_action;
  return ret;
}

/**
 Check if undo is anabled and if the last undo action can be redone.
 \see canUndo()
 */
bool Fl_Text_Buffer::can_redo() const {
  return (mCanUndo && mRedoList->size());
}

/*
 Set a flag if undo function will work.
 */
void Fl_Text_Buffer::canUndo(char flag)
{
  if (flag) {
    if (!mCanUndo) {
      mUndo = new Fl_Text_Undo_Action();
    }
  } else {
    if (mCanUndo) {
      delete mUndo;
      mUndo = NULL;
    }
  }
  mCanUndo = flag;
}


/*
 Change the tab width. This will cause a couple of callbacks and a complete
 redisplay.
 Matt: I am not entirely sure why we need to trigger callbacks because
 tabs are only a graphical hint, not changing any text at all, but I leave
 this in here for back compatibility.
 */
void Fl_Text_Buffer::tab_distance(int tabDist)
{
  /* First call the pre-delete callbacks with the previous tab setting
   still active. */
  call_predelete_callbacks(0, mLength);

  /* Change the tab setting */
  mTabDist = tabDist;

  /* Force any display routines to redisplay everything (unfortunately,
   this means copying the whole buffer contents to provide "deletedText" */
  const char *deletedText = text();
  call_modify_callbacks(0, mLength, mLength, 0, deletedText);
  free((void *) deletedText);
}


/*
 Select a range of text.
 Start and End must be at a character boundary.
 */
void Fl_Text_Buffer::select(int start, int end)
{
  IS_UTF8_ALIGNED2(this, (start))
  IS_UTF8_ALIGNED2(this, (end))

  Fl_Text_Selection oldSelection = mPrimary;

  mPrimary.set(start, end);
  redisplay_selection(&oldSelection, &mPrimary);
}


/*
 Clear the primary selection.
 */
void Fl_Text_Buffer::unselect()
{
  Fl_Text_Selection oldSelection = mPrimary;

  mPrimary.mSelected = 0;
  redisplay_selection(&oldSelection, &mPrimary);
}


/*
 Return the primary selection range.
 */
int Fl_Text_Buffer::selection_position(int *start, int *end)
{
  return mPrimary.selected(start, end);
}


/*
 Return a copy of the selected text.
 */
char *Fl_Text_Buffer::selection_text()
{
  return selection_text_(&mPrimary);
}


/*
 Remove the selected text.
 */
void Fl_Text_Buffer::remove_selection()
{
  remove_selection_(&mPrimary);
}


/*
 Replace the selected text.
 */
void Fl_Text_Buffer::replace_selection(const char *text)
{
  replace_selection_(&mPrimary, text);
}


/*
 Select text.
 Start and End must be at a character boundary.
 */
void Fl_Text_Buffer::secondary_select(int start, int end)
{
  Fl_Text_Selection oldSelection = mSecondary;

  mSecondary.set(start, end);
  redisplay_selection(&oldSelection, &mSecondary);
}


/*
 Deselect text.
 */
void Fl_Text_Buffer::secondary_unselect()
{
  Fl_Text_Selection oldSelection = mSecondary;

  mSecondary.mSelected = 0;
  redisplay_selection(&oldSelection, &mSecondary);
}


/*
 Return the selected range.
 */
int Fl_Text_Buffer::secondary_selection_position(int *start, int *end)
{
  return mSecondary.selected(start, end);
}


/*
 Return a copy of the text in this selection.
 */
char *Fl_Text_Buffer::secondary_selection_text()
{
  return selection_text_(&mSecondary);
}


/*
 Remove the selected text.
 */
void Fl_Text_Buffer::remove_secondary_selection()
{
  remove_selection_(&mSecondary);
}


/*
 Replace selected text.
 */
void Fl_Text_Buffer::replace_secondary_selection(const char *text)
{
  replace_selection_(&mSecondary, text);
}


/*
 Highlight a range of text.
 Start and End must be at a character boundary.
 */
void Fl_Text_Buffer::highlight(int start, int end)
{
  Fl_Text_Selection oldSelection = mHighlight;

  mHighlight.set(start, end);
  redisplay_selection(&oldSelection, &mHighlight);
}


/*
 Remove text highlighting.
 */
void Fl_Text_Buffer::unhighlight()
{
  Fl_Text_Selection oldSelection = mHighlight;

  mHighlight.mSelected = 0;
  redisplay_selection(&oldSelection, &mHighlight);
}


/*
 Return position of highlight.
 */
int Fl_Text_Buffer::highlight_position(int *start, int *end)
{
  return mHighlight.selected(start, end);
}


/*
 Return a copy of highlighted text.
 */
char *Fl_Text_Buffer::highlight_text()
{
  return selection_text_(&mHighlight);
}


/*
 Add a callback that is called whenever text is modified.
 */
void Fl_Text_Buffer::add_modify_callback(Fl_Text_Modify_Cb bufModifiedCB,
                                         void *cbArg)
{
  Fl_Text_Modify_Cb *newModifyProcs =
  new Fl_Text_Modify_Cb[mNModifyProcs + 1];
  void **newCBArgs = new void *[mNModifyProcs + 1];
  for (int i = 0; i < mNModifyProcs; i++) {
    newModifyProcs[i + 1] = mModifyProcs[i];
    newCBArgs[i + 1] = mCbArgs[i];
  }
  if (mNModifyProcs != 0) {
    delete[]mModifyProcs;
    delete[]mCbArgs;
  }
  newModifyProcs[0] = bufModifiedCB;
  newCBArgs[0] = cbArg;
  mNModifyProcs++;
  mModifyProcs = newModifyProcs;
  mCbArgs = newCBArgs;
}


/*
 Remove a callback.
 */
void Fl_Text_Buffer::remove_modify_callback(Fl_Text_Modify_Cb bufModifiedCB,
                                            void *cbArg)
{
  int i, toRemove = -1;

  /* find the matching callback to remove */
  for (i = 0; i < mNModifyProcs; i++) {
    if (mModifyProcs[i] == bufModifiedCB && mCbArgs[i] == cbArg) {
      toRemove = i;
      break;
    }
  }
  if (toRemove == -1) {
    Fl::error
    ("Fl_Text_Buffer::remove_modify_callback(): Can't find modify CB to remove");
    return;
  }

  /* Allocate new lists for remaining callback procs and args (if
   any are left) */
  mNModifyProcs--;
  if (mNModifyProcs == 0) {
    mNModifyProcs = 0;
    delete[]mModifyProcs;
    mModifyProcs = NULL;
    delete[]mCbArgs;
    mCbArgs = NULL;
    return;
  }
  Fl_Text_Modify_Cb *newModifyProcs = new Fl_Text_Modify_Cb[mNModifyProcs];
  void **newCBArgs = new void *[mNModifyProcs];

  /* copy out the remaining members and free the old lists */
  for (i = 0; i < toRemove; i++) {
    newModifyProcs[i] = mModifyProcs[i];
    newCBArgs[i] = mCbArgs[i];
  }
  for (; i < mNModifyProcs; i++) {
    newModifyProcs[i] = mModifyProcs[i + 1];
    newCBArgs[i] = mCbArgs[i + 1];
  }
  delete[]mModifyProcs;
  delete[]mCbArgs;
  mModifyProcs = newModifyProcs;
  mCbArgs = newCBArgs;
}


/*
 Add a callback that is called before deleting text.
 */
void Fl_Text_Buffer::add_predelete_callback(Fl_Text_Predelete_Cb bufPreDeleteCB,
                                            void *cbArg)
{
  Fl_Text_Predelete_Cb *newPreDeleteProcs =
  new Fl_Text_Predelete_Cb[mNPredeleteProcs + 1];
  void **newCBArgs = new void *[mNPredeleteProcs + 1];
  for (int i = 0; i < mNPredeleteProcs; i++) {
    newPreDeleteProcs[i + 1] = mPredeleteProcs[i];
    newCBArgs[i + 1] = mPredeleteCbArgs[i];
  }
  if (mNPredeleteProcs > 0) {
    delete[] mPredeleteProcs;
    delete[] mPredeleteCbArgs;
  }
  newPreDeleteProcs[0] = bufPreDeleteCB;
  newCBArgs[0] = cbArg;
  mNPredeleteProcs++;
  mPredeleteProcs = newPreDeleteProcs;
  mPredeleteCbArgs = newCBArgs;
}


/*
 Remove a callback.
 */
void Fl_Text_Buffer::remove_predelete_callback(Fl_Text_Predelete_Cb bufPreDeleteCB, void *cbArg)
{
  int i, toRemove = -1;
  /* find the matching callback to remove */
  for (i = 0; i < mNPredeleteProcs; i++) {
    if (mPredeleteProcs[i] == bufPreDeleteCB &&
        mPredeleteCbArgs[i] == cbArg) {
      toRemove = i;
      break;
    }
  }
  if (toRemove == -1) {
    Fl::error
    ("Fl_Text_Buffer::remove_predelete_callback(): Can't find pre-delete CB to remove");
    return;
  }

  /* Allocate new lists for remaining callback procs and args (if any are left) */
  mNPredeleteProcs--;
  if (mNPredeleteProcs == 0) {
    delete[]mPredeleteProcs;
    mPredeleteProcs = NULL;
    delete[]mPredeleteCbArgs;
    mPredeleteCbArgs = NULL;
    return;
  }
  Fl_Text_Predelete_Cb *newPreDeleteProcs = new Fl_Text_Predelete_Cb[mNPredeleteProcs];
  void **newCBArgs = new void *[mNPredeleteProcs];

  /* copy out the remaining members and free the old lists */
  for (i = 0; i < toRemove; i++) {
    newPreDeleteProcs[i] = mPredeleteProcs[i];
    newCBArgs[i] = mPredeleteCbArgs[i];
  }
  for (; i < mNPredeleteProcs; i++) {
    newPreDeleteProcs[i] = mPredeleteProcs[i + 1];
    newCBArgs[i] = mPredeleteCbArgs[i + 1];
  }
  delete[] mPredeleteProcs;
  delete[] mPredeleteCbArgs;
  mPredeleteProcs = newPreDeleteProcs;
  mPredeleteCbArgs = newCBArgs;
}


/*
 Return a copy of the line that contains a given index.
 Pos must be at a character boundary.
 */
char *Fl_Text_Buffer::line_text(int pos) const {
  return text_range(line_start(pos), line_end(pos));
}


/*
 Find the beginning of the line.
 */
int Fl_Text_Buffer::line_start(int pos) const
{
  if (!findchar_backward(pos, '\n', &pos))
    return 0;
  return pos + 1;
}


/*
 Find the end of the line.
 */
int Fl_Text_Buffer::line_end(int pos) const {
  if (!findchar_forward(pos, '\n', &pos))
    pos = mLength;
  return pos;
}


/** Returns whether character at position \p pos is a word separator.
 Pos must be at a character boundary.
 */
bool Fl_Text_Buffer::is_word_separator(int pos) const {
  int c = char_at(pos);
  if (c < 128) {
    return !(isalnum(c) || c == '_');  // non alphanumeric ASCII
  }
  return (c == 0xA0 ||                 // NO-BREAK SPACE
          (c >= 0x3000 && c <= 0x301F) // IDEOGRAPHIC punctuation
         );
}


/*
 Find the beginning of a word.
 */
int Fl_Text_Buffer::word_start(int pos) const {
  while (pos > 0 && !is_word_separator(pos))
  {
    pos = prev_char(pos);
  }
  if (is_word_separator(pos))
    pos = next_char(pos);
  return pos;
}


/*
 Find the end of a word.
 */
int Fl_Text_Buffer::word_end(int pos) const {
  while (pos < length() && !is_word_separator(pos))
  {
    pos = next_char(pos);
  }
  return pos;
}


/*
 Count the number of characters between two positions.
 */
int Fl_Text_Buffer::count_displayed_characters(int lineStartPos,
                                               int targetPos) const
{
  IS_UTF8_ALIGNED2(this, (lineStartPos))
  IS_UTF8_ALIGNED2(this, (targetPos))

  int charCount = 0;

  int pos = lineStartPos;
  while (pos < targetPos) {
    pos = next_char(pos);
    charCount++;
  }
  return charCount;
}


/*
 Skip ahead a number of characters from a given index.
 This function breaks early if it encounters a newline character.
 */
int Fl_Text_Buffer::skip_displayed_characters(int lineStartPos, int nChars)
{
  IS_UTF8_ALIGNED2(this, (lineStartPos))

  int pos = lineStartPos;

  for (int charCount = 0; charCount < nChars && pos < mLength; charCount++) {
    unsigned int c = char_at(pos);
    if (c == '\n')
      return pos;
    pos = next_char(pos);
  }
  return pos;
}


/*
 Count the number of newline characters between start and end.
 startPos and endPos must be at a character boundary.
 This function is optimized for speed by not using UTF-8 calls.
 */
int Fl_Text_Buffer::count_lines(int startPos, int endPos) const {
  IS_UTF8_ALIGNED2(this, (startPos))
  IS_UTF8_ALIGNED2(this, (endPos))

  int gapLen = mGapEnd - mGapStart;
  int lineCount = 0;

  int pos = startPos;
  while (pos < mGapStart)
  {
    if (pos == endPos)
      return lineCount;
    if (mBuf[pos++] == '\n')
      lineCount++;
  }
  while (pos < mLength) {
    if (pos == endPos)
      return lineCount;
    if (mBuf[pos++ + gapLen] == '\n')
      lineCount++;
  }
  return lineCount;
}

/**
 Estimate the number of newlines between \p startPos and \p endPos in buffer.
 This call takes line wrapping into account. It assumes a line break at every
 `lineLen` characters after the beginning of a line.
 */
int Fl_Text_Buffer::estimate_lines(int startPos, int endPos, int lineLen) const
{
  IS_UTF8_ALIGNED2(this, (startPos))
  IS_UTF8_ALIGNED2(this, (endPos))

  int gapLen = mGapEnd - mGapStart;
  int lineCount = 0;
  int softLineBreaks = 0, softLineBreakCount = lineLen;

  int pos = startPos;
  while (pos < mGapStart)
  {
    if (pos == endPos)
      return lineCount + softLineBreaks;
    if (mBuf[pos++] == '\n') {
      softLineBreakCount = lineLen;
      lineCount++;
    }
    if (--softLineBreakCount == 0) {
      softLineBreakCount = lineLen;
      softLineBreaks++;
    }
  }
  while (pos < mLength) {
    if (pos == endPos)
      return lineCount + softLineBreaks;
    if (mBuf[pos++ + gapLen] == '\n') {
      softLineBreakCount = lineLen;
      lineCount++;
    }
    if (--softLineBreakCount == 0) {
      softLineBreakCount = lineLen;
      softLineBreaks++;
    }
  }
  return lineCount + softLineBreaks;
}

/*
 Skip to the first character, n lines ahead.
 StartPos must be at a character boundary.
 This function is optimized for speed by not using UTF-8 calls.
 */
int Fl_Text_Buffer::skip_lines(int startPos, int nLines)
{
  IS_UTF8_ALIGNED2(this, (startPos))

  if (nLines == 0)
    return startPos;

  int gapLen = mGapEnd - mGapStart;
  int pos = startPos;
  int lineCount = 0;
  while (pos < mGapStart) {
    if (mBuf[pos++] == '\n') {
      lineCount++;
      if (lineCount == nLines) {
        IS_UTF8_ALIGNED2(this, (pos))
        return pos;
      }
    }
  }
  while (pos < mLength) {
    if (mBuf[pos++ + gapLen] == '\n') {
      lineCount++;
      if (lineCount >= nLines) {
        IS_UTF8_ALIGNED2(this, (pos))
        return pos;
      }
    }
  }
  IS_UTF8_ALIGNED2(this, (pos))
  return pos;
}


/*
 Skip to the first character, n lines back.
 StartPos must be at a character boundary.
 This function is optimized for speed by not using UTF-8 calls.
 */
int Fl_Text_Buffer::rewind_lines(int startPos, int nLines)
{
  IS_UTF8_ALIGNED2(this, (startPos))

  int pos = startPos - 1;
  if (pos <= 0)
    return 0;

  int gapLen = mGapEnd - mGapStart;
  int lineCount = -1;
  while (pos >= mGapStart) {
    if (mBuf[pos + gapLen] == '\n') {
      if (++lineCount >= nLines) {
        IS_UTF8_ALIGNED2(this, (pos+1))
        return pos + 1;
      }
    }
    pos--;
  }
  while (pos >= 0) {
    if (mBuf[pos] == '\n') {
      if (++lineCount >= nLines) {
        IS_UTF8_ALIGNED2(this, (pos+1))
        return pos + 1;
      }
    }
    pos--;
  }
  return 0;
}


/*
 Find a matching string in the buffer.
 */
int Fl_Text_Buffer::search_forward(int startPos, const char *searchString,
                                   int *foundPos, int matchCase) const
{
  IS_UTF8_ALIGNED2(this, (startPos))
  IS_UTF8_ALIGNED(searchString)

  if (!searchString)
    return 0;
  int bp;
  const char *sp;
  if (matchCase) {
    while (startPos < length()) {
      bp = startPos;
      sp = searchString;
      for (;;) {
        char c = *sp;
        // we reached the end of the "needle", so we found the string!
        if (!c) {
          *foundPos = startPos;
          return 1;
        }
        int l = fl_utf8len1(c);
        if (memcmp(sp, address(bp), l))
          break;
        sp += l; bp += l;
      }
      startPos = next_char(startPos);
    }
  } else {
    while (startPos < length()) {
      bp = startPos;
      sp = searchString;
      for (;;) {
        // we reached the end of the "needle", so we found the string!
        if (!*sp) {
          *foundPos = startPos;
          return 1;
        }
        int l;
        unsigned int b = char_at(bp);
        unsigned int s = fl_utf8decode(sp, 0, &l);
        if (fl_tolower(b)!=fl_tolower(s))
          break;
        sp += l;
        bp = next_char(bp);
      }
      startPos = next_char(startPos);
    }
  }
  return 0;
}

int Fl_Text_Buffer::search_backward(int startPos, const char *searchString,
                                    int *foundPos, int matchCase) const
{
  IS_UTF8_ALIGNED2(this, (startPos))
  IS_UTF8_ALIGNED(searchString)

  if (!searchString)
    return 0;
  int bp;
  const char *sp;
  if (matchCase) {
    while (startPos >= 0) {
      bp = startPos;
      sp = searchString;
      for (;;) {
        char c = *sp;
        // we reached the end of the "needle", so we found the string!
        if (!c) {
          *foundPos = startPos;
          return 1;
        }
        int l = fl_utf8len1(c);
        if (memcmp(sp, address(bp), l))
          break;
        sp += l; bp += l;
      }
      startPos = prev_char(startPos);
    }
  } else {
    while (startPos >= 0) {
      bp = startPos;
      sp = searchString;
      for (;;) {
        // we reached the end of the "needle", so we found the string!
        if (!*sp) {
          *foundPos = startPos;
          return 1;
        }
        int l;
        unsigned int b = char_at(bp);
        unsigned int s = fl_utf8decode(sp, 0, &l);
        if (fl_tolower(b)!=fl_tolower(s))
          break;
        sp += l;
        bp = next_char(bp);
      }
      startPos = prev_char(startPos);
    }
  }
  return 0;
}



/*
 Insert a string into the buffer.
 Pos must be at a character boundary. Text must be a correct UTF-8 string.
 */
int Fl_Text_Buffer::insert_(int pos, const char *text, int insertedLength)
{
  if (!text || !*text)
    return 0;

  if (insertedLength == -1) insertedLength = (int) strlen(text);

  /* Prepare the buffer to receive the new text.  If the new text fits in
   the current buffer, just move the gap (if necessary) to where
   the text should be inserted.  If the new text is too large, reallocate
   the buffer with a gap large enough to accomodate the new text and a
   gap of mPreferredGapSize */
  if (insertedLength > mGapEnd - mGapStart)
    reallocate_with_gap(pos, insertedLength + mPreferredGapSize);
  else if (pos != mGapStart)
    move_gap(pos);

  /* Insert the new text (pos now corresponds to the start of the gap) */
  memcpy(&mBuf[pos], text, insertedLength);
  mGapStart += insertedLength;
  mLength += insertedLength;
  update_selections(pos, 0, insertedLength);

  if (mCanUndo) {
    if (mUndo->undoat == pos && mUndo->undoinsert) {
      // continue inserting text at the given cursor position
      mUndo->undoinsert += insertedLength;
    } else {
      int yankcut = (mUndo->undoat == pos) ? mUndo->undocut : 0;
      if (!yankcut) {
        // insert text at a new position, so generate a new undo action
        mRedoList->clear();
        mUndoList->push(mUndo);
        mUndo = new Fl_Text_Undo_Action();
      } else {
        // we deleted and inserted at the same position, making this a yankcut
      }
      mUndo->undoinsert = insertedLength;
      mUndo->undoyankcut = yankcut;
    }
    mUndo->undoat = pos + insertedLength;
    mUndo->undocut = 0;
  }

  return insertedLength;
}


/*
 Remove a string from the buffer.
 Unicode safe. Start and end must be at a character boundary.
 Start must be less than end.
 */
void Fl_Text_Buffer::remove_(int start, int end)
{
  if (start >= end) return;
  if (mCanUndo) {
    if (mUndo->undoat == end && mUndo->undocut) {
      // continue to remove text at the same cursor position
      mUndo->undobuffersize(mUndo->undocut + end - start + 1);
      memmove(mUndo->undobuffer + end - start, mUndo->undobuffer, mUndo->undocut);
      mUndo->undocut += end - start;
    } else {
      // remove text at a new position, so generate a new undo action
      mRedoList->clear();
      mUndoList->push(mUndo);
      mUndo = new Fl_Text_Undo_Action();
      mUndo->undocut = end - start;
      mUndo->undobuffersize(mUndo->undocut);
    }
    mUndo->undoat = start;
    mUndo->undoinsert = 0;
    mUndo->undoyankcut = 0;
  }

  if (start > mGapStart) {
    if (mCanUndo)
      memcpy(mUndo->undobuffer, mBuf + (mGapEnd - mGapStart) + start,
             end - start);
    move_gap(start);
  } else if (end < mGapStart) {
    if (mCanUndo)
      memcpy(mUndo->undobuffer, mBuf + start, end - start);
    move_gap(end);
  } else {
    int prelen = mGapStart - start;
    if (mCanUndo) {
      memcpy(mUndo->undobuffer, mBuf + start, prelen);
      memcpy(mUndo->undobuffer + prelen, mBuf + mGapEnd, end - start - prelen);
    }
  }

  /* expand the gap to encompass the deleted characters */
  mGapEnd += end - mGapStart;
  mGapStart = start;

  /* update the length */
  mLength -= end - start;

  /* fix up any selections which might be affected by the change */
  update_selections(start, end - start, 0);
}


/**
 \brief Sets the selection range.

  \p startpos and \p endpos must be at a character boundary.

  If \p startpos != \p endpos selected() is set to true, else to false.

  If \p startpos is greater than \p endpos they are swapped so that
  \p startpos \<= \p endpos.

  \param[in] startpos  byte offset to first selected character
  \param[in] endpos    byte offset pointing after last selected character
*/
void Fl_Text_Selection::set(int startpos, int endpos)
{
  mSelected = (startpos != endpos);
  mStart = min(startpos, endpos);
  mEnd = max(startpos, endpos);
}


/**
  \brief Returns the status and the positions of this selection.

  This method returns the same as \p selected() as an \p int (0 or 1)
  in its return value and the offsets to the start of the selection
  in \p startpos and to the byte after the last selected character
  in \p endpos, if selected() is \p true.

  If selected() is \p false, both offsets are set to 0.

  \note In FLTK 1.3.x \p startpos and \p endpos were \b not \b modified
    if selected() was false.

  \param startpos  return byte offset to first selected character
  \param endpos    return byte offset pointing after last selected character

  \return whether the selection is active (selected()) or not
  \retval 0 if not selected
  \retval 1 if selected

  \see selected(), start(), end()
*/
int Fl_Text_Selection::selected(int *startpos, int *endpos) const {
  if (!mSelected) {
    *startpos = 0;
    *endpos = 0;
    return 0;
  }
  *startpos = mStart;
  *endpos = mEnd;
  return 1;
}


/**
  Returns true if position \p pos is in the Fl_Text_Selection.

  \p pos must be at a character boundary.
*/
int Fl_Text_Selection::includes(int pos) const {
  return (selected() && pos >= start() && pos < end() );
}


/*
 Return a duplicate of the selected text, or an empty string.
 Unicode safe.
 */
char *Fl_Text_Buffer::selection_text_(Fl_Text_Selection * sel) const {
  int start, end;

  /* If there's no selection, return an allocated empty string */
  if (!sel->selected(&start, &end))
  {
    char *s = (char *) malloc(1);
    *s = '\0';
    return s;
  }

  /* Return the selected range */
    return text_range(start, end);
}


/*
 Remove the selected text.
 Unicode safe.
 */
void Fl_Text_Buffer::remove_selection_(Fl_Text_Selection * sel)
{
  int start, end;

  if (!sel->selected(&start, &end))
    return;
  remove(start, end);
  //undoyankcut = undocut;
}


/*
 Replace selection with text.
 Unicode safe.
 */
void Fl_Text_Buffer::replace_selection_(Fl_Text_Selection * sel,
                                        const char *text)
{
  Fl_Text_Selection oldSelection = *sel;

  /* If there's no selection, return */
  int start, end;
  if (!sel->selected(&start, &end))
    return;

  /* Do the appropriate type of replace */
  replace(start, end, text);

  /* Unselect (happens automatically in BufReplace, but BufReplaceRect
   can't detect when the contents of a selection goes away) */
  sel->mSelected = 0;
  redisplay_selection(&oldSelection, sel);
}


/*
 Call all callbacks.
 Unicode safe.
 */
void Fl_Text_Buffer::call_modify_callbacks(int pos, int nDeleted,
                                           int nInserted, int nRestyled,
                                           const char *deletedText) const {
  IS_UTF8_ALIGNED2(this, pos)
  for (int i = 0; i < mNModifyProcs; i++)
    (*mModifyProcs[i]) (pos, nInserted, nDeleted, nRestyled,
                        deletedText, mCbArgs[i]);
}


/*
 Call all callbacks.
 Unicode safe.
 */
void Fl_Text_Buffer::call_predelete_callbacks(int pos, int nDeleted) const {
  for (int i = 0; i < mNPredeleteProcs; i++)
    (*mPredeleteProcs[i]) (pos, nDeleted, mPredeleteCbArgs[i]);
}


/*
 Redisplay a new selected area.
 Unicode safe.
 */
void Fl_Text_Buffer::redisplay_selection(Fl_Text_Selection *
                                           oldSelection,
                                           Fl_Text_Selection *
                                           newSelection) const
{
  int oldStart, oldEnd, newStart, newEnd, ch1Start, ch1End, ch2Start,
  ch2End;

  /* If either selection is rectangular, add an additional character to
   the end of the selection to request the redraw routines to wipe out
   the parts of the selection beyond the end of the line */
  oldStart = oldSelection->mStart;
  newStart = newSelection->mStart;
  oldEnd = oldSelection->mEnd;
  newEnd = newSelection->mEnd;

  /* If the old or new selection is unselected, just redisplay the
   single area that is (was) selected and return */
  if (!oldSelection->mSelected && !newSelection->mSelected)
    return;
  if (!oldSelection->mSelected)
  {
    call_modify_callbacks(newStart, 0, 0, newEnd - newStart, NULL);
    return;
  }
  if (!newSelection->mSelected) {
    call_modify_callbacks(oldStart, 0, 0, oldEnd - oldStart, NULL);
    return;
  }

  /* If the selections are non-contiguous, do two separate updates
   and return */
  if (oldEnd < newStart || newEnd < oldStart) {
    call_modify_callbacks(oldStart, 0, 0, oldEnd - oldStart, NULL);
    call_modify_callbacks(newStart, 0, 0, newEnd - newStart, NULL);
    return;
  }

  /* Otherwise, separate into 3 separate regions: ch1, and ch2 (the two
   changed areas), and the unchanged area of their intersection,
   and update only the changed area(s) */
  ch1Start = min(oldStart, newStart);
  ch2End = max(oldEnd, newEnd);
  ch1End = max(oldStart, newStart);
  ch2Start = min(oldEnd, newEnd);
  if (ch1Start != ch1End)
    call_modify_callbacks(ch1Start, 0, 0, ch1End - ch1Start, NULL);
  if (ch2Start != ch2End)
    call_modify_callbacks(ch2Start, 0, 0, ch2End - ch2Start, NULL);
}


/*
 Move the gap around without changing buffer content.
 Unicode safe. Pos must be at a character boundary.
 */
void Fl_Text_Buffer::move_gap(int pos)
{
  int gapLen = mGapEnd - mGapStart;

  if (pos > mGapStart)
    memmove(&mBuf[mGapStart], &mBuf[mGapEnd], pos - mGapStart);
  else
    memmove(&mBuf[pos + gapLen], &mBuf[pos], mGapStart - pos);
  mGapEnd += pos - mGapStart;
  mGapStart += pos - mGapStart;
}


/*
 Create a larger gap.
 Unicode safe. Start must be at a character boundary.
 */
void Fl_Text_Buffer::reallocate_with_gap(int newGapStart, int newGapLen)
{
  char *newBuf = (char *) malloc(mLength + newGapLen);
  int newGapEnd = newGapStart + newGapLen;

  if (newGapStart <= mGapStart) {
    memcpy(newBuf, mBuf, newGapStart);
    memcpy(&newBuf[newGapEnd], &mBuf[newGapStart],
           mGapStart - newGapStart);
    memcpy(&newBuf[newGapEnd + mGapStart - newGapStart],
           &mBuf[mGapEnd], mLength - mGapStart);
  } else {                      /* newGapStart > mGapStart */
    memcpy(newBuf, mBuf, mGapStart);
    memcpy(&newBuf[mGapStart], &mBuf[mGapEnd], newGapStart - mGapStart);
    memcpy(&newBuf[newGapEnd],
           &mBuf[mGapEnd + newGapStart - mGapStart],
           mLength - newGapStart);
  }
  free((void *) mBuf);
  mBuf = newBuf;
  mGapStart = newGapStart;
  mGapEnd = newGapEnd;
}


/*
 Update selection range if characters were inserted.
 Unicode safe. Pos must be at a character boundary.
 */
void Fl_Text_Buffer::update_selections(int pos, int nDeleted,
                                       int nInserted)
{
  mPrimary.update(pos, nDeleted, nInserted);
  mSecondary.update(pos, nDeleted, nInserted);
  mHighlight.update(pos, nDeleted, nInserted);
}


/**
  \brief Updates a selection after text was modified.

  Updates an individual selection for changes in the corresponding text.

  \param pos byte offset into text buffer at which the change occurred
  \param nDeleted number of bytes deleted from the buffer
  \param nInserted number of bytes inserted into the buffer
*/
// unicode safe, assuming the arguments are on character boundaries

void Fl_Text_Selection::update(int pos, int nDeleted, int nInserted)
{
  if (!mSelected || pos > mEnd)
    return;
  if (pos + nDeleted <= mStart) {
    mStart += nInserted - nDeleted;
    mEnd += nInserted - nDeleted;
  } else if (pos <= mStart && pos + nDeleted >= mEnd) {
    mStart = pos;
    mEnd = pos;
    mSelected = 0;
  } else if (pos <= mStart && pos + nDeleted < mEnd) {
    mStart = pos;
    mEnd = nInserted + mEnd - nDeleted;
  } else if (pos < mEnd) {
    mEnd += nInserted - nDeleted;
    if (mEnd <= mStart)
      mSelected = 0;
  }
}


/*
 Find a UCS-4 character.
 StartPos must be at a character boundary, searchChar is UCS-4 encoded.
 */
int Fl_Text_Buffer::findchar_forward(int startPos, unsigned searchChar,
                                     int *foundPos) const
{
  if (startPos >= mLength) {
    *foundPos = mLength;
    return 0;
  }

  if (startPos<0)
    startPos = 0;

  for ( ; startPos<mLength; startPos = next_char(startPos)) {
    if (searchChar == char_at(startPos)) {
      *foundPos = startPos;
      return 1;
    }
  }

  *foundPos = mLength;
  return 0;
}


/*
 Find a UCS-4 character.
 StartPos must be at a character boundary, searchChar is UCS-4 encoded.
 */
int Fl_Text_Buffer::findchar_backward(int startPos, unsigned int searchChar,
                                      int *foundPos) const {
  if (startPos <= 0) {
    *foundPos = 0;
    return 0;
  }

  if (startPos > mLength)
    startPos = mLength;

  for (startPos = prev_char(startPos); startPos>=0; startPos = prev_char(startPos)) {
    if (searchChar == char_at(startPos)) {
      *foundPos = startPos;
      return 1;
    }
  }

  *foundPos = 0;
  return 0;
}

//#define EXAMPLE_ENCODING // shows how to process any encoding for which a decoding function exists
#ifdef EXAMPLE_ENCODING

// returns the UCS equivalent of *p in CP1252 and advances p by 1
unsigned cp1252toucs(char* &p)
{
  // Codes 0x80..0x9f from the Microsoft CP1252 character set, translated
  // to Unicode
  static unsigned cp1252[32] = {
    0x20ac, 0x0081, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008d, 0x017d, 0x008f,
    0x0090, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0x009d, 0x017e, 0x0178
  };
  unsigned char uc = *(unsigned char*)p;
  p++;
  return (uc < 0x80 || uc >= 0xa0 ? uc : cp1252[uc - 0x80]);
}

// returns the UCS equivalent of *p in UTF-16 and advances p by 2 (or more for surrogates)
unsigned utf16toucs(char* &p)
{
  union {
#if WORDS_BIGENDIAN
    struct { unsigned char a, b;} chars;
#else
    struct { unsigned char b, a;} chars;
#endif
    U16 short_val;
  } u;
  u.chars.a = *(unsigned char*)p++;
  u.chars.b = *(unsigned char*)p++;
  return u.short_val;
}

// filter that produces, from an input stream fed by reading from fp,
// a UTF-8-encoded output stream written in buffer.
// Input can be any (e.g., 8-bit, UTF-16) encoding.
// Output is true UTF-8.
// p_trf points to a function that transforms encoded byte(s) into one UCS
// and that increases the pointer by the adequate quantity
static int general_input_filter(char *buffer, int buflen,
                                 char *line, int sline, char* &endline,
                                 unsigned (*p_trf)(char* &),
                                 FILE *fp)
{
  char *p, *q, multibyte[5];
  int lq, r, offset;
  p = line;
  q = buffer;
  while (q < buffer + buflen) {
    if (p >= endline) {
      r = fread(line, 1, sline, fp);
      endline = line + r;
      if (r == 0) return q - buffer;
      p = line;
    }
    if (q + 4 /*max width of UTF-8 char*/ > buffer + buflen) {
      memmove(line, p, endline - p);
      endline -= (p - line);
      return q - buffer;
    }
    lq = fl_utf8encode( p_trf(p), multibyte );
    memcpy(q, multibyte, lq);
    q += lq;
  }
  memmove(line, p, endline - p);
  endline -= (p - line);
  return q - buffer;
}
#endif // EXAMPLE_ENCODING

/*
 filter that produces, from an input stream fed by reading from fp,
 a UTF-8-encoded output stream written in buffer.
 Returns #bytes read into 'buffer'.

 Input can be UTF-8. If it is not, it is decoded with CP1252.
 Output is UTF-8.

 *input_was_changed returns true if input was not strict UTF-8, so output
 differs from input.
 */
static int utf8_input_filter(char *buffer,              // result buffer we fill with utf8 encoded text
                             int buflen,                // max size of buffer from caller
                             char *line,                // file line buffer caller wants us to use
                             int sline,                 // max size of line buffer
                             char* &endline,            // keeps track of leftovers in line[] buffer between calls
                             FILE *fp,                  // open file we're reading data from
                             int *input_was_changed)    // returned flag: 'true' if buffer[] different from file due to utf8 encoding
{
  // p - work pointer to line[]
  // q - work pointer to buffer[]
  // l - length of utf8 sequence being worked on
  // lp - fl_utf8decode() length of utf8 sequence being worked on
  // lq - fl_utf8encode() length of utf8 sequence being worked on
  // r - bytes read from last fread()
  // u - utf8 decoded sequence as a single multibyte unsigned integer
  char *p, *q, multibyte[5];
  int l, lp, lq, r;
  unsigned u;
  p = line;
  q = buffer;
  while (q < buffer + buflen) {
    if (p >= endline) {                 // walked off end of input file's line buffer?
      r = (int) fread(line, 1, sline, fp);      // read another block of sline bytes from file
      endline = line + r;
      if (r == 0) return (int) (q - buffer);    // EOF? return bytes read into buffer[]
      p = line;
    }
    // Predict length of utf8 sequence
    //    See if utf8 seq we're working on would extend off end of line buffer,
    //    and if so, adjust + load more data so that it doesn't.
    //
    l = fl_utf8len1(*p);                // anticipate length of utf8 sequence
    if (p + l > endline) {              // would walk off end of line buffer?
      memmove(line, p, endline - p);    // re-jigger line buffer to get some room
      endline -= (p - line);
      r = (int) fread(endline, 1, sline - (endline - line), fp);         // re-fill line buffer
      endline += r;
      p = line;
      if (endline - line < l) break;    // sequence *still* extends past end? stop loop
    }
    while ( l > 0) {
      u = fl_utf8decode(p, p+l, &lp);   // get single utf8 encoded char as a Unicode value
      lq = fl_utf8encode(u, multibyte); // re-encode Unicode value to utf8 in multibyte[]
      if (lp != l || lq != l) *input_was_changed = true;

      if (q + lq > buffer + buflen) {   // encoding would walk off end of buffer[]?
        memmove(line, p, endline - p);  // re-jigger line[] buffer for next call
        endline -= (p - line);          // adjust end of line[] buffer for next call
        return (int) (q - buffer);              // return what's decoded so far, caller will consume buffer
      }
      memcpy(q, multibyte, lq);
      q += lq;
      p += lp;
      l -= lp;
    }
  }
  memmove(line, p, endline - p);
  endline -= (p - line);
  return (int) (q - buffer);
}

const char *Fl_Text_Buffer::file_encoding_warning_message =
"Displayed text contains the UTF-8 transcoding\n"
"of the input file which was not UTF-8 encoded.\n"
"Some changes may have occurred.";

/*
 Insert text from a file.
 Input file can be of various encodings according to what input fiter is used.
 utf8_input_filter accepts UTF-8 or CP1252 as input encoding.
 Output is always UTF-8.
 */
 int Fl_Text_Buffer::insertfile(const char *file, int pos, int buflen)
{
  FILE *fp;
  if (!(fp = fl_fopen(file, "r")))
    return 1;
  char *buffer = new char[buflen + 1];
  char *endline, line[100];
  int l;
  input_file_was_transcoded = false;
  endline = line;
  while (true) {
#ifdef EXAMPLE_ENCODING
    // example of 16-bit encoding: UTF-16
    l = general_input_filter(buffer, buflen,
                                  line, sizeof(line), endline,
                                  utf16toucs, // use cp1252toucs to read CP1252-encoded files
                                  fp);
    input_file_was_transcoded = true;
#else
    l = utf8_input_filter(buffer, buflen, line, sizeof(line), endline,
                          fp, &input_file_was_transcoded);
#endif
    if (l == 0) break;
    buffer[l] = 0;
    insert(pos, buffer);
    pos += l;
  }
  int e = ferror(fp) ? 2 : 0;
  fclose(fp);
  delete[]buffer;
  if ( (!e) && input_file_was_transcoded && transcoding_warning_action) {
    transcoding_warning_action(this);
  }
  return e;
}


/*
 Write text to file.
 Unicode safe.
 */
int Fl_Text_Buffer::outputfile(const char *file,
                               int start, int end,
                               int buflen) {
  FILE *fp;
  if (!(fp = fl_fopen(file, "w")))
    return 1;
  for (int n; (n = min(end - start, buflen)); start += n) {
    const char *p = text_range(start, start + n);
    int r = (int) fwrite(p, 1, n, fp);
    free((void *) p);
    if (r != n)
      break;
  }

  int e = ferror(fp) ? 2 : 0;
  fclose(fp);
  return e;
}


/*
 Return the previous character position.
 Unicode safe.
 */
int Fl_Text_Buffer::prev_char_clipped(int pos) const
{
  if (pos<=0)
    return 0;

  IS_UTF8_ALIGNED2(this, (pos))

  char c;
  do {
    pos--;
    if (pos==0)
      return 0;
    c = byte_at(pos);
  } while ( (c&0xc0) == 0x80);

  IS_UTF8_ALIGNED2(this, (pos))
  return pos;
}


/*
 Return the previous character position.
 Returns -1 if the beginning of the buffer is reached.
 */
int Fl_Text_Buffer::prev_char(int pos) const
{
  if (pos==0) return -1;
  return prev_char_clipped(pos);
}


/*
 Return the next character position.
 Returns length() if the end of the buffer is reached.
 */
int Fl_Text_Buffer::next_char(int pos) const
{
  IS_UTF8_ALIGNED2(this, (pos))
  int n = fl_utf8len1(byte_at(pos));
  pos += n;
  if (pos>=mLength)
    return mLength;
  IS_UTF8_ALIGNED2(this, (pos))
  return pos;
}


/*
 Return the next character position.
 If the end of the buffer is reached, it returns the current position.
 */
int Fl_Text_Buffer::next_char_clipped(int pos) const
{
  return next_char(pos);
}

/*
 Align an index to the current UTF-8 boundary.
 */
int Fl_Text_Buffer::utf8_align(int pos) const
{
  char c = byte_at(pos);
  while ( (c&0xc0) == 0x80) {
    pos--;
    c = byte_at(pos);
  }
  return pos;
}
