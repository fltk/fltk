#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <stdio.h>

int main(int, char **) {
  fl_message("Spelling check sucessfull.");
  fl_alert("Quantum fluctuations in the space-time continuim detected.");
  printf("fl_ask returned %d\n",
    fl_ask("Do you really want to?"));
  printf("fl_choice returned %d\n",
    fl_choice("Choose one of the following:","choice0","choice1","choice2"));
  printf("fl_show_input returned \"%s\"\n",
    fl_show_input("Please enter a string:", "this is the default value"));
  printf("fl_password returned \"%s\"\n",
    fl_password("Enter your password:"));
  return 0;
}
