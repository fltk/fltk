// fl_file_chooser.H

#ifndef fl_file_chooser_H
#define fl_file_chooser_H

char *fl_file_chooser(const char *message,const char *pat,const char *fname);
void fl_file_chooser_callback(void (*cb)(const char *));

#endif
